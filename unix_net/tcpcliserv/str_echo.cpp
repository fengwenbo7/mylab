#include "../unp.h"

void str_echo(int sock_fd){
    ssize_t n;
    char buf[MAXLINE];
again:
    while((n=read(sock_fd,buf,MAXLINE))>0)
        Writen(sock_fd,buf,n);
    if(n<0&&errno==EINTR)
        goto again;
    else if(n<0);
        err_sys("str_echo:read error.");
}