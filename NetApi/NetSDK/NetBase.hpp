#ifndef _SOCKET_HELPER
#define _SOCKET_HELPER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

class NetBase{
public:
    //for server
    int virtual CreateSocketPeer(){
        return 0;
    }
    //for client
    int virtual CreateSocketPeer(char* _cp,int len){
        return 0;
    }
protected:
    //create socket for listen in server
    int virtual CreateSocket(){
        listen_fd=socket(AF_INET,SOCK_STREAM,0);
        printf("create server socket...\n");
        return listen_fd;
    }

    //create socket for listen in client
    int virtual CreateSocket(char* _cp,int len){
        communication_fd=socket(AF_INET,SOCK_STREAM,0);
        printf("create client socket...\n");
        if(communication_fd!=-1){
            memset(communication_ip,'\0',sizeof(communication_ip));
            if(sizeof(communication_ip)>len){
                strcpy(communication_ip,_cp);
            }
        }
        return communication_fd;
    }

    //bind socket in server
    int virtual BindSocket(){
        own_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        own_addr.sin_port=htons(9999);
        own_addr.sin_family=AF_INET;
        printf("bind listen socket...\n");
        return bind(listen_fd,(const sockaddr*)(&own_addr),sizeof(own_addr));
    }

    //listen socket in server
    int virtual ListenSocket(){
        printf("start listen...\n");
        return listen(listen_fd,127);
    }

    //accept connect from client in server
    int virtual AcceptSocket(){
        socklen_t len=sizeof(cust_addr);
        printf("start accept...\n");
        communication_fd=accept(listen_fd,(sockaddr*)(&cust_addr),&len);
        if(communication_fd!=-1){
            SendSocketMessage(greet,strlen(greet));
        }
        return communication_fd;
    }

    //connect to server in client
    int virtual ConnectSocket(){
        cust_addr.sin_family=AF_INET;
        cust_addr.sin_port=htons(9999);
        inet_pton(AF_INET,communication_ip,&cust_addr.sin_addr.s_addr);
        printf("connect.\n");
        return connect(communication_fd,(const sockaddr*)(&cust_addr),sizeof(cust_addr));
    }

    //receive msg from cust
    int virtual ReceiveSocketMsg(){
        while(1){
            char msg[1024];
            int msgret=recv(communication_fd,msg,sizeof(msg),0);
            if(HandleSocketMessage(msgret,msg)==-1){
                break;
            }
        }
        if(communication_fd){
            close(communication_fd);
        }
        if(listen_fd){
            close(listen_fd);
        }
        return 0;
    }

    int virtual SendSocketMessage(const char* messgae,int length){
        if(communication_fd!=-1){
            send(communication_fd,messgae,length,0);
        }
        else{
            return -1;
        }
        return 0;
    }

    //handle msg in server&client,need override
    int virtual HandleSocketMessage(int recvret,void *msg){
        return 0;
    }
protected:
    const char greet[19]="hello,i am server.";
    const char respon[7]="roger.";
    int listen_fd;//socket for listen
    int communication_fd;//socket for communication
    char communication_ip[64]={0};
    struct sockaddr_in own_addr;//server address
    struct sockaddr_in cust_addr;//client address
};

#endif