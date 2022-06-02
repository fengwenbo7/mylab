#ifndef PROCESS_POOL_H
#define PROCESS_POOL_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

class process{
public:
    process():pid_(-1){}
private:
    int pid_;
    int pipefd[2];//communication with parent
};

template<typename T> 
class processpool{
private:
    processpool(int listenfd,int process_number=8);
public:
    static processpool<T>* create(int listenfd,int process_number=8){
        if(!instance_){
            instance_=new processpool<T>(listenfd,process_number);
        }
        return instance_;
    }
    ~processpool(){

    }
    void run();

private:
    void run_parent();
    void run_child();

private:
    static const int MAX_PROCESS_NUMBER=16;//max process number allowed this pool
    static const int USER_PER_PROCESS=65535;//max count of users that each process can handle
    static const int MAX_EVENT_NUMBER=10000;//max count of events that epoll can handle
    static processpool<T>* instance_;
    process* sub_process_;

    int process_number_;
    int epoll_fd_;
    int listen_fd_;
    int index_;//the index of sub-process itself in sub_process list which works in child process.

    bool sub_stop_;
};

template<typename T>
processpool<T>* processpool<T>::instance_=NULL;

template<typename T>
processpool<T>::processpool(int listenfd,int process_number):listen_fd_(listenfd),process_number_(process_number),sub_stop_(false){
    //init sub-processes
    assert(process_number_>0&&process_number_<MAX_PROCESS_NUMBER);
    sub_process_=new process[process_number_];
    assert(sub_process_);
    //init each process in sub-processes
    for(int i=0;i<process_number_;i++){
        int ret=socketpair(PF_UNIX,SOCK_STREAM,0,sub_process_[i].pipefd);
        assert(ret==0);
        sub_process_[i].pid_=fork();
        assert(sub_process_[i].pid_>=0);
        if(sub_process_[i].pid_>0){
            //parent
            close(sub_process_[i].pipefd[1]);
            continue;
        }
        else{
            //child
            close(sub_process_[i].pipefd[0]);
            index_=i;
            break;
        }
    }
}

template<typename T>
void processpool<T>::run(){
    if(index_>0){
        run_child();
        return;
    }
    run_parent();
}

static int sig_pipefd[2];//pipe for signal handle

void setnonblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
}

static void add_fd_epoll(int epollfd,int fd){
    epoll_event event;
    event.data.fd=fd;
    event.events=EPOLLIN|EPOLLET;
    epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
    setnonblocking(fd);
}

static void sig_handle(int sig){
    int save_errno=errno;
    int msg=sig;
    send(sig_pipefd[1],&msg,sizeof(msg),0);
    errno=save_errno;
}

static void add_sig(int sig,void(handler)(int),bool restart=true){
    struct sigaction sa;
    memset(&sa,'\0',sizeof(sa));
    sa.sa_handler=handler;
    if(restart){
        sa.sa_flags|=SA_RESTART;
    }
    sigfillset(&sa.sa_mask);
    int ret=sigaction(sig,&sa,NULL);
    assert(ret!=-1);
}

static void set_sig(){

}

template<typename T>
void processpool<T>::run_child(){
    //add conn_fd to epoll
    //add sig to epoll
}

template<typename T>
void processpool<T>::run_parent(){
    
}

#endif