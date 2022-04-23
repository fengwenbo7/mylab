#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <poll.h>

#define SERVER_PORT 9999
#define BUFFER_SIZE 1024

int main(){
    char buf[BUFFER_SIZE];
    struct sockaddr_in conn_addr;
    conn_addr.sin_family=AF_INET;
    conn_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    conn_addr.sin_port=htons(SERVER_PORT);
    int conn_fd=socket(AF_INET,SOCK_DGRAM,0);
    if(conn_fd<0){
        perror("socket error.");
        return -1;
    }

    //bind is not need as sender
    int pipefd[2];
    int ret=pipe(pipefd);
    pollfd fds[1];
    fds[0].fd=0;
    fds[0].events=POLLIN;
    fds[0].revents=0;
    if(ret<0){
        perror("pipe error.");
        return -1;
    }
    while(1){
        ret=poll(fds,1,-1);
        if(ret<0){
            perror("poll error.");
            return -1;
        }
        if(fds[0].revents&POLLIN){
            //splice(0,NULL,pipefd[1],NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
            //splice(pipefd[0],NULL,conn_fd,NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
            //printf("has data input.\n");
            char buf[BUFFER_SIZE];
            memset(buf,'\0',sizeof(buf));
            read(0,buf,BUFFER_SIZE);
            printf("send to server with message:%s\n",buf);
            sendto(conn_fd,buf,sizeof(buf),0,(sockaddr*)&conn_addr,sizeof(conn_addr));
        }
    }
}