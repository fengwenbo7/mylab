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
    int conn_fd;//file description
    struct sockaddr_in adress;//address of client
    pid_t pid;//process for this client
    int pipefd[2];//parent process communicates with parent process
};

epoll_event epollevents[EPOLL_EVENTS_NUM];
static const char* shm_name="/my_shm";
int pipefd[2];
int epoll_fd;
int listen_fd;
int shm_fd;
char* share_mem=0;//shared memory
client_data* users=0;//container of client
int* sub_processes=0;//processes for each client
int user_count=0;//the count of users

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
    send(pipefd[1],&msg,1,0);
    errno=save_errno;
}

void add_sig(int sig){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler=sig_handler;
    sa.sa_flags|=SA_RESTART;
    sigfillset(&sa.sa_mask);
    assert(sigaction(sig,&sa,NULL)!=-1);
}

void del_resources(){
    close(pipefd[0]);
    close(pipefd[1]);
    close(epoll_fd);
    close(listen_fd);
    shm_unlink(share_mem);
    delete[] users;
    delete[] sub_processes;
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
    ret=socketpair(PF_INET,SOCK_STREAM,0,pipefd);
    assert(ret!=-1);
    set_unblocking(pipefd[1]);
    add_fd_to_epoll(epoll_fd,pipefd[0]);

    //add sig handler
    add_sig(SIGCHLD);
    add_sig(SIGTERM);
    add_sig(SIGINT);
    add_sig(SIGPIPE);

    //read buffer of memory shared for all users
    shm_fd=shm_open(shm_name,O_CREAT|O_EXCL,0666);
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
        int num=epoll_wait(epoll_fd,epollevents,EPOLL_EVENTS_NUM,0);
        if(num<0&&errno!=EINTR){
            perror("epoll_wait error.");
            break;
        }
        for(int i=0;i<num;i++){
            int sock_fd=epollevents[i].data.fd;
            if(sock_fd==listen_fd){
                //new client
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
                    //2.call sub process
                    //3.release munmap
                }
                else{
                    //parent
                    //1.close fd used in sub process
                    //2.add fd into epoll
                    //3.record sub process info
                }
            }
            else if(sock_fd==pipefd[0]&&(epollevents[i].events&EPOLLIN)){
                //receive signal
            }
            else if(epollevents[i].events&EPOLLIN){
                //receive message
            }
        }
    }
    del_resources();
    return 0;
}