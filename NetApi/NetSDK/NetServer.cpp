#include "NetBase.h"

class NetServer:public NetBase{
public:
    int HandleSocketMessage(int recvret,void *msg) override{
        if(recvret>0){
            char ip[32];
            printf("client[%s:%d]:%s",inet_ntoa(cust_addr.sin_addr),ntohs(cust_addr.sin_port),msg);
            send(own_fd,"roger.",7,0);
            return 0;
        }
        else if(recvret==0){
            printf("client disconnected...");
            return -1;
        }
        else{
            perror("receive error");
            return -1;
        }
    }
};