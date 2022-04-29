#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_EVENT_NUM 1024
#define __USE_POSIX199309
#define __USE_XOPEN_EXTENDED

static int pipefd[2];

void addfd(int epoll_fd,int targetfd){
    epoll_event event;
    event.data.fd=targetfd;
    event.events=EPOLLIN|EPOLLET;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,targetfd,&event);
    setnonblocking(targetfd);
}

void setnonblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
}

void sig_handler(int sig){
    //save primary errno,retain it in the end to ensure the reentered
    int save_errno=errno;
    int msg=sig;
    send(pipefd[1],&msg,1,0);//write the signal to pipe to notify the main loop.
    errno=save_errno;
}

void addsig(int sig){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_flags|=SA_RESTART;
    //sa.__sigaction_handler.sa_handler=sig_handler;
    int ret=sigaction(sig,&sa,NULL);
    if(ret==-1){
        perror("sigaction error.");
    }
}

int main(){
    struct sockaddr_in listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(9999);

    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket error.");
        return -1;
    }
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret<0){
        perror("bind error.");
        return -1;
    }
    ret=listen(listen_fd,5);
    if(ret<0){
        perror("{listen error.");
        return -1;
    }

    epoll_event events[MAX_EVENT_NUM];
    int epoll_fd=epoll_create(5);
    if(epoll_fd<0){
        perror("epoll create error.");
        return -1;
    }
    addfd(epoll_fd,listen_fd);//add listen_fd to epoll

    ret=socketpair(PF_UNIX,SOCK_STREAM,0,pipefd);//create pipe by socketpair,and register read ready in pipefd
    if(ret<0){
        perror("socketpair error.");
        return -1;
    }
    setnonblocking(pipefd[1]);//input(write),for signal
    addfd(epoll_fd,pipefd[0]);//output(read),for epoll

    //set handler for signals
    addsig(SIGHUP);
    addsig(SIGCHLD);
    addsig(SIGTERM);
    addsig(SIGINT);

    bool stop_server=false;
    while(!stop_server){
        int num=epoll_wait(epoll_fd,events,MAX_EVENT_NUM,-1);
        if(num<0&&errno!=EINTR){
            printf("epoll failed.\n");
            break;
        }
        for(int i=0;i<num;i++){
            int sockfd=events[i].data.fd;
            if(sockfd==listen_fd){
                //new client
                struct sockaddr_in client_addr;
                socklen_t len=sizeof(client_addr);
                int conn_fd=accept(sockfd,(sockaddr*)&client_addr,&len);
                addfd(epoll_fd,conn_fd);
            }
            else if(sockfd==pipefd[0]&&events[i].events&EPOLLIN){
                //receive signals,so we should handle that
                int sig;
                char signals[1024];
                ret=recv(sockfd,signals,sizeof(signals),0);
                if(ret==-1||ret==0){
                    continue;
                }
                else{
                    //receive signal per byte
                    for(int i=0;i<ret;i++){
                        switch (signals[i])
                        {
                        case SIGCHLD:
                        case SIGHUP:{
                            continue;
                        }
                        case SIGTERM:
                        case SIGINT:{
                            stop_server=true;
                        }
                        default:
                            break;
                        }
                    }
                }
            }
        }
    }

    printf("close fds\n");
    close(listen_fd);
    close(pipefd[0]);
    close(pipefd[1]);
    return 0;
}