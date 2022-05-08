/****for linux/ipc/shm.c*****/
#include <sys/shm.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <sys/wait.h>

/***for poxis****/
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>

void write_sys_v(){
    key_t key=ftok("/Documents/shm/demo",0);
    int shm_id=shmget(key,0x400000,IPC_CREAT|0666);
    char* paddress=(char*)shmat(shm_id,NULL,0);

    memset(paddress,'S',0x400000);
    shmdt(paddress);
}

void write_poxis(){
    int shm_id=shm_open("/shm",O_CREAT|O_RDWR,0666);
    ftruncate(shm_id,0x400000);
    char* paddres=(char*)mmap(NULL,0x400000,PROT_READ|PROT_WRITE,MAP_SHARED,shm_id,0);
    memset(paddres,'P',0x400000);
    munmap(paddres,0x400000);
}

void read_sys_v(){
    key_t key=ftok("/Documents/shm/demo",0);
    int shm_id=shmget(key,0x400000,0666);
    char* paddress=(char*)shmat(shm_id,NULL,0);
    printf("%c %c %c %c\n",paddress[0],paddress[1],paddress[2],paddress[3]);
    shmdt(paddress);
}

void read_poxis(){
    int shm_id=shm_open("/shm",O_RDONLY,0666);
    ftruncate(shm_id,0x400000);
    char* paddress=(char*)mmap(NULL,0x400000,PROT_READ,MAP_SHARED,shm_id,0);
    printf("%c %c %c %c\n",paddress[0],paddress[1],paddress[2],paddress[3]);
    munmap(paddress,0x400000);
}

int main(){
    pid_t pid=fork();
    if(pid<0){
        printf("fork error.\n");
    }
    else if(pid==0){
        //child
        printf("child handle,start to write.\n");
        write_poxis();
    }
    else{
        //parent
        sleep(5);
        printf("parent handle,start to read.\n");
        read_poxis();
    }
    waitpid(pid,NULL,0);
    return 0;
}