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

int main(int argc,char** argv){
    int listen_fd,conn_fd;
    pid_t child_pid;
    socklen_t child_len;
    struct sockaddr_in child_addr,server_addr;

    listen_fd=socket(AF_INET,SOCK_STREAM,0);
    bzero(&server_addr,sizeof(server_addr));
    server_addr.sin_family=AF_INET;
    server_addr.sin_port=htons(SERV_PORT);//the server's well-known port
    server_addr.sin_addr.s_addr=htonl(INADDR_ANY);

    bind(listen_fd,(const sockaddr*)&server_addr,sizeof(server_addr));
    listen(listen_fd,LISTENQ);
    while(true){
        child_len=sizeof(child_addr);
        conn_fd=accept(listen_fd,(sockaddr*)&child_addr,&child_len);
        if((child_pid=fork())==0){
            //child
            close(listen_fd);
            str_echo(conn_fd);
            exit(0);//0-exit normally,1-exit exceptionally
        }
        close(conn_fd);
    }
}