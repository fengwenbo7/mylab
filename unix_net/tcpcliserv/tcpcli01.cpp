#include "../unp.h"

int main(int argc,char **argv)
{
    int sock_fd;
    struct sockaddr_in servaddr;
    if(argc!=2){
        err_quit("usage:tcpcli");
    }
    sock_fd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&servaddr,sizeof(servaddr));
    servaddr.sin_family=AF_INET;
    servaddr.sin_port=htons(SERV_PORT);
    inet_pton(AF_INET,argv[1],&servaddr.sin_addr);

    connect(sock_fd,(SA*)&servaddr,sizeof(servaddr));
    str_cli(stdin,sock_fd);
    exit(0);
}