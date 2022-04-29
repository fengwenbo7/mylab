#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <iostream>

int timeout_connect(const char* ip,int port,int time){
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    inet_pton(AF_INET,ip,&address.sin_addr);
    address.sin_port=htons(port);

    int sock_fd=socket(AF_INET,SOCK_STREAM,0);
    assert(sock_fd>=0);

    struct timeval timeout;
    timeout.tv_sec=time;
    timeout.tv_usec=0;
    socklen_t len=sizeof(timeout);
    int ret=setsockopt(sock_fd,SOL_SOCKET,SO_SNDTIMEO,&timeout,len);
    assert(ret!=-1);

    ret=connect(sock_fd,(struct sockaddr*)&address,sizeof(address));
    if(ret==-1){
        if(errno==EINPROGRESS){//errno for timeout
            std::cout<<"connect timeout,process timeout logic."<<std::endl;
            return -1;
        }
        printf("error occur when connectint to server.\n");
        return -1;
    }
    return sock_fd;
}

int main(){
    timeout_connect("127.0.0.1",9999,1);
    return 0;
}