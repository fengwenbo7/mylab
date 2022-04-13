#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <poll.h>

using namespace std;

int main(){
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<=0){
        perror("socket error");
        return -1;
    }
    struct sockaddr_in listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(9999);
    listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret<=0){
        perror("bind error");
        return -1;
    }
    ret=listen(listen_fd,5);
    if(ret<=0){
        perror("listen error");
        return -1;
    }
    pollfd fds[5];
}