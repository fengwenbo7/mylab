#ifndef TIMER_WHEEL_TIMER
#define TIMER_WHEEL_TIMER

#include <time.h>
#include <netinet/in.h>
#include <stdio.h>

#define BUFFER_SIZE 64
class tw_timer;
//bind timer and socket
struct client_data
{
    sockaddr_in address;
    int sockfd;
    char buf[BUFFER_SIZE];
    tw_timer* timer;
};

//class for timer
class tw_timer{
public:
    tw_timer(int rot,int ts):next(NULL),prev(NULL),rotation(rot),timer_slot(ts){}

public:
    int rotation;//work after rotation number
    int timer_slot;//bleong to which slot
    void(*cb_func)(client_data*);//callback function
    client_data* user_data;//client data
    tw_timer* next;
    tw_timer* prev;
};

//wheel for timers
class timer_wheel{
public:
    timer_wheel():cur_slot(0){
        //init the head node of each slot
        for(int i=0;i<number_in_wheel;i++){
            slots[i]=NULL;
        }
    }
    ~timer_wheel(){
        //destory the timers in each slot
        for(int i=0;i<number_in_wheel;i++){
            tw_timer* head=slots[i];
            while(head){
                slots[i]=head->next;
                delete head;
                head=slots[i];
            }
        }
    }

    tw_timer* add_timer(int timeout){
        if(timeout<0) return NULL;
        //1.calc the number of ticks
        int num_ticks=0;
        if(timeout<si){
            num_ticks=1;
        }
        else{
            num_ticks=timeout/si;
        } 
        //2.calc the number of laps
        int num_lap=timeout/number_in_wheel;
        //3.calc which slot should be the target
        int tar_slot=(cur_slot+(num_ticks%number_in_wheel))%number_in_wheel;
        //4.create a timer
        tw_timer* timer =new tw_timer(num_lap,tar_slot);
        //5.insert into the wheel of timer
        //5.1 null,so it should be the head
        if(slots[tar_slot]==NULL){
            slots[tar_slot]=timer;
        }
        //5.2 not head,so insert in head
        else{
            timer->next=slots[tar_slot];
            slots[tar_slot]->prev=timer;
            slots[tar_slot]=timer;
        }
        return timer;
    }
    void del_timer(tw_timer* timer){
        if(!timer) return;
        int ts=timer->timer_slot;
        //if the timer is the head
        if(slots[ts]==timer){
            tw_timer* next=slots[ts]->next;
            if(next){
                next->prev=NULL;
            }
            slots[ts]=next;
        }
        else{
            tw_timer* next=slots[ts]->next;
            tw_timer* prev=slots[ts]->prev;
            if(next){
                next->prev=prev;
            }
            prev->next=next;
        }
        delete timer;
    }
    void tick(){
        tw_timer* tmp=slots[cur_slot];
        while(tmp){
            if(tmp->rotation>0){
                tmp->rotation--;
                tmp=tmp->next;
            }
            else{
                tmp->cb_func(tmp->user_data);
                if(tmp=slots[cur_slot]){
                    //head
                    slots[cur_slot]=slots[cur_slot]->next;
                    if(slots[cur_slot]){
                        slots[cur_slot]->prev=NULL;
                    }
                    delete tmp;
                    tmp=slots[cur_slot];
                }
                else{
                    tw_timer* prev=tmp->prev;
                    tw_timer* next=tmp->next;
                    prev->next=next;
                    if(next){
                        next->prev=prev;
                    }
                    delete tmp;
                    tmp=next;
                }
            }
        }
        cur_slot=(cur_slot+1)%number_in_wheel;
    }
private:
    static const int number_in_wheel=60;//slot cout in the timer wheel
    static const int si=1;//span in slot which means ratate each second
    tw_timer* slots[number_in_wheel];//timer link list
    int cur_slot;//current slot
};

#endif