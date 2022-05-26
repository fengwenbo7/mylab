#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <assert.h>

#define USER_LIMIT 5
#define EPOLL_EVENTS_NUM 1024
#define BUFFER_SIZE 1024
#define FD_LIMIT 65535
#define PROCESS_LIMIT 65536

struct client_data{
    int conn_fd;//file description,communication with client
    struct sockaddr_in adress;//address of client
    pid_t pid;//process for this client
    int pipefd[2];//parent process communicates with parent process
};

epoll_event epollevents[EPOLL_EVENTS_NUM];
static const char* shm_name="/my_shm";
int sig_pipefd[2];
int epoll_fd;
int listen_fd;
int shm_fd;
char* share_mem=0;//shared memory
client_data* users=0;//container of client
int* sub_processes=0;//processes for each users,index is pid,value is index of users
int user_count=0;//the count of users
bool stop_child=false;

void set_unblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
}

void add_fd_to_epoll(int epoll_fd,int fd){
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN|EPOLLET;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&event);
    //ET模式下每次write或read需要循环write或read直到返回EAGAIN错误。以读操作为例，这是因为ET模式只在socket描述符状态发生变化时才触发事件，
    //如果不一次把socket内核缓冲区的数据读完，会导致socket内核缓冲区中即使还有一部分数据，该socket的可读事件也不会被触发
    //根据上面的讨论，若ET模式下使用阻塞IO，则程序一定会阻塞在最后一次write或read操作，因此说ET模式下一定要使用非阻塞IO
    set_unblocking(fd);
}

void sig_handler(int sig){
    int save_errno=errno;
    int msg=sig;
    send(sig_pipefd[1],&msg,1,0);
    errno=save_errno;
}

void add_sig(int sig,void(*handler)(int),bool restart=true){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler=handler;
    if(restart){
        sa.sa_flags|=SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL)!=-1);
}

void del_resources(){
    close(sig_pipefd[0]);
    close(sig_pipefd[1]);
    close(epoll_fd);
    close(listen_fd);
    shm_unlink(share_mem);
    delete[] users;
    delete[] sub_processes;
}

void child_term_handle(int sig){
    stop_child=true;
}

//child running in sub-process,listen socket and pipefd
int run_child(int child_index,client_data* users,char* share_mem){
    //create epoll
    int sub_epollfd=epoll_create(5);
    assert(sub_epollfd!=-1);
    epoll_event events[EPOLL_EVENTS_NUM];

    //add client connfd into epoll
    int conn_fd=users[child_index].conn_fd;
    add_fd_to_epoll(sub_epollfd,conn_fd);

    //add pipefd into epoll
    int pipefd=users[child_index].pipefd[1];
    add_fd_to_epoll(sub_epollfd,pipefd);

    //add sig handle
    add_sig(SIGTERM,child_term_handle,false);

    //loop in child
    while(!stop_child){
        //pipefd saved for communication with parent
        //connfd saved for communication with clients
        int ret=epoll_wait(sub_epollfd,events,EPOLL_EVENTS_NUM,-1);
        if(ret<0&&errno!=EINTR){
            perror("epoll_wait error.");
            break;
        }
        for(int i=0;i<ret;i++){
            int sock_fd=events[i].data.fd;
            if(sock_fd==conn_fd&&(events[i].events&EPOLLIN)){//the client that this process handle send data---client
                memset(share_mem+BUFFER_SIZE*child_index,'\0',BUFFER_SIZE);//read data from client and sava into the mem_share
                ret=recv(sock_fd,share_mem+child_index*BUFFER_SIZE,BUFFER_SIZE-1,0);//receive msg from client and save in the memory shared,then call parent process
                if(ret<0){
                    if(errno!=EAGAIN){
                        perror("recv error.");
                        stop_child=true;
                    }
                }
                else if(ret==0){
                    stop_child=true;
                }
                else{
                    send(pipefd,(char*)&child_index,sizeof(child_index),0);
                }
            }
            else if(sock_fd==pipefd&&(events[i].events&EPOLLIN)){//parent send data to client which index of child_index---parent
                int client=0;
                ret=recv(sock_fd,(char*)&client,sizeof(client),0);
                if(ret<0){
                    if(errno!=EAGAIN){
                        stop_child=true;
                    }
                }
                else if(ret==0){
                    stop_child=true;
                }
                else{
                    send(conn_fd,share_mem+client*BUFFER_SIZE,BUFFER_SIZE,0);
                }
            }
            else{
                continue;
            }
        }
    }
    close(conn_fd);
    close(pipefd);
    close(sub_epollfd);
    return 0;
}

int server_init(){
    int ret=0;
    //create socket
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_port=htons(9999);
    inet_pton(AF_INET,"127.0.0.1",&address.sin_addr);

    listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket error.");
        return -1;
    }
    ret=bind(listen_fd,(const struct sockaddr*)&address,sizeof(address));
    if(ret==-1){
        perror("bind error.");
        return -1;
    }

    ret=listen(listen_fd,5);
    if(ret==-1){
        perror("listen error.");
        return -1;
    }

    //add socket to epoll
    epoll_fd=epoll_create(5);
    assert(epoll_fd>0);
    add_fd_to_epoll(epoll_fd,listen_fd);

    //create pipe for sig,add sig to epoll
    ret=socketpair(PF_UNIX,SOCK_STREAM,0,sig_pipefd);
    assert(ret!=-1);
    set_unblocking(sig_pipefd[1]);
    add_fd_to_epoll(epoll_fd,sig_pipefd[0]);

    //add sig handler
    add_sig(SIGCHLD,sig_handler);
    add_sig(SIGTERM,sig_handler);
    add_sig(SIGINT,sig_handler);
    add_sig(SIGPIPE,sig_handler);

    //read buffer of memory shared for all users
    shm_fd=shm_open(shm_name,O_CREAT|O_RDWR,0666);
    assert(shm_fd!=-1);
    ret=ftruncate(shm_fd,USER_LIMIT*BUFFER_SIZE);
    assert(ret!=-1);
    share_mem=(char*)mmap(NULL,USER_LIMIT*BUFFER_SIZE,PROT_READ|PROT_WRITE,MAP_SHARED,shm_fd,0);
    assert(share_mem!=MAP_FAILED);
    close(shm_fd);

    //init client data
    user_count=0;
    users=new client_data[USER_LIMIT+1];//1 server+5 clients
    sub_processes=new int[PROCESS_LIMIT];
    for(int i=0;i<PROCESS_LIMIT;i++){
        sub_processes[i]=-1;
    }

    return 1;
}

int main(){
    int ret=0;
    assert(server_init()==1);
    bool stop_server=false;
    bool terminate=false;
    while(!stop_server){
        int num=epoll_wait(epoll_fd,epollevents,EPOLL_EVENTS_NUM,-1);
        //1.pipefd for sig from system
        //2.listen_fd for socket listening of new client
        //3.sub-process-pipefd for sub-process
        if(num<0&&errno!=EINTR){
            perror("epoll_wait error.");
            break;
        }
        for(int i=0;i<num;i++){
            int sock_fd=epollevents[i].data.fd;
            if(sock_fd==listen_fd){//new client
                struct sockaddr_in address;
                socklen_t len=sizeof(address);
                int conn_fd=accept(sock_fd,(sockaddr*)&address,&len);
                if(conn_fd<0){
                    perror("accept error.");
                    continue;
                }
                if(user_count>=USER_LIMIT){
                    const char* info="too many clients.\n";
                    send(conn_fd,info,sizeof(info),0);
                    printf("%s",info);
                    close(conn_fd);
                    continue;
                }
                printf("new client connect.\n");
                //save client data
                users[user_count].adress=address;
                users[user_count].conn_fd=conn_fd;
                //create pipe bewteen server and client
                ret=socketpair(PF_UNIX,SOCK_STREAM,0,users[user_count].pipefd);
                assert(ret!=-1);
                //create child process
                pid_t pid=fork();
                if(pid<0){
                    perror("fork error.");
                    close(conn_fd);
                    continue;
                }
                else if(pid==0){
                    //child
                    //1.close fd used in parent process
                    close(epoll_fd);
                    close(listen_fd);
                    close(sig_pipefd[0]);
                    close(sig_pipefd[1]);
                    close(users[user_count].pipefd[0]);
                    //2.call sub process
                    run_child(user_count,users,share_mem);
                    //3.release munmap
                    munmap(share_mem,BUFFER_SIZE*USER_LIMIT);
                    exit(0);
                }
                else{
                    //parent
                    //1.close fd used in sub process
                    close(conn_fd);//communicate through pipe,so close the socket fd after create client data
                    close(users[user_count].pipefd[1]);
                    //2.add fd into epoll
                    add_fd_to_epoll(epoll_fd,users[user_count].pipefd[0]);
                    //3.record sub process info
                    users[user_count].pid=pid;
                    sub_processes[pid]=user_count;
                    user_count++;
                }
            }
            else if(sock_fd==sig_pipefd[0]&&(epollevents[i].events&EPOLLIN)){//receive signal
                int sig;
                char signals[1024];
                ret=recv(sig_pipefd[0],signals,1024,0);
                if(ret==-1){
                    continue;
                }
                else if(ret==0){
                    continue;
                }
                else{
                    printf("new signal arrives.\n");
                    for(int i=0;i<ret;i++){
                        sig=signals[i];
                        switch (sig)
                        {
                        case SIGCHLD:{
                            //sub-process exit which means one client close the connection
                            pid_t pid;
                            int stat;
                            while((pid=waitpid(-1,&stat,WNOHANG))>0){
                                //get sub-process id:pid
                                int del_user=sub_processes[pid];
                                sub_processes[pid]=-1;
                                if((del_user<0)||(del_user>USER_LIMIT)){
                                    continue;
                                }
                                //clear connection with index of del_user
                                close(users[del_user].pipefd[0]);
                                epoll_ctl(epoll_fd,EPOLL_CTL_DEL,users[del_user].pipefd[0],0);
                                //clear info
                                users[del_user]=users[--user_count];
                                sub_processes[users[del_user].pid]=del_user;
                            }
                            if(terminate&&user_count==0){
                                stop_child=true;
                            }
                        }
                            break;
                        case SIGTERM:
                        case SIGINT:{
                            //terminal server
                            printf("kill server and all the children.\n");
                            if(user_count==0){
                                stop_child=true;
                                break;
                            }
                            for(int i=0;i<user_count;++i){
                                int pid=users[i].pid;
                                kill(pid,SIGTERM);
                            }
                            terminate=true;
                        }
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
            else if(epollevents[i].events&EPOLLIN){//receive message from sub-process,while sub-process receive data from client,sock_fd=sub_pipefd[0]
                int child=0;
                ret=recv(sock_fd,(char*)&child,sizeof(child),0);
                if(ret==-1||ret==0){
                    continue;
                }
                else{
                    printf("receive sub-process message.\n");
                    //notify other clients
                    for(int j=0;j<user_count;++j){
                        if(users[j].pipefd[0]!=sock_fd){
                            printf("sub-process notify client:%d\n",child);
                            send(users[j].pipefd[0],(char*)&child,sizeof(child),0);
                        }
                    }
                }
            }
        }
    }
    del_resources();
    return 0;
}