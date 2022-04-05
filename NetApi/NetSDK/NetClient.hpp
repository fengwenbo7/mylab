#include "NetBase.hpp"
#include "../IOSDK/IOFunc.hpp"
#include "iostream"

class NetClient{
private:
    const char* respon="roger.";
    int communication_fd;//socket for communication
    char communication_ip[64]={0};
    struct sockaddr_in communication_addr;//client address
    int client_uid=0;
private:
    int CreateSocket(char* _cp,int len){
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

    //connect to server in client
    int ConnectSocket(){
        bzero(&communication_addr,sizeof(communication_addr));
        communication_addr.sin_family=AF_INET;
        communication_addr.sin_port=htons(9999);
        inet_pton(AF_INET,communication_ip,&communication_addr.sin_addr.s_addr);
        printf("connect.\n");
        return connect(communication_fd,(const sockaddr*)(&communication_addr),sizeof(communication_addr));
    }

    //receive msg from cust
    int ReceiveSocketMsg(){
        //IOFuncUtil::ReflectSpliceFunc(communication_fd);
        while(1){
            char msg[1024];
            memset(msg,'\0',1024);
            int msgret=recv(communication_fd,msg,sizeof(msg)-1,0);
            if(HandleSocketMessage(msgret,msg)==-1){
                break;
            }
        }
        if(communication_fd){
            close(communication_fd);
        }
        return 0;
    }

    int SendSocketMessage(const char* messgae,int length){
        if(communication_fd!=-1){
            send(communication_fd,messgae,length,0);
        }
        else{
            return -1;
        }
        return 0;
    }

    int HandleSocketMessage(int recvret,void *msg){
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
    int CreateSocketPeer(char* _cp,int length){
        //IOFuncUtil::PutoutDataToSTDOUTandFILE("log.txt");
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