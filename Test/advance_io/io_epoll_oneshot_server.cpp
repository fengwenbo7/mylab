#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>
#include <memory.h>

#define MAX_EVENT_NUM 1024
#define BUFFER_SIZE 1024

struct fds
{
    int epoll_fd;
    int sock_fd;
};

void add_fd_into_epoll_fds(int epollfd,int tarfd,bool oneshot){
    //add fd to epoll
    epoll_event event;
    event.data.fd=tarfd;
    event.events=EPOLLIN|EPOLLET;
    if(oneshot){
        event.events|=EPOLLONESHOT;
    }
    epoll_ctl(epollfd,EPOLL_CTL_ADD,tarfd,&event);
    //set non-block
    int old_option=fcntl(tarfd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(tarfd,F_SETFL,new_option);
}

//epoll events shot only once because we set the oneshot attribute,so we should set again after EAGAIN occurs
void reset_epolloneshot(int epoll_fd,int conn_fd){
    epoll_event event;
    event.data.fd=conn_fd;
    event.events=EPOLLIN|EPOLLET|EPOLLONESHOT;
    epoll_ctl(epoll_fd,EPOLL_CTL_MOD,conn_fd,&event);
}

void* worker(fds* fds){
    int conn_fd=fds->sock_fd;
    int epoll_fd=fds->epoll_fd;
    pthread_t pid=pthread_self();
    char buf[BUFFER_SIZE];
    memset(buf,'\0',BUFFER_SIZE);
    while(1){
        int ret=recv(conn_fd,buf,BUFFER_SIZE-1,0);
        printf("receive ret:%d\n",ret);
        if(ret==0){//close connection
            printf("client closed.\n");
            epoll_ctl(epoll_fd,EPOLL_CTL_DEL,conn_fd,NULL);//remove current socket from epoll
            close(conn_fd);
            break;
        }
        else if(ret<0){
            if(errno==EAGAIN){//can register again,not internet error
                printf("read later.\n");
                reset_epolloneshot(epoll_fd,conn_fd);
                break;
            }
            else{//error occurs
                perror("recv error\n");
                break;
            }
        }
        else{
            printf("[thread:%d] get messgae:%s\n",(int)pid,buf);
            sleep(5);//data handle
        }
    }
    printf("end thread receiving data on fd:%d",conn_fd);
    return NULL;
}

int main(){
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket error\n");
        return -1;
    }

    struct sockaddr_in listen_addr;
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_addr.s_addr=INADDR_ANY;
    listen_addr.sin_port=htons(9999);
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret==-1){
        perror("bind error\n");
        return -1;
    }

    ret=listen(listen_fd,5);
    if(ret==-1){
        perror("listen error\n");
        return -1;
    }

    epoll_event events[MAX_EVENT_NUM];
    int epoll_fd=epoll_create(5);
    if(epoll_fd<0){
        perror("epoll create error\n");
        return -1;
    }

    add_fd_into_epoll_fds(epoll_fd,listen_fd,false);

    while(1){
        ret=epoll_wait(epoll_fd,events,MAX_EVENT_NUM,-1);
        if(ret<0){
            perror("epoll wait error\n");
            break;
        }
        for(int i=0;i<ret;i++){
            int sock_fd=events[i].data.fd;
            if(sock_fd==listen_fd){
                struct sockaddr_in client_addr;
                socklen_t len=sizeof(client_addr);
                int conn_fd=accept(listen_fd,(sockaddr*)&client_addr,&len);
                add_fd_into_epoll_fds(epoll_fd,conn_fd,true);
                printf("client connect\n");
            }
            else if(events[i].events&EPOLLIN){
                printf("epoll in occurs\n");
                fds fd_new_process;
                fd_new_process.epoll_fd=epoll_fd;
                fd_new_process.sock_fd=sock_fd;
                int pid=fork();
                if(pid==0){
                    //child process
                    close(listen_fd);
                    worker(&fd_new_process);
                }
            }
            else{
                printf("something else happened\n");
            }
        }
    }
    close(listen_fd);
    return 0;
}