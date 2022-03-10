#include "../header/sockethelper.h"
#include <assert.h>

int SocketHelper::CreateSocket(){
    sfd = socket(AF_INET,SOCK_STREAM,0);
    if(sfd==-1){
        perror("socket error");
    }
    return sfd;
}

int SocketHelper::BindSocket(){
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(9999);
    saddr.sin_addr.s_addr=htonl(INADDR_ANY);
    int ret = bind(sfd,(const sockaddr*)&saddr,sizeof(saddr));
    if(ret==-1){
        perror("bind error");
    }
    return ret;
}

int SocketHelper::ListenSocket(){
    int ret=listen(sfd,10);
    if(ret==-1){
        perror("listen error");
    }
    return ret;
}

int SocketHelper::AcceptSocket(){
    socklen_t slen=sizeof(caddr);
    cfd = accept(sfd,(struct sockaddr*)&caddr,&slen);
    if(cfd==-1){
        perror("accept error");
    }
    return cfd;
}

int SocketHelper::HandleSocketMessage(){
    char ip[32];
    printf("client ip:%d,port:%d\n",caddr.sin_addr.s_addr,caddr.sin_port);
    printf("client ip:%s,port:%d\n",inet_ntoa(caddr.sin_addr),ntohs(caddr.sin_port));
    while(1){
        char buff[1024];
        char response[64]="roger.\n";
        int len = recv(cfd,buff,sizeof(buff),0);
        if(len>0){
            printf("client:%s\n",buff);
            send(cfd,response,sizeof(response),0);
        }
        else if(len==0){
            perror("client disconnected");
            break;
        }
        else{
            perror("receive error");
            break;
        }
    }
    close(sfd);
    close(cfd);
    return 0;
}

int main(){
    SocketHelper server;
    int ret = server.CreateSocket();
    ret = server.BindSocket();
    ret = server.ListenSocket();
    ret = server.AcceptSocket();
    ret = server.HandleSocketMessage();
}