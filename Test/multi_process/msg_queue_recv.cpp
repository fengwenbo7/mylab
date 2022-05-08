#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/msg.h>
#include <errno.h>
#include <sys/wait.h>

#define BUFFER_SIZE 512

struct msg_st{
    long msg_type;
    char msg_text[BUFFER_SIZE];
};

void receive_msg(){
    int msgid=-1;
    struct msg_st data;
    long msg_type=0;//attention!
    //create msg queue
    msgid=msgget((key_t)1234,0666|IPC_CREAT);
    if(msgid==-1){
        printf("msgget error:%d\n",errno);
        exit(EXIT_FAILURE);
    }
    //pull msg from queue
    while(1){
        if(msgrcv(msgid,(void*)&data,BUFFER_SIZE,msg_type,0)==-1){
            printf("recv error:%d\n",errno);
        }
        printf("msg:%s",data.msg_text);
        //break when end
        if(strncmp(data.msg_text,"end",3)==0){
            break;
        }
    }
    //delete msg queue
    if(msgctl(msgid,IPC_RMID,0)==-1){
        printf("IPC_RMID error:%d\n",errno);
    }
    exit(EXIT_SUCCESS);
}

int main(){
    int pid=fork();
    if(pid<0){
        perror("fork error.");
        return -1;
    }
    else if(pid==0){
        receive_msg();
    }
    waitpid(pid,NULL,0);
    return 0;
}