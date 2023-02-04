#include <iostream>     // std::cout、std::endl
#include <sys/socket.h> // socket、bind、listen、accept
#include <netinet/in.h> // htonl、htons
#include <unistd.h>     // write、close
#include <string.h>     // bzero
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <strings.h>
#include <sys/epoll.h>
#include <memory.h>
#include <fcntl.h>
#include <vector>

#include <iostream>
#include "ContanceManager.h"
using namespace std;

#define PORT 8099
#define LISTENQ 5
#define MAX_EVENT_NUM 1024
#define BUFFER_SIZE 1024

struct fds
{
    int epoll_fd;
    int sock_fd;
};

void add_fd_into_epoll_fds(int epollfd, int tarfd, bool oneshot)
{
    // add fd to epoll
    epoll_event event;
    event.data.fd = tarfd;
    event.events = EPOLLIN | EPOLLET;
    if (oneshot)
    {
        event.events |= EPOLLONESHOT;
    }
    epoll_ctl(epollfd, EPOLL_CTL_ADD, tarfd, &event);
    // set non-block
    int old_option = fcntl(tarfd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(tarfd, F_SETFL, new_option);
}

// epoll events shot only once because we set the oneshot attribute,so we should set again after EAGAIN occurs
void reset_epolloneshot(int epoll_fd, int conn_fd)
{
    epoll_event event;
    event.data.fd = conn_fd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epoll_fd, EPOLL_CTL_MOD, conn_fd, &event);
}

// 使用字符分割
void Stringsplit(const string &str, const char split, vector<string> &res)
{
    if (str == "")
        return;
    string strs = str;
    size_t pos = strs.find(split);

    // 若找不到内容则字符串搜索函数返回 npos
    while (pos != strs.npos)
    {
        string temp = strs.substr(0, pos);
        res.push_back(temp);
        //去掉已分割的字符串,在剩下的字符串中进行分割
        strs = strs.substr(pos + 1, strs.size());
        pos = strs.find(split);
    }
}

void *worker(fds *fds)
{
    ContactManager *cm = ContactManager::GetInstance("ContactDat.dat", "ResumeDat.dat");
    int conn_fd = fds->sock_fd;
    int epoll_fd = fds->epoll_fd;
    pthread_t pid = pthread_self();
    char buf[BUFFER_SIZE];
    memset(buf, '\0', BUFFER_SIZE);
    while (1)
    {
        int ret = recv(conn_fd, buf, BUFFER_SIZE - 1, 0);
        // printf("receive ret:%d\n",ret);
        if (ret == 0)
        { // close connection
            printf("client closed.\n");
            epoll_ctl(epoll_fd, EPOLL_CTL_DEL, conn_fd, NULL); // remove current socket from epoll
            close(conn_fd);
            break;
        }
        else if (ret < 0)
        {
            if (errno == EAGAIN)
            { // can register again,not internet error
                // printf("read later.\n");
                reset_epolloneshot(epoll_fd, conn_fd);
                break;
            }
            else
            { // error occurs
                perror("recv error\n");
                break;
            }
        }
        else
        {
            printf("[thread:%d] get messgae:%s", (int)pid, buf);
            vector<string> query_str;
            Stringsplit(string(buf), '/', query_str);
            if (query_str.size() > 0)
            {
                if ((strcmp(query_str[0].c_str(), "new") == 0) && (query_str.size() == 7))
                {
                    PersonInfoWithResume full_info;
                    PersonInfo info;
                    memcpy(info.name, query_str[1].c_str(), sizeof(query_str[1].c_str()));
                    memcpy(info.phonenumber, query_str[4].c_str(), sizeof(query_str[4].c_str()));
                    memcpy(info.email, query_str[5].c_str(), sizeof(query_str[5].c_str()));
                    memcpy(full_info.resume, query_str[6].c_str(), sizeof(query_str[6].c_str()));
                    info.age = atoi(query_str[2].c_str());
                    info.gender = atoi(query_str[3].c_str());
                    full_info.person_info = info;
                    info.resume_length = strlen(full_info.resume);
                    std::cout << "save to dat,resume size:" << info.resume_length << std::endl;
                    cm->SaveData(full_info);
                    const char *msg_to_send = "new ok.";
                    send(conn_fd, msg_to_send, strlen(msg_to_send), 0);
                }
                else if ((strcmp(query_str[0].c_str(), "search") == 0) && (query_str.size() == 4))
                {
                    string in_name = query_str[1];
                    string in_phonenum = query_str[2];
                    string in_email = query_str[3];
                    std::cout << "search name:" << in_name << ",phonenumber:" << in_phonenum << ",email:" << in_email << ",data_size:" << cm->GetDataSize() << std::endl;
                    list<PersonInfoWithResume> results = cm->SearchFromDat(in_name.c_str(), in_phonenum.c_str(), in_email.c_str(), true);
                    for (auto iter = results.begin(); iter != results.end(); iter++)
                    {
                        std::cout << "name:" << iter->person_info.name << " "
                                  << "age:" << iter->person_info.age << " "
                                  << "phonenumber:" << iter->person_info.phonenumber << " "
                                  << "email:" << iter->person_info.email << " "
                                  << "resume:" << iter->resume << std::endl;

                        const char *msg_to_send = "search ok.";
                        send(conn_fd, msg_to_send, strlen(msg_to_send), 0);
                    }
                }
            }
        }
    }
    // printf("end thread receiving data on fd:%d",conn_fd);
    return NULL;
}

int main()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd < 0)
    {
        perror("socket error\n");
        return -1;
    }

    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    listen_addr.sin_port = htons(PORT);
    int ret = bind(listen_fd, (const sockaddr *)&listen_addr, sizeof(listen_addr));
    if (ret == -1)
    {
        perror("bind error\n");
        return -1;
    }

    ret = listen(listen_fd, LISTENQ);
    if (ret == -1)
    {
        perror("listen error\n");
        return -1;
    }

    epoll_event events[MAX_EVENT_NUM];
    int epoll_fd = epoll_create(LISTENQ);
    if (epoll_fd < 0)
    {
        perror("epoll create error\n");
        return -1;
    }

    add_fd_into_epoll_fds(epoll_fd, listen_fd, false);

    while (1)
    {
        ret = epoll_wait(epoll_fd, events, MAX_EVENT_NUM, -1);
        if (ret < 0)
        {
            perror("epoll wait error\n");
            break;
        }
        for (int i = 0; i < ret; i++)
        {
            int sock_fd = events[i].data.fd;
            if (sock_fd == listen_fd)
            {
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int conn_fd = accept(listen_fd, (sockaddr *)&client_addr, &len);
                add_fd_into_epoll_fds(epoll_fd, conn_fd, true);
                printf("client connect\n");
                const char *msg_to_send = "input fucntion.1.new/<name>/<age>/<gender>/<phonenumber>/<email>/<resume>/ 2.search/<name>/<phonenumber>/<email>/";
                send(conn_fd, msg_to_send, strlen(msg_to_send), 0);
                std::cout << "send to client:" << msg_to_send << std::endl;
            }
            else if (events[i].events & EPOLLIN)
            {
                printf("epoll in occurs\n");
                fds fd_new_process;
                fd_new_process.epoll_fd = epoll_fd;
                fd_new_process.sock_fd = sock_fd;
                int pid = fork();
                if (pid == 0)
                {
                    // child process
                    close(listen_fd);
                    worker(&fd_new_process);
                }
            }
            else
            {
                printf("something else happened\n");
            }
        }
    }
    close(listen_fd);
    return 0;
}
