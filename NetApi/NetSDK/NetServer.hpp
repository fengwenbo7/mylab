#include "NetBase.hpp"
#include "../IOSDK/IOFunc.hpp"
#include "../IOSDK/epoll_func.hpp"

struct NetObject{
    int socket_fd;
    struct sockaddr_in socket_addr;
};

enum class SocketMsgHandler{
    ReceiveHandler,
    SelectHandler,
};

void* ReceiveSocketMsg(void* arg){
    struct NetObject* netob=(struct NetObject*)arg;
    while(1){
        char msg[1024];
        memset(msg,'\0',1024);
        int msgret=recv(netob->socket_fd,msg,sizeof(msg)-1,0);
        printf("receive ret:%d\n",msgret);
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

void* SelectSocketMsg(void* arg){
    struct NetObject* netob=(struct NetObject*)arg;
    int ret = 0;
    char buf[1024];
    fd_set read_fs;
    fd_set exception_fs;
    FD_ZERO(&read_fs);
    FD_ZERO(&exception_fs);

    while(1){
        memset(buf,'\0',sizeof(buf));
        //set communication_fs in read_fs and exception_fs
        FD_SET(netob->socket_fd,&read_fs);
        FD_SET(netob->socket_fd,&exception_fs);
        ret = select(netob->socket_fd+1,&read_fs,NULL,&exception_fs,NULL);
        if(ret<0){
            perror("select error\n");
            break;
        }
        //read ready-recv
        if(FD_ISSET(netob->socket_fd,&read_fs)){
            ret=recv(netob->socket_fd,buf,sizeof(buf)-1,0);
            if(ret<=0) break;
            printf("get %d bytes of normal data:%s",ret,buf);
        }
        else if(FD_ISSET(netob->socket_fd,&exception_fs)){
            ret=recv(netob->socket_fd,buf,sizeof(buf)-1,MSG_OOB);
            if(ret<=0) break;
            printf("get %d bytes of oob data:%s",ret,buf);
        }
    }
    return NULL;
}

void* EPollSoccketMsg(void* arg){
    struct NetObject* netob=(struct NetObject*)arg;
    int epfd=epoll_create(5);
    epoll_event* events=new epoll_event[MAX_EVENT_NUMBER];
    epoll_ctl(epfd,EPOLL_CTL_ADD,netob->socket_fd,events);
    while(1){
        epoll_wait(epfd,events,MAX_EVENT_NUMBER,5000);
    }
}

class NetServer{
private:
    const char* greet="hello,i am server.";
    int listen_fd;//socket for listen
    int communication_fd;//socket for communication
    struct sockaddr_in listen_addr;//server address
    struct sockaddr_in communication_addr;//client address
    std::unordered_map<int,NetObject> clients;
    friend class HttpHandleUtil;
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
    int AcceptSocketThread(SocketMsgHandler handler){
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
                switch (handler)
                {
                case SocketMsgHandler::SelectHandler :
                    pthread_create(&tid,NULL,SelectSocketMsg,newClinet);
                    break;
                
                case SocketMsgHandler::ReceiveHandler :
                default:
                    pthread_create(&tid,NULL,ReceiveSocketMsg,newClinet);
                    break;
                }
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
        //IOFuncUtil::Daemonize();
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
        if(AcceptSocketThread(SocketMsgHandler::ReceiveHandler)==-1){
            perror("accept error");
            return -1;
        }
        return 0;
    }        
};