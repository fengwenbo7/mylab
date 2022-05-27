#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

#define CUSTOM_COUNT 2
#define PRODUCT_COUNT 3

sem_t blanks;
sem_t datas;
pthread_mutex_t mutex_=PTHREAD_MUTEX_INITIALIZER;

int p_c;

void* product(void* arg){
    while(1){
        sleep(1);
        sem_wait(&blanks);
        sem_post(&datas);
        pthread_mutex_lock(&mutex_);
        p_c++;
        pthread_mutex_unlock(&mutex_);
        printf("product,produce:%d\n",p_c);
    }
    pthread_exit(NULL);
}

void* consumer(void* arg){
    while(1){
        sleep(1);
        sem_wait(&datas);
        sem_post(&blanks);
        pthread_mutex_lock(&mutex_);
        p_c--;
        pthread_mutex_unlock(&mutex_);
        printf("consume,produce:%d\n",p_c);
    }
    pthread_exit(NULL);
}

int main(){
    p_c=0;
    sem_init(&blanks,0,10);
    sem_init(&datas,0,0);

    pthread_t customs[CUSTOM_COUNT];
    pthread_t products[PRODUCT_COUNT];
    for(int i=0;i<CUSTOM_COUNT;i++){
        pthread_create(&customs[i],NULL,consumer,NULL);
    }
    sleep(3);
    for(int i=0;i<PRODUCT_COUNT;i++){
        pthread_create(&products[i],NULL,product,NULL);
    }

    //wait for custom threads
    for(int i=0;i<CUSTOM_COUNT;i++){
        pthread_join(customs[i],NULL);
    }
    //wait for product threads
    for(int i=0;i<PRODUCT_COUNT;i++){
        pthread_join(products[i],NULL);
    }

    printf("parent exit.\n");
    exit(0);
}