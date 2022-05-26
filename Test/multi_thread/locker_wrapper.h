#ifndef LOCKER_H
#define LOCKER_H

#include <exception>
#include <pthread.h>
#include <semaphore.h>
#include <iostream>

//semaphore wrapper
class sem{
public:
    sem(){
        if(sem_init(&sem_,0,0)!=0){
            perror("sem init error.");
        }
    }
    ~sem(){
        sem_destroy(&sem_);
    }

    bool wait_sem(){
        return sem_wait(&sem_);
    }
    bool post_sem(){
        return sem_post(&sem_)==0;
    }

private:
    sem_t sem_;
};

//metux wrapper
class locker{
public:
    locker(){
        if(pthread_mutex_init(&mutex_,NULL)!=0){
            perror("mutex init error.");
        }
    }
    ~locker(){
        pthread_mutex_destroy(&mutex_);
    }

    bool lock(){
        return pthread_mutex_lock(&mutex_)==0;
    }

    bool unlock(){
        return pthread_mutex_unlock(&mutex_)==0;
    }

private:
    pthread_mutex_t mutex_;
};

//condition
class cond{
public:
    cond(){
        if(pthread_mutex_init(&mutex_,NULL)!=0){
            perror("mutex init error.");
        }
        if(pthread_cond_init(&cond_,NULL)!=0){
            perror("cond init error.");
            pthread_mutex_destroy(&mutex_);
        }
    }
    ~cond(){
        pthread_mutex_destroy(&mutex_);
        pthread_cond_destroy(&cond_);
    }

    bool wait(){
        int ret=0;
        pthread_mutex_lock(&mutex_);
        ret=pthread_cond_wait(&cond_,&mutex_);
        pthread_mutex_unlock(&mutex_);
        return ret==0;
    }

    bool signal(){
        return pthread_cond_signal(&cond_)==0;
    }

private:
    pthread_cond_t cond_;
    pthread_mutex_t mutex_;
};

#endif