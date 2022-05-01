#ifndef LST_TIMER
#define LST_TIMER

#include <time.h>
#include <sys/socket.h>
#include <iostream>
#define BUFFER_SIZE 64

class util_timer;

struct client_data{
    sockaddr address;
    int sockfd;
    char buf[BUFFER_SIZE];
    util_timer* timer;
};

class util_timer{
public:
    util_timer():prev(NULL),next(NULL){}
public:
    time_t expire;//out-time
    void(*cb_func)(client_data*);//callback function
    client_data* user_data;//client data for callback
    util_timer* prev;
    util_timer* next;
};

class sort_timer_list{
public:
    sort_timer_list():head(NULL),tail(NULL){}

    //delete all the timers
    ~sort_timer_list(){
        util_timer* tmp=head;
        while(tmp){
            head=tmp->next;
            delete tmp;
            tmp=head;
        }
    }

    //add timer to the list
    void add_timer(util_timer* timer){
        if(!timer) return;
        if(!head){
            head=tail=timer;
            return;
        }
        if(head->expire>timer->expire){
            timer->next=head;
            head->prev=timer;
            head=timer;
            return;
        }
        add_timer(timer,head);
    }

    //adjust the position of timer in this list only when timer longer
    void adjust_timer(util_timer* timer){
        if(!timer) return;
        util_timer* tmp=timer->next;
        //tail or less than next
        if(!tmp||timer->expire<=tmp->expire) return;
        //head then pick it and insert into list
        if(timer==head){
            head=head->next;
            head->prev=NULL;
            timer->next=NULL;
            add_timer(timer,head);
        }
        //middle then pick it and insert into list
        else{
            timer->prev->next=timer->next;
            timer->next->prev=timer->prev;
            add_timer(timer,head);
        }
    }

    //delete the timer from the list
    void del_timer(util_timer* timer){
        if(!timer) return;
        //only one node
        if(timer==head&&timer==tail){
            head=tail=NULL;
            delete timer;
            return;
        }
        //delete head
        else if(timer==head){
            head=head->next;
            head->prev=NULL;
            delete timer;
            return;
        }
        //delete tail
        else if(timer==tail){
            tail=tail->prev;
            tail->next=NULL;
            delete timer;
            return;
        }
        //delete middle node
        else{
            timer->prev->next=timer->next;
            timer->next->prev=timer->prev;
            delete timer;
        }
    }

    //call tick when receive SIGALRM to handle the in-time task in the list
    void tick(){
        if(!head) return;
        std::cout<<"timer tick"<<std::endl;
        time_t cur=time(NULL);//get time now
        util_timer* tmp=head;
        //handle each node from head
        while(tmp){
            if(cur<tmp->expire){
                break;
            }
            tmp->cb_func(tmp->user_data);//process task by callback
            head=tmp->next;
            if(head){
                head->prev=NULL;
            }
            delete tmp;
            tmp=head;
        }
    }

private:
    //called by add_timer and adjust_timer which means add the timer to the list after lst_head
    void add_timer(util_timer* timer,util_timer* lst_head){
        util_timer* prev=lst_head;
        util_timer* tmp=lst_head->next;
        while (tmp)
        {
            if(tmp->expire>timer->expire){
                prev->next=timer;
                tmp->prev=timer;
                timer->next=tmp;
                timer->prev=prev;
                break;
            }
            prev=prev->next;
            tmp=tmp->next;
        }
        //tmp is null which means timer should be inserted into tail
        if(!tmp){
            tail->next=timer;
            timer->prev=tail;
            timer->next=NULL;
            tail=timer;
        }
    }

private:
    util_timer* head;
    util_timer* tail;
};

#endif