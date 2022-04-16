#include <iostream>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <poll.h>
#include <unistd.h>
#include <memory.h>

using namespace std;

int main(){
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<=0){
        perror("socket error");
        return -1;
    }
    struct sockaddr_in listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(9999);
    listen_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret<0){
        perror("bind error");
        return -1;
    }
    ret=listen(listen_fd,5);
    if(ret<0){
        perror("listen error");
        return -1;
    }
    pollfd fds[1];
    struct sockaddr_in client_addr;
    socklen_t len=sizeof(client_addr);
    int conn_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&len);
    if(conn_fd<=0){
        perror("accept error.\n");
        return -1;
    }
    fds[0].fd=conn_fd;
    fds[0].events=POLLIN;
    while(1){
        int pret=poll(fds,1,5000);
        if (pret == 0){
			/* 超时返回 */
			printf("time out\n");
		}else if(pret<0){
			/* 出错返回 */
		    printf("poll error\n"); 
		}else{     
			/* 有数据可读,读取数据 */
			/* 这里为了简单就不对返回的事件revents，做判断和重置了 */
            char buf[1024];
            memset(buf,'\0',sizeof(buf)-1);
			read(fds[0].fd, buf, 1);
			printf("client data = %s\n", buf);
		}
    }
}