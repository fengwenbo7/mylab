#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <wait.h>

pthread_mutex_t mutex_;

void* another(void* arg){
    printf("in child thread,lock the mutex\n");
    pthread_mutex_lock(&mutex_);
    sleep(5);
    pthread_mutex_unlock(&mutex_);
}

//run before fork works
void prepare(){
    pthread_mutex_lock(&mutex_);
}

//run in parent process after sub-process created from fork but before fork return,which releases all the locks locked in the prepare
void parent(){
    pthread_mutex_unlock(&mutex_);
}

//run in child process before fork return which releases all locks locked in the prepare
void child(){
    pthread_mutex_unlock(&mutex_);
}

int main(){
    pthread_mutex_init(&mutex_,NULL);
    pthread_t ptid;
    pthread_create(&ptid,NULL,another,NULL);
    sleep(1);
    //call pthread_atfork() before fork()
    pthread_atfork(prepare,parent,child);
    int pid=fork();
    if(pid<0){
        pthread_join(ptid,NULL);
        pthread_mutex_destroy(&mutex_);
        return 1;
    }
    else if(pid==0){
        //child
        pthread_mutex_lock(&mutex_);
        printf("child handing...");
        sleep(2);
        pthread_mutex_unlock(&mutex_);
        exit(0);
    }
    else{
        //parent
        wait(NULL);
    }
    pthread_join(ptid,NULL);
    pthread_mutex_destroy(&mutex_);
    return 0;
}