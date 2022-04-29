#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>

#define MAX_BUFFER_SIZE 1024
#define PIPE_BUFFER_SIZE 32768

int main(){
    int conn_fd=socket(PF_INET,SOCK_STREAM,0);
    struct sockaddr_in client_address;
    bzero(&client_address,sizeof(client_address));
    client_address.sin_family=AF_INET;
    client_address.sin_port=htons(9999);
    inet_pton(AF_INET,"127.0.0.1",&client_address.sin_addr.s_addr);
    socklen_t len=sizeof(client_address);
    int ret=connect(conn_fd,(const sockaddr*)&client_address,len);
    if(ret<0){
        perror("connect error.\n");
        close(conn_fd);
        return -1;
    }
    pollfd fds[2];
    fds[0].fd=conn_fd;
    fds[0].events=POLLIN|POLLRDHUP;
    fds[0].revents=0;
    fds[1].fd=0;
    fds[1].events=POLLIN;
    fds[1].revents=0;
    int pipefd[2];
    ret=pipe(pipefd);
    if(ret<0){
        perror("pipe error.\n");
        return -1;
    }
    char buffer[MAX_BUFFER_SIZE];
    while(1){
        ret=poll(fds,2,-1);
        if(ret<0){
            perror("poll error.\n");
            return -1;
        }
        if(fds[0].revents&POLLRDHUP){
            printf("server disconnected.\n");
            close(conn_fd);
        }
        else if(fds[0].revents&POLLIN){
            memset(buffer,'\0',sizeof(buffer));
            ret=recv(conn_fd,buffer,MAX_BUFFER_SIZE,0);
            if(ret>0){
                printf("receive server msg:%s",buffer);
            }
        }   
        else if(fds[1].revents&POLLIN){
            splice(0,NULL,pipefd[1],NULL,PIPE_BUFFER_SIZE,SPLICE_F_MORE|SPLICE_F_MOVE);
            splice(pipefd[0],NULL,conn_fd,NULL,PIPE_BUFFER_SIZE,SPLICE_F_MORE|SPLICE_F_MOVE);
        }
    }
    close(conn_fd);
    return 0;
}