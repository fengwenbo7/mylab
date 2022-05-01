#include "ascend_order_list_timer.h"
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

#define __USE_POSIX199309
#define FD_LIMIT 65535
#define TIMESLOT 5
#define MAX_EVENT_NUM 1024
int pipefd[2];
//use ascend-order-list to manage timers
static sort_timer_list timer_list;
static int epoll_fd=0;

void addfdtoepoll(int epoll_fd,int tar_fd){
    epoll_event event;
    event.data.fd=tar_fd;
    event.events=EPOLLIN|EPOLLET;
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,tar_fd,&event);
}

void setnonblock(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,old_option);
}

void sig_handle(int sig){
    int saveerrno=errno;
    send(pipefd[1],(char*)&sig,1,0);//send sig to pipe write node(1)
    errno=saveerrno;
}

void addsig(int sig){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    //sa.__sigaction_handler=sig_handle;
    sa.sa_flags|=SA_RESTART;
    sigfillset(&sa.sa_mask);
    int ret=sigaction(sig,&sa,NULL);
    assert(ret!=-1);
}

//callback for timer,delete it from the list
void timer_cb(client_data* data){
    epoll_ctl(epoll_fd,EPOLL_CTL_DEL,data->sockfd,0);
    assert(data);
    close(data->sockfd);
    printf("timer tick,close fd:%d.\n",data->sockfd);
}

//1.首先创建一个管道，其用来写入信号
//2.为每个连接的客户端都分配一个定时器，设置超时时间(绝对时间),和超时回调函数(回调函数关闭连接)，并将定时器加入到升序双向链表中
//3.为了统一事件源，我们将信号管道的读端加入到epoll中，这样就方便监听所有客户连接和超时事件发生，然后进行处理
//4.在主循环中，判断是否是信号管道的读端文件描述符，如果是超时信号，就调用超时处理函数，不过是在所有I/O事件处理完成之后
//5.超时处理函数，检测定时器升序链表，若有超时事件，则调用回调函数，关闭连接，删除定时器
int main(){
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_port=htons(9999);
    inet_pton(AF_INET,"127.0.0.1",&address.sin_addr);

    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    assert(listen_fd>=0);

    int ret=bind(listen_fd,(struct sockaddr*)&address,sizeof(address));
    assert(ret!=-1);

    ret=listen(listen_fd,5);
    assert(ret!=-1);

    epoll_event events[MAX_EVENT_NUM];
    epoll_fd=epoll_create(5);
    assert(epoll_fd>0);
    addfdtoepoll(epoll_fd,listen_fd);

    //pipe(pipefd);
    ret=socketpair(AF_UNIX,SOCK_STREAM,0,pipefd);//create a local pipe
    std::cout<<"pipefd[0]:"<<pipefd[0]<<",pipefd[1]:"<<pipefd[1];
    assert(ret!=-1);
    setnonblock(pipefd[1]);
    addfdtoepoll(epoll_fd,pipefd[0]);//o-read-epoll listen

    //set signal to write pipe write node(1)
    addsig(SIGALRM);
    addsig(SIGTERM);

    alarm(TIMESLOT);//timer for SIGALRM
    bool timeout=false;

    //clients
    client_data* users=new client_data[FD_LIMIT];
    bool stop_server=false;
    while(!stop_server){
        int number=epoll_wait(epoll_fd,events,MAX_EVENT_NUM,-1);
        if(number<0){
            printf("epoll failed.\n");
            break;
        }
        for(int i=0;i<number;i++){
            int sockfd=events[i].data.fd;
            if(sockfd==listen_fd){//new client
                //record new client
                struct sockaddr_in client_addr;
                socklen_t len=sizeof(client_addr);
                int conn_fd=accept(sockfd,(struct sockaddr*)&client_addr,&len);
                assert(conn_fd!=-1);
                addfdtoepoll(epoll_fd,conn_fd);
                users[conn_fd].sockfd=conn_fd;
                users[conn_fd].address=client_addr;
                //create timer for list
                util_timer* newtimer=new util_timer;
                newtimer->user_data=&users[conn_fd];
                newtimer->cb_func=timer_cb;
                newtimer->expire=time(NULL)+3*TIMESLOT;
                users[conn_fd].timer=newtimer;
                timer_list.add_timer(newtimer);
            }
            else if(sockfd==pipefd[0]&&(events[i].events&EPOLLIN)){//receive signals
                char signals[1024];
                ret=recv(sockfd,signals,sizeof(signals),0);
                if(ret==-1){
                    perror("signal error.");
                    //handle error
                    continue;
                }
                else if(ret==0){
                    continue;
                }
                else{
                    for(int j=0;j<ret;j++){
                        switch (signals[j])
                        {
                        case SIGALRM://timer to handle
                            timeout=true;
                            break;
                        case SIGTERM:
                            stop_server=true;
                            break;
                        default:
                            break;
                        }
                    }
                }
            }
            else if(events[i].events&EPOLLIN){//receive client message
                memset(users[sockfd].buf,'\0',BUFFER_SIZE);
                ret=recv(sockfd,users[sockfd].buf,BUFFER_SIZE-1,0);
                printf("get %dbytes of client data %s from %d\n",ret,users[sockfd].buf,sockfd);
                util_timer* timer=users[sockfd].timer;
                if(ret<0){
                    if(errno!=EAGAIN){
                        //close and delete
                        timer_cb(&users[sockfd]);
                        if(timer){
                            timer_list.del_timer(timer);
                        }
                    }
                }
                else if(ret==0){
                    //close because client disconnect
                    timer_cb(&users[sockfd]);
                    if(timer){
                        timer_list.del_timer(timer);
                    }
                }
                else{
                    //data to read means active,so adjust the timer
                    if(timer){
                        timer->expire=time(NULL)+3*TIMESLOT;
                        timer_list.adjust_timer(timer);
                        printf("adjust timer.\n");
                    }
                }
            }
            else{
                //others
            }
        }
        if(timeout){
            timer_list.tick();
            alarm(TIMESLOT);
            timeout=false;
        }
    }
    close(listen_fd);
    close(pipefd[0]);
    close(pipefd[1]);
    delete[] users;
    return 0;
}