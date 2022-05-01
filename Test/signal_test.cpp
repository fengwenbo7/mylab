#include <signal.h>
#include <iostream>
#include <stdio.h>
#include <unistd.h>

using namespace std;

//第一次按下终止命令（ctrl+c）时，进程并没有被终止，面是输出get a signal 2，因为SIGINT的默认行为被signal()函数改变了.
//当进程接受到信号SIGINT时，它就去调用函数sig_handle去处理.
//注意sig_handle函数把信号SIGINT的处理方式改变成默认的方式，所以当你再按一次ctrl+c时，进程就像之前那样被终止了。
void sig_handle(int sig){
    cout<<"get a signal:"<<sig<<endl;
    // 恢复终端中断信号SIGINT的默认行为
    (void)signal(SIGINT,SIG_DFL);
}

int main(){
    // 改变终端中断信号SIGINT的默认行为，使之执行sig_handle函数,而不是终止程序的执行
    (void)signal(SIGINT,sig_handle);
    while(1){
        cout<<"hello world."<<endl;
        sleep(1);
    }
    return 0;
}