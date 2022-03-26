#include "NetBase.h"

class NetClient:public NetBase{
public:
    int HandleSocketMessage(int recvret,void *msg) override{
        if(recvret>0){
            printf("server:%s",msg);
            send(own_fd,"roger.",7,0);
            return 0;
        }
        else if(recvret==0){
            printf("server disconnected...");
            return -1;
        }
        else{
            perror("receive error");
            return -1;
        }
    }
};