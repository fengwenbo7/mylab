#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <unistd.h>

using namespace std;

int main(){
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket error");
        return -1;
    }
    struct sockaddr_in listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(9999);
    listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret<0){
        perror("bind error");
        return -1;
    }
    ret=listen(listen_fd,5);
    if(ret<0){
        perror("listen error");
        return -1;
    }
    
    char buf[1024];
    //fd_set
    fd_set read_fd;
    fd_set exception_fd;
    FD_ZERO(&read_fd);
    FD_ZERO(&exception_fd);
    //1.for single peer
    struct sockaddr_in client_addr;
    socklen_t len=sizeof(client_addr);
    int conn_fd=accept(listen_fd,(sockaddr*)&listen_addr,&len);
    if(conn_fd<0){
        perror("listen error");
        close(conn_fd);
        return -1;
    }
    while(1){
        FD_SET(conn_fd,&read_fd);
        FD_SET(conn_fd,&exception_fd);
        ret = select(conn_fd+1,&read_fd,NULL,&exception_fd,NULL);
        if(ret<0){
            perror("select error");
            break;
        }
        else if(ret==0){
            printf("select ret == 0.\n");
        }
        if(FD_ISSET(conn_fd,&read_fd)){
            if(recv(conn_fd,buf,sizeof(buf)-1,0)>0){
                printf("client data:%s\n",(char*)buf);
            }
            else{
                break;
            }
        }
        else if(FD_ISSET(conn_fd,&exception_fd)){
            
        }
    }
    return 0;
    //for multi peers
    int fds[5];
    int max_fd=0;
    for(int i=0;i<5;i++){
        struct sockaddr_in client_addr;
        socklen_t len=sizeof(client_addr);
        int conn_fd=accept(listen_fd,(sockaddr*)&listen_addr,&len);
        if(conn_fd<0){
            perror("listen error");
            return -1;
        }
        fds[i]=conn_fd;
        max_fd=max_fd<conn_fd?conn_fd:max_fd;
    }
    while(1){
        for(int i=0;i<5;i++){
            FD_SET(fds[i],&read_fd);
            FD_SET(fds[i],&exception_fd);
        }
        select(max_fd,&read_fd,NULL,&exception_fd,NULL);
        for(int i=0;i<5;i++){
            char buf[1024];
            if(FD_ISSET(fds[i],&read_fd)){
                if(recv(conn_fd,buf,sizeof(buf)-1,0)>0){
                    printf("client data:%s\n",(char*)buf);
                }
            }
            else if(FD_ISSET(fds[i],&exception_fd)){

            }
        }
    }
}