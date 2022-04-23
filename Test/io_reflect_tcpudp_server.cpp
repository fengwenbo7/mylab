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

#define MAX_EVENT_NUM 1024
#define BUFFER_SIZE 1024

int main(){
    struct sockaddr_in listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_addr.s_addr=INADDR_ANY;
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(9999);
    //create tcp
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket tcp error.\n");
        return -1;
    }
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret<0){
        perror("bind error.\n");
        return -1;
    }
    ret=listen(listen_fd,5);
    if(ret<0){
        perror("listen error.\n");
        return -1;
    }
    //create udp
    int udp_fd=socket(AF_INET,SOCK_DGRAM,0);
    if(udp_fd<0){
        perror("socket udp error.\n");
        return -1;
    }
    
}