#ifndef _SOCKET_HELPER
#define _SOCKET_HELPER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

class NetBase{
public:
    int virtual CreateSocketPeer(){
        return 0;
    }
    int virtual CreateSocketPeer(const char* _cp){
        return 0;
    }
protected:
    //create socket for server&client
    int virtual CreateSocket(){
        own_fd=socket(AF_INET,SOCK_STREAM,0);
        return own_fd;
    }

    //bind socket in server
    int virtual BindSocket(){
        own_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        own_addr.sin_port=htons(9999);
        own_addr.sin_family=AF_INET;
        return bind(own_fd,(const sockaddr*)(&own_addr),sizeof(own_addr));
    }

    //listen socket in server
    int virtual ListenSocket(){
        return listen(own_fd,127);
    }

    //accept connect from client in server
    int virtual AcceptSocket(){
        socklen_t len=sizeof(cust_addr);
        cust_fd=accept(own_fd,(sockaddr*)(&cust_addr),&len);
        return cust_fd;
    }

    //connect to server in client
    int virtual ConnectSocket(const char* _cp){
        cust_addr.sin_family=AF_INET;
        cust_addr.sin_port=htons(9999);
        inet_pton(AF_INET,_cp,&cust_addr.sin_addr.s_addr);
        return connect(own_fd,(const sockaddr*)(&cust_addr),sizeof(cust_addr));
    }

    //receive msg from cust
    int virtual ReceiveSocketMsg(){
        while(1){
            char msg[1024];
            int msgret=recv(own_fd,msg,sizeof(msg),0);
            if(HandleSocketMessage(msgret,msg)==-1){
                break;
            }
        }
        if(own_fd){
            close(own_fd);
        }
        if(cust_fd){
            close(cust_fd);
        }
    }

    //handle msg in server&client,need override
    int virtual HandleSocketMessage(int recvret,void *msg){
        return 0;
    }
protected:
    int own_fd;//server socket instance
    int cust_fd;//client socket instance
    struct sockaddr_in own_addr;//server address
    struct sockaddr_in cust_addr;//client address
};

#endif