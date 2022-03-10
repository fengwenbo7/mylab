#include "sockethelper.h"
#include <assert.h>
#include <cstring>

int SocketHelper::CreateSocket(){
    cfd = socket(AF_INET,SOCK_STREAM,0);
    if(cfd==-1){
        perror("socket error");
    }
    return cfd;
}

int SocketHelper::ConnectSocket(const char* _cp){
    struct sockaddr_in saddr;
    saddr.sin_family=AF_INET;
    saddr.sin_port=htons(9999);
    inet_pton(AF_INET,_cp,&saddr.sin_addr.s_addr);
    int ret = connect(cfd,(const sockaddr*)&saddr,sizeof(saddr));
    if(ret==-1){
        perror("connect error");
    }
    return ret;
}

int SocketHelper::HandleSocketMessage(){
    int num=0;
    while(1){
        num++;
        //send data
        char buff[1024];
        sprintf(buff,"hello server,%d...\n",num);
        send(cfd,buff,strlen(buff)+1,0);
        int relen=recv(cfd,buff,sizeof(buff),0);
        if(relen>0){
            printf("server:%s\n",buff);
        }
        else if(relen==0){
            printf("server disconnected...\n");
            break;
        }
        else{
            perror("receive error");
            break;
        }
        sleep(1);
    }
    close(cfd);
    return 0;
}

int main(){
    SocketHelper client;
    client.CreateSocket();
    client.ConnectSocket("192.168.56.128");
    client.HandleSocketMessage();
}