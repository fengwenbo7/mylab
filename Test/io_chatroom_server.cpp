#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <poll.h>

#define MAX_EVENT_NUM 1024
#define MAX_CLINET 5
#define BUFFER_SIZE 64
#define FD_LIMIT 65535

struct client_data{
    sockaddr_in address;
    char* write_buf;
    char read_buf[BUFFER_SIZE];
};

void setnonblock(int fd){
    int old_option=fcntl(fd,F_GETFL);
    int new_option=old_option|O_NONBLOCK;
    fcntl(fd,F_SETFL,new_option);
}

int main(){
    int listen_fd=socket(AF_INET,SOCK_STREAM,0);
    if(listen_fd<0){
        perror("socket error.\n");
        return -1;
    }
    struct sockaddr_in listen_addr;
    bzero(&listen_addr,sizeof(listen_addr));
    listen_addr.sin_family=AF_INET;
    listen_addr.sin_port=htons(9999);
    listen_addr.sin_addr.s_addr=INADDR_ANY;
    int ret=bind(listen_fd,(const sockaddr*)&listen_addr,sizeof(listen_addr));
    if(ret<0){
        perror("bind error.\n");
        return -1;
    }
    ret=listen(listen_fd,MAX_CLINET);
    if(ret<0){
        perror("listen error.\n");
        return -1;
    }

    client_data* users=new client_data[FD_LIMIT];//for each connection
    pollfd fds[MAX_CLINET+1];
    int user_count=0;
    fds[0].fd=listen_fd;
    fds[0].events=POLLIN|POLLERR;
    fds[0].revents=0;
    for(int i=1;i<=MAX_CLINET;i++){
        fds[i].fd=0;
        fds[i].revents=0;
    }
    while (1)
    {
        int poll_ret=poll(fds,MAX_CLINET,5000);
        if(poll_ret<0){
            perror("poll error.\n");
            break;
        }
        for(int i=0;i<MAX_CLINET;i++){
            if(fds[i].fd==listen_fd&&(fds[i].events&POLLIN)){
                struct sockaddr_in client_addr;
                socklen_t len=sizeof(client_addr);
                bzero(&client_addr,len);
                int conn_fd=accept(listen_fd,(struct sockaddr*)&client_addr,&len);
                if(conn_fd<0){
                    perror("accept error.\n");
                    continue;
                }
                if(user_count>=MAX_CLINET){
                    const char* msg="reject connect because of out range of user numbers.";
                    printf("%s",msg);
                    send(conn_fd,msg,sizeof(msg),0);
                    close(conn_fd);
                    continue;
                }
                //new client
                user_count++;
                fds[user_count].fd=conn_fd;
                fds[user_count].events=POLLIN|POLLHUP|POLLERR;
                fds[user_count].revents=0;
                users[conn_fd].address=client_addr;
                setnonblock(conn_fd);
                printf("new client connects.now users num:%d\n",user_count);
            }
            else if(fds[i].events&POLLERR){
                printf("fd:%d error happened.\n",fds[i].fd);
                continue;
            }
            else if(fds[i].events&POLLHUP){
                printf("fd:%d disconnected.\n",fds[i].fd);
                close(fds[i].fd);
                fds[i]=fds[user_count];
                i--;
                user_count--;
            }
            else if(fds[i].events&POLLIN){
                ret=recv(fds[i].fd,users[fds[i].fd].read_buf,BUFFER_SIZE-1,0);
                printf("get %d length message:%s from fd:%d\n",ret,users[fds[i].fd].read_buf,fds[i].fd);
                if(ret<0){
                    if(errno!=EAGAIN){
                        printf("fd:%d error.so we disconnect it.\n",fds[i].fd);
                        close(fds[i].fd);
                        fds[i]=fds[user_count];
                        i--;
                        user_count--;
                    }
                }
                else if(ret>0){
                    //notify other clients to write if receive message
                    for(int j=0;j<user_count;j++){
                        if(fds[j].fd==fds[i].fd){
                            continue;
                        }
                        fds[j].events|=~POLLIN;
                        fds[j].events|=POLLOUT;
                        users[fds[j].fd].write_buf=users[fds[i].fd].read_buf;
                    }
                }
            }
            else if(fds[i].events&POLLOUT){
                  if(!users[fds[i].fd].write_buf){
                      continue;
                  }
                  ret=send(fds[i].fd,users[fds[i].fd].write_buf,strlen(users[fds[i].fd].write_buf),0);
                  users[fds[i].fd].write_buf=NULL;
                  //update read ready
                  fds[i].events|=~POLLOUT;
                  fds[i].events|=POLLIN;
            }
        }
    }
    delete[] users;
    close(listen_fd);
    return 0;
}