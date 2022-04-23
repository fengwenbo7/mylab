#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>

#define MAX_EVENT_NUM 1024
#define UDP_BUFFER_SIZE 1024
#define TCP_BUFFER_SIZE 512

void setnonblock(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
}

void add_fd_to_epoll(int epoll_fd,int target_fd){
    epoll_event event;
    event.data.fd=target_fd;
    event.events=EPOLLIN|EPOLLET;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,target_fd,&event);
    setnonblock(target_fd);
}

int main(){
    struct sockaddr_in listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_addr.s_addr=INADDR_ANY;
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(9999);
    //create tcp
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket tcp error.");
        return -1;
    }
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret<0){
        perror("bind error.");
        return -1;
    }
    ret=listen(listen_fd,5);
    if(ret<0){
        perror("listen error.");
        return -1;
    }
    //create udp
    int udp_fd=socket(AF_INET,SOCK_DGRAM,0);
    if(udp_fd<0){
        perror("socket udp error.");
        return -1;
    }
    ret=bind(udp_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret<0){
        perror("bind error.");
        return -1;
    }
    
    epoll_event events[MAX_EVENT_NUM];
    int epoll_fd=epoll_create(5);
    if(epoll_fd<0){
        perror("epoll error.");
        return -1;
    }
    add_fd_to_epoll(epoll_fd,listen_fd);
    add_fd_to_epoll(epoll_fd,udp_fd);
    int udp_msg_index=0;
    udp_msg_index++;
    while(1)
    {
        int event_size=epoll_wait(epoll_fd,events,MAX_EVENT_NUM,-1);
        if(event_size<0){
            perror("epoll_wait error.");
            return -1;
        }
        for(int i=0;i<event_size;i++){
            int sock_fd=events[i].data.fd;
            if(sock_fd==listen_fd){
                //tcp connect
                struct sockaddr_in client_addr;
                socklen_t len=sizeof(client_addr);
                int conn_fd=accept(sock_fd,(struct sockaddr*)&client_addr,&len);
                if(conn_fd<0){
                    perror("accept error.");
                    return -1;
                }
                else{
                    printf("receive tcp client connect\n");
                }
                add_fd_to_epoll(epoll_fd,conn_fd);
            }
            else if(sock_fd==udp_fd){
                //udp connect
                char buf[UDP_BUFFER_SIZE];
                memset(buf,'\0',UDP_BUFFER_SIZE);
                struct sockaddr_in send_addr;
                socklen_t len=sizeof(send_addr);
                ret=recvfrom(sock_fd,buf,UDP_BUFFER_SIZE,0,(struct sockaddr*)&send_addr,&len);
                if(ret>0){
                    printf("%d receive message from udp-client:%s\n",udp_msg_index++,buf);
                    sendto(sock_fd,buf,UDP_BUFFER_SIZE,0,(struct sockaddr*)&send_addr,len);
                }
            }
            else{
                //message
                if(events[i].events&EPOLLIN){
                    char buf[TCP_BUFFER_SIZE];
                    memset(buf,'\0',TCP_BUFFER_SIZE);
                    int len=recv(sock_fd,buf,TCP_BUFFER_SIZE,0);
                    if(len<0){
                        if(errno==EAGAIN||errno==EWOULDBLOCK){
                            break;
                        }
                        close(sock_fd);
                        break;
                    }
                    else if(len==0){
                        printf("tcp-client disconnect\n");
                        close(sock_fd);
                    }
                    else{
                        printf("receive message from tcp-client:%s",buf);
                        send(sock_fd,buf,ret,0);
                    }
                }
                else{
                    printf("something else happened \n");
                }
            }
        }
    }
    close(listen_fd);
    return 0;
}