#include "NetBase.hpp"

class NetServer:public NetBase{
protected:
    int HandleSocketMessage(int recvret,void *msg) override{
        if(recvret>0){
            char ip[32];
            printf("client[%s:%d]:%s\n",inet_ntoa(cust_addr.sin_addr),ntohs(cust_addr.sin_port),(char*)msg);
            return 0;
        }
        else if(recvret==0){
            printf("client disconnected...\n");
            return -1;
        }
        else{
            perror("receive error\n");
            return -1;
        }
        return 0;
    }
public:
    int CreateSocketPeer() override{
        if(CreateSocket()==-1){
            perror("socket error");
            return -1;
        }
        if(BindSocket()==-1){
            perror("bind error");
            return -1;
        }
        if(ListenSocket()==-1){
            perror("listen error");
            return -1;
        }
        if(AcceptSocket()==-1){
            perror("accept error");
            return -1;
        }
        return ReceiveSocketMsg();
    }        
};