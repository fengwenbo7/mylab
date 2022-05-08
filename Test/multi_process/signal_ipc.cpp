#include <sys/sem.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

union semun
{
    int val;//for SETVAL
    struct semid_ds* buf;//for IPC_STAT and IPC_SET
    unsigned short* array;//for GETALL and SETALL
    struct seminfo* _buf;//for IPC_INFO
};

void pv(int sem_id,int op){
    struct sembuf sem_b;
    sem_b.sem_num=0;//0-first signal
    sem_b.sem_op=op;//+,0,-
    sem_b.sem_flg=SEM_UNDO;//IPC_NOWAIT-return immediately,SEM_UNDO-cancel op when exit
    semop(sem_id,&sem_b,1);
}

int main(){
    int sem_id=semget(IPC_PRIVATE,1,0666);

    union semun sem_un;
    sem_un.val=1;
    semctl(sem_id,0,SETVAL,sem_un);

    pid_t pid=fork();
    if(pid<0){
        return -1;
    }
    else if(pid==0){
        printf("child work.try to get binary sem.\n");
        pv(sem_id,-1);
        printf("child get the sem.\n");
        printf("child handle\n");
        sleep(5);
        pv(sem_id,1);
        exit(0);

    }
    else{
        printf("parent work.try to get binary sem.\n");
        pv(sem_id,-1);
        printf("parent get the sem.\n");
        printf("parent handle\n");
        pv(sem_id,1);
    }
    waitpid(pid,NULL,0);
    semctl(sem_id,0,IPC_RMID,sem_un);
    return 0;
}