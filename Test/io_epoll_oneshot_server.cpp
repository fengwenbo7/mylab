#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <pthread.h>
#include <fcntl.h>
#include <unistd.h>
#include <iostream>

#define MAX_EVENT_NUM 1024
#define BUFFER_SIZE 1024

struct fds
{
    int epoll_fd;
    int sock_fd;
};

void add_fd_into_epoll_fds(int epollfd,int tarfd){
    //add fd to epoll
    epoll_event event;
    event.data.fd=tarfd;
    event.events=EPOLLIN|EPOLLET|EPOLLONESHOT;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,tarfd,&event);
    //set non-block
    int old_option=fcntl(tarfd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(tarfd,F_SETFL,new_option);
}

void* worker(void* arg){
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

    add_fd_into_epoll_fds(epoll_fd,listen_fd);

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
                add_fd_into_epoll_fds(epoll_fd,sock_fd);
            }
            else if(events[i].events&EPOLLIN){
                pthread_t thread;
                fds fd_new_thread;
                fd_new_thread.epoll_fd=epoll_fd;
                fd_new_thread.sock_fd=sock_fd;
                pthread_create(&thread,NULL,worker,&fd_new_thread);
            }
            else{
                printf("something else happened\n");
            }
        }
    }
    close(listen_fd);
    return 0;
}