#include <iostream>     // std::cout、std::endl
#include <sys/socket.h> // socket、bind、listen、accept
#include <netinet/in.h> // htonl、htons
#include <unistd.h>     // write、close
#include <string.h>     // bzero
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/epoll.h>
#include <memory.h>
#include <fcntl.h>

#include <iostream>
#include "ContanceManager.h"
using namespace std;

#define PORT 8099
#define LISTENQ 5
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
        //printf("receive ret:%d\n",ret);
        if(ret==0){//close connection
            printf("client closed.\n");
            epoll_ctl(epoll_fd,EPOLL_CTL_DEL,conn_fd,NULL);//remove current socket from epoll
            close(conn_fd);
            break;
        }
        else if(ret<0){
            if(errno==EAGAIN){//can register again,not internet error
                //printf("read later.\n");
                reset_epolloneshot(epoll_fd,conn_fd);
                break;
            }
            else{//error occurs
                perror("recv error\n");
                break;
            }
        }
        else{
            printf("[thread:%d] get messgae:%s",(int)pid,buf);
            if(strcmp(buf,"Connect")){
                std::string msg_to_send("input fucntion.1.new 2.search");
                send(conn_fd,msg_to_send.c_str(),strlen(msg_to_send.c_str()),0);
                std::cout<<"send to client:"<<msg_to_send<<std::endl;
            }
        }
    }
    //printf("end thread receiving data on fd:%d",conn_fd);
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
    listen_addr.sin_port=htons(PORT);
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret==-1){
        perror("bind error\n");
        return -1;
    }

    ret=listen(listen_fd,LISTENQ);
    if(ret==-1){
        perror("listen error\n");
        return -1;
    }

    epoll_event events[MAX_EVENT_NUM];
    int epoll_fd=epoll_create(LISTENQ);
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

int _main()
{
    ContactManager cm("ContactDat.dat", "ResumeDat.dat");
    while (true)
    {
        std::cout << "input function:" << std::endl;
        std::cout << "1.new" << std::endl;
        std::cout << "2.search" << std::endl;
        int func_type;
        std::cin >> func_type;
        switch (func_type)
        {
        case 1:
        {
            PersonInfoWithResume full_info;
            PersonInfo info;
            std::cout << "name:";
            std::cin >> info.name;
            std::cout << "age:";
            std::cin >> info.age;
            std::cout << "gender:";
            std::cin >> info.gender;
            std::cout << "phonenumber:";
            std::cin >> info.phonenumber;
            std::cout << "email:";
            std::cin >> info.email;
            std::cout << "resume:";
            std::cin >> full_info.resume;
            std::cout << "save to dat" << std::endl;
            full_info.person_info = info;
            info.resume_length = strlen(full_info.resume);
            std::cout << "save to dat,resume size:" << info.resume_length << std::endl;
            cm.SaveData(full_info);
        }
        break;
        case 2:
        {
            std::cout << "search name:";
            string in_name;
            std::cin >> in_name;

            std::cout << "search phone number:";
            string in_phonenum;
            std::cin >> in_phonenum;

            std::cout << "search email:";
            string in_email;
            std::cin >> in_email;

            list<PersonInfoWithResume> results = cm.SearchFromDat(in_name.c_str(), in_phonenum.c_str(), in_email.c_str(), true);
            for (auto iter = results.begin(); iter != results.end(); iter++)
            {
                std::cout << "name:" << iter->person_info.name << " "
                          << "age:" << iter->person_info.age << " "
                          << "phonenumber:" << iter->person_info.phonenumber << " "
                          << "email:" << iter->person_info.email << " "
                          << "resume:" << iter->resume << std::endl;
            }
        }
        break;
        default:
            break;
        }
    }
}