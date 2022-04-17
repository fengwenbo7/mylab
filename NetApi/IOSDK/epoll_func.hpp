#include <sys/types.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <pthread.h>

#define MAX_EVENT_NUMBER 1024
class EpollFunc{
public:
//set no-block
static int set_non_blocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

//register the epollin event of fd into epoll kernal events
static void add_fd(int epoll_fd,int fd,bool enable_et){
    epoll_event e_event;
    e_event.events=EPOLLIN;
    e_event.data.fd=fd;
    if(enable_et){
        e_event.events|=EPOLLET;
    }
    epoll_ctl(epoll_fd,EPOLL_CTL_ADD,fd,&e_event);
    set_non_blocking(fd);
}

//workfolw of LF
static void lt_func(epoll_event* events,int num,int epoll_fd,int listen_fd){
    char buf[BUFFER_SIZE];
    for(int i=0;i<num;i++){
        int sock_fd=events[i].data.fd;
        if(sock_fd==listen_fd){

        }
        else if(events[i].events&EPOLLIN){

        }
        else{

        }
    }
}

static void et_func(epoll_event* events,int num,int epoll_fd,int listen_fd){
    char buf[BUFFER_SIZE];
    for(int i=0;i<num;i++){
        int sock_fd=events->data.fd;
        if(sock_fd==listen_fd){

        }
        else if(events[i].events&EPOLLIN){

        }
        else{

        }
    }
}
};