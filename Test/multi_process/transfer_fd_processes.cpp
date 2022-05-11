#include <sys/socket.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>

static const int CONTROL_LEN=CMSG_LEN(sizeof(int));

//fd:for socket,fd_to_send:wait for sending
void send_fd(int fd,int fd_to_send){
    struct iovec iov[1];
    struct msghdr msg;
    cmsghdr cm;
    char buf[0];

    iov[0].iov_base=buf;
    iov[0].iov_len=1;
    msg.msg_name=NULL;
    msg.msg_namelen=0;
    msg.msg_iov=iov;
    msg.msg_iovlen=1;
    cm.cmsg_len=CONTROL_LEN;
    cm.cmsg_level=SOL_SOCKET;
    cm.cmsg_type=SCM_RIGHTS;
    *(int*)CMSG_DATA(&cm)=fd_to_send;
    msg.msg_control=&cm;
    msg.msg_controllen=CONTROL_LEN;

    sendmsg(fd,&msg,0);
}

int recv_fd(int fd){
    struct iovec iov[1];
    struct msghdr msg;
    char buf[0];

    iov[0].iov_base=buf;
    iov[0].iov_len=1;
    msg.msg_name=NULL;
    msg.msg_namelen=0;
    msg.msg_iov=iov;
    msg.msg_iovlen=1;

    cmsghdr cm;
    msg.msg_control=&cm;
    msg.msg_controllen=CONTROL_LEN;

    recvmsg(fd,&msg,0);

    int fd_to_read=*(int*)CMSG_DATA(&cm);
    return fd_to_read;
}

int main(){
    int pipefd[2];
    int ret=socketpair(PF_UNIX,SOCK_DGRAM,0,pipefd);//pipe for parent&child
    assert(ret!=-1);

    int fd_to_pass=0;

    pid_t pid=fork();
    assert(pid>=0);

    if(pid==0){
        fd_to_pass=open("test.txt",O_RDWR,0666);
        //child send fd to parent through pipe
        send_fd(pipefd[1],fd_to_pass);
    }
    //parent receive fd from pipe
    fd_to_pass = recv_fd(pipefd[0]);
    printf("parent get fd from child:%d\n",fd_to_pass);
}