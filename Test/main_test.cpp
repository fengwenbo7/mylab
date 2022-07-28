#include <arpa/inet.h>
#include <iostream>

using namespace std;

char* sock_ntop(const struct sockaddr* sa,socklen_t salen){
    static char str[128];
    switch (sa->sa_family)
    {
    case AF_INET:{
        struct sockaddr_in *sin=(struct sockaddr_in *) sa;
        if(inet_ntop(AF_INET,&sin->sin_addr,str,sizeof(str))==NULL){
            return(NULL);
        }
        return str;
    }
        break;
    
    default:
        break;
    }
}

int main(){
    const char* addr="192.168.0.1";
    char buf[INET_ADDRSTRLEN];

    struct sockaddr_in addr_;
    inet_aton(addr,&addr_.sin_addr);
    printf("address:%s\n",inet_ntoa(addr_.sin_addr));

    inet_pton(AF_INET,"192.168.0.2",&addr_.sin_addr);
    inet_ntop(AF_INET,&addr_.sin_addr,buf,INET_ADDRSTRLEN);
    printf("address:%s\n",buf);
}