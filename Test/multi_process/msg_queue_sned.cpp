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

void send_msg(){
    int msgid=-1;
    struct msg_st data;
    char buff[BUFFER_SIZE];
    //create msg queue
    msgid=msgget((key_t)1234,0666|IPC_CREAT);
    if(msgid==-1){
        printf("msgget error:%d\n",errno);
        exit(EXIT_FAILURE);
    }    
    //write to the msg queue until end
    while(1){
        printf("enter msg:\n");
        fgets(buff,BUFFER_SIZE,stdin);
        data.msg_type=1;//attention!
        strcpy(data.msg_text,buff);
        if(msgsnd(msgid,(void*)&data,BUFFER_SIZE,0)==-1){
            printf("msg send error:%d\n",errno);
            exit(EXIT_FAILURE);
        }
        if(strncmp(buff,"end",3)==0){
            break;
        }
        sleep(1);
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
        send_msg();
    }
    waitpid(pid,NULL,0);
    return 0;
}