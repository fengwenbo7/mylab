#include "NetBase.hpp"
#include "../IOSDK/IOFunc.hpp"

struct NetObject{
    int socket_fd;
    struct sockaddr_in socket_addr;
};

void* ReceiveSocketMsg(void* arg){
    struct NetObject* netob=(struct NetObject*)arg;
    while(1){
        char msg[1024];
        memset(msg,'\0',1024);
        int msgret=recv(netob->socket_fd,msg,sizeof(msg)-1,0);
        if(msgret>0){
            char ip[32];
            printf("client[%s:%d]:%s\n",inet_ntoa(netob->socket_addr.sin_addr),ntohs(netob->socket_addr.sin_port),(char*)msg);
        }
        else if(msgret==0){
            printf("client[%s:%d] disconnected!\n",inet_ntoa(netob->socket_addr.sin_addr),ntohs(netob->socket_addr.sin_port));
            break;
        }
        else{
            perror("receive error!\n");
            break;
        }
    }
    return NULL;
}

class NetServer{
private:
    const char* greet="hello,i am server.";
    int listen_fd;//socket for listen
    int communication_fd;//socket for communication
    struct sockaddr_in listen_addr;//server address
    struct sockaddr_in communication_addr;//client address
    std::unordered_map<int,NetObject> clients;
private:
     int CreateSocket(){
        listen_fd=socket(AF_INET,SOCK_STREAM,0);
        printf("create server socket...\n");
        return listen_fd;
    }

    //bind socket in server
    int BindSocket(){
        bzero(&listen_addr,sizeof(listen_addr));
        listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
        listen_addr.sin_port=htons(9999);
        listen_addr.sin_family=AF_INET;
        printf("bind listen socket...\n");
        return bind(listen_fd,(const sockaddr*)(&listen_addr),sizeof(listen_addr));
    }

    //listen socket in server
    int ListenSocket(){
        printf("start listen...\n");
        return listen(listen_fd,127);
    }

    //accept connect from client in server
    int AcceptSocket(){
        int ret=0;
        while(1){
            bzero(&communication_addr,sizeof(communication_addr));
            socklen_t len=sizeof(communication_addr);
            communication_fd=accept(listen_fd,(sockaddr*)(&communication_addr),&len);
            if(communication_fd!=-1){
                struct NetObject* newClinet;
                newClinet->socket_fd=communication_fd;
                newClinet->socket_addr=communication_addr;
                clients.insert(std::make_pair(communication_fd,*newClinet));
                SendSocketMessage(communication_fd,greet,strlen(greet));
                //create sub thread
                pthread_t tid;
                pthread_create(&tid,NULL,ReceiveSocketMsg,newClinet);
                pthread_detach(tid);//子线程有可能随着连接终止而会终止，但是不能影响主线程的正常运行，因此在这里将主线程与子线程分离
            }
            else{
                ret=-1;
                break;
            }
        }
        return ret;
    }

    int SendSocketMessage(int com_fd,const char* messgae,int length){
        if(com_fd!=-1){
            send(com_fd,messgae,length,0);
        }
        else{
            return -1;
        }
        return 0;
    }

public:
    int CreateSocketPeer(){
        IOFuncUtil::Daemonize();
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
        return 0;
    }        
};