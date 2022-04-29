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

#define BUFFER_SIZE 1023

int set_nonblocking(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
    return old_option;
}

//connect function with timeout
int unblock_connect(const char* ip,int port,int time){
    struct sockaddr_in client_addr;
    bzero(&client_addr,sizeof(client_addr));
    client_addr.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&client_addr.sin_addr.s_addr);
    client_addr.sin_port=htons(port);

    int conn_fd=socket(PF_INET,SOCK_STREAM,0);
    int old_option=set_nonblocking(conn_fd);
    int ret=connect(conn_fd,(const sockaddr*)&client_addr,sizeof(client_addr));
    if(ret==0){
        printf("connect success.\nreset the option.\n");
        fcntl(conn_fd,F_SETFL,old_option);
        return conn_fd;
    }
    else if(errno!=EINPROGRESS){
        printf("unblock connect not support.\n");
        return -1;
    }
    //support unblocking connect,continue to handle---use select
    fd_set read_fds;
    fd_set write_fds;
    FD_ZERO(&write_fds);
    FD_SET(conn_fd,&write_fds);
    int ret=select(conn_fd+1,NULL,&write_fds,NULL,0);
    if(ret<=0){
        perror("select error\n");
        close(conn_fd);
        return -1;
    }
    if(!FD_ISSET(conn_fd,&write_fds)){
        printf("no events ready found.\n");
        close(conn_fd);
        return -1;
    }

    int errorno=0;
    socklen_t len=sizeof(errorno);

    //get and clear errors by calling getsockopt
    if(getsockopt(conn_fd,SOL_SOCKET,SO_ERROR,&errorno,&len)<0){
        printf("get socket option failed:%d\n",errorno);
        close(conn_fd);
        return -1;
    }

    //connect error when errorno doesn't euqal zeop
    if(errorno!=0){
        printf("connect failed after selecting with the error:%d\n",errorno);
        close(conn_fd);
        return -1;
    }

    //connect success
    printf("connect ready after selecting with the socket:%d\n",conn_fd);
    fcntl(conn_fd,F_SETFL,old_option);
    return conn_fd;
}

int main(){
    int sockfd=unblock_connect("127.0.0.1",9999,10);
    if(sockfd<0){
        return 1;
    }
    close(sockfd);
    return 0;
}