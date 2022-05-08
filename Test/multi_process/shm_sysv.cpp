/****for linux/ipc/shm.c*****/

#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

void write_node(){
    key_t key=ftok("/Documents/shm/demo",0);
    int shm_id=shmget(key,0x400000,IPC_CREAT|0666);
    char* paddress=(char*)shmat(shm_id,NULL,0);

    memset(paddress,'A',0x400000);
    shmdt(paddress);
}

void read_node(){
    key_t key=ftok("/Documents/shm/demo",0);
    int shm_id=shmget(key,0x400000,0666);
    char* paddress=(char*)shmat(shm_id,NULL,0);
    printf("%c %c %c %c\n",paddress[0],paddress[1],paddress[2],paddress[3]);
    shmdt(paddress);
}

int main(){
    pid_t pid=fork();
    if(pid<0){
        printf("fork error.\n");
    }
    else if(pid==0){
        //child
        printf("child handle,start to write.\n");
        write_node();
    }
    else{
        //parent
        sleep(5);
        printf("parent handle,start to read.\n");
        read_node();
    }
    waitpid(pid,NULL,0);
    return 0;
}