#include <thread>
class thread_guard{
    std::thread& t;
public:
    explicit thread_guard(std::thread& t_):t(t_){}
    ~thread_guard(){
        if(t.joinable()){
            t.join();
        }
    }
};

struct func{
    int& i;
    func(int& i_):i(i_){}
    void operator()(){
        for(auto j=0;j<100000;j++){
            i++;
        }
    }
};

void f(){
    int some_local_state=0;
    func my_func(some_local_state);
    std::thread t(my_func);
    thread_guard g(t);
}