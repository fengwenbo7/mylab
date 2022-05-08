#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <sys/stat.h>

int main(){
    int ret=0;
    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    address.sin_port=htons(9999);
    inet_pton(AF_INET,"127.0.0.1",&address.sin_addr);

    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket error.");
        return -1;
    }

    ret=bind(listen_fd,(const struct sockaddr*)&address,sizeof(address));
    if(ret==-1){
        perror("bind error.");
        return -1;
    }

    ret=listen(listen_fd,5);
    if(ret==-1){
        perror("listen error.\n");
        return -1;
    }
}