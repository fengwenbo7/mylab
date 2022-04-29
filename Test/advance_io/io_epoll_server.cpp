#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <memory.h>
#include <fcntl.h>

#define MAX_EVENT_NUM 1024

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

int main(){
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd==-1){
        perror("socket error\n");
        return -1;
    }
    struct sockaddr_in listen_addr;
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(9999);
    listen_addr.sin_addr.s_addr=INADDR_ANY;
    int ret=bind(listen_fd,(const struct sockaddr*)&listen_addr,sizeof(listen_addr));
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
            if(sock_fd==listen_fd){//events of listen_fd invoke,which means client connect,so we should add client_fd(conn_fd) into the epoll-lists
                struct sockaddr_in client_addr;
                socklen_t len=sizeof(client_addr);
                int conn_fd=accept(sock_fd,(sockaddr*)&client_addr,&len);
                add_fd_into_epoll_fds(epoll_fd,conn_fd);
            }
            else if(events[i].events&EPOLLIN){//message read ready
                char buf[1024];
                memset(buf,'\0',sizeof(buf));
                ret=recv(sock_fd,buf,sizeof(buf)-1,0);
                if(ret<0){
                    close(sock_fd);
                    break;
                }
                else if(ret==0){
                    break;
                }
                else{
                    printf("client data:%s\n",buf);
                }
            }
            else{
                printf("something else happened.\n");
            }
        }
    }
    close(listen_fd);
    return 0;
}