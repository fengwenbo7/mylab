#ifndef MIN_HEAP
#define MIN_HEAP

#include <iostream>
#include <netinet/in.h>
#include <time.h>
using std::exception;

#define BUFFER_SIZE 64

class heap_timer;
class client_data{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    heap_timer* timer;
};

class heap_timer{
public:
    heap_timer(int delay){
        expire=time(NULL)+delay;
    }

public:
    time_t expire;
    void(*cb_func)(client_data* data);
    client_data* user_data;
};

class timer_heap{
public:
    //construct in one case that creates a stack with cap capacity
    timer_heap(int cap):capacity(cap),cur_size(0){
        timer_array=new heap_timer*[capacity];
        if(!timer_array){
            throw std::exception();
        }
        for(int i=0;i<capacity;i++){
            timer_array[i]=NULL;
        }
    }

    //construct from existed array
    timer_heap(heap_timer** init_array,int size,int capacity) throw (std::exception):cur_size(size),capacity(capacity){
        if(capacity<size) throw std::exception();
        timer_array=new heap_timer*[capacity];
        if(!timer_array) throw std::exception();
        for(int i=0;i<capacity;i++){
            timer_array[i]=NULL;
        }
        if(size!=0){
            for(int i=0;i<size;i++){
                timer_array[i]=init_array[i];
            }
            for(int i=0;i<(size-1)/2;i++){
                percolate_down(i);
            }
        }
    }

    ~timer_heap(){
        for(int i=0;i<cur_size;i++){
            delete timer_array[i];
        }
        delete[] timer_array;
    }

public:
    void add_timer(heap_timer* timer){
        if(!timer) return;
        if(cur_size>=capacity){
            resize();
        }
        percolate_up(cur_size++,timer);
    }

    void del_timer(heap_timer* timer){
        if(!timer) return;
        //destory lazy
        timer->cb_func=NULL;
    }

    heap_timer* top(){
        if(cur_size==0) return NULL;
        return timer_array[0];
    }

    void pop_timer(){
        if(cur_size==0) return;
        if(timer_array[0]){
            delete timer_array[0];
            timer_array[0]=timer_array[--cur_size];
            percolate_down(0);
        }
    }

    void tick(){
        //callback and delete top
        heap_timer* timer=timer_array[0];
        if(!timer) return;
        time_t cur=time(NULL);
        while(cur_size!=0){
            if(!timer||cur<timer->expire){
                break;
            }
            if(timer->cb_func){
                timer->cb_func(timer->user_data);
            }
            pop_timer();
            timer=timer_array[0];
        }
    }

private:
    void resize() throw(std::exception){
        heap_timer** temp=new heap_timer*[2*capacity];
        for(int i=0;i<2*capacity;i++){
            temp[i]=NULL;
        }
        if(!temp) throw std::exception();
        for(int i=0;i<cur_size;i++){
            temp[i]=timer_array[i];
        }
        delete[] timer_array;
        timer_array=temp;
    }

    void percolate_down(int hole){
        heap_timer* timer=timer_array[hole];
        int child=0;
        for(;(hole*2+1)<cur_size;hole=child){
            child=hole*2+1;
            //find the smaller child
            if(child<cur_size-1&&(timer_array[child+1]->expire<timer_array[child]->expire)){
                ++child;
            }
            if(timer->expire>timer_array[child]->expire){
                timer_array[hole]=timer_array[child];
            }
            else{
                break;
            }
        }
        timer_array[hole]=timer;
    }

    void percolate_up(int hole,heap_timer* timer){
        int parent=0;
        for(;hole>0;hole=parent){
            parent=hole/2;
            if(timer_array[parent]->expire<timer->expire){
                break;
            }
            timer_array[hole]=timer_array[parent];
        }
        timer_array[hole]=timer;
    }

private:
    heap_timer** timer_array;//container for heap_timer
    int capacity;
    int cur_size;
};

#endif