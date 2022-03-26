#include "NetBase.hpp"

class NetClient:public NetBase{
protected:
    int HandleSocketMessage(int recvret,void *msg) override{
        if(recvret>0){
            printf("server:%s\n",(char*)msg);
            SendSocketMessage(respon,strlen(respon));
        }
        else if(recvret==0){
            printf("server disconnected...\n");
            return -1;
        }
        else{
            perror("receive error\n");
            return -1;
        }
        return 0;
    }
public:
    int CreateSocketPeer(char* _cp,int length) override{
        if(CreateSocket(_cp,length)==-1){
            perror("socket error");
            return -1;
        }
        if(ConnectSocket()==-1){
            perror("connect error");
            return -1;
        }
        return ReceiveSocketMsg();
    }
};