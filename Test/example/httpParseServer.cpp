#include "../NetApi/NetSDK/HttpHandle.hpp"
#include "../NetApi/NetSDK/NetServer.hpp"

int main(){
    
    int listen_fd=socket(PF_INET,SOCK_STREAM,0);

    struct sockaddr_in address;
    bzero(&address,sizeof(address));
    address.sin_family=AF_INET;
    inet_pton(AF_INET,"127.0.0.1",&address.sin_addr);
    int ret = bind(listen_fd,(struct sockaddr*)&address.sin_addr,sizeof(address));
    assert(ret!=-1);

    ret = listen(listen_fd,0);
    assert(ret!=-1);

    struct sockaddr_in client_address;
    socklen_t sock_len=sizeof(client_address);
    int conn_fd=accept(listen_fd,(struct sockaddr*)&client_address,&sock_len);

    if(conn_fd<0){
        printf("accept error:%d\n",errno);
        close(listen_fd);
    }
    else{

    }
}