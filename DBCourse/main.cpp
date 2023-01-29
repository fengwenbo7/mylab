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

#include <iostream>
#include "ContanceManager.h"
using namespace std;

#define PORT 8099
#define LISTENQ 5
#define MAXLINE 4096
#define MAX_EVENT_NUM 1024

void add_fd_into_epoll_fds(int epollfd, int tarfd)
{
    // add fd to epoll
    epoll_event event;
    event.data.fd = tarfd;
    event.events = EPOLLIN | EPOLLET | EPOLLONESHOT;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, tarfd, &event);
    // set non-block
    int old_option = fcntl(tarfd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(tarfd, F_SETFL, new_option);
}

int _main()
{
    int listen_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (listen_fd == -1)
    {
        perror("socket error\n");
        return -1;
    }
    struct sockaddr_in listen_addr;
    listen_addr.sin_family = AF_INET;
    listen_addr.sin_port = htons(PORT);
    listen_addr.sin_addr.s_addr = INADDR_ANY;
    int ret = bind(listen_fd, (const struct sockaddr *)&listen_addr, sizeof(listen_addr));
    if (ret == -1)
    {
        perror("bind error\n");
        return -1;
    }
    ret = listen(listen_fd, 5);
    if (ret == -1)
    {
        perror("listen error\n");
        return -1;
    }

    epoll_event events[MAX_EVENT_NUM];
    int epoll_fd = epoll_create(5);
    if (epoll_fd < 0)
    {
        perror("epoll create error\n");
        return -1;
    }

    add_fd_into_epoll_fds(epoll_fd, listen_fd);

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
                // events of listen_fd invoke,which means client connect,so we should add client_fd(conn_fd) into the epoll-lists
                struct sockaddr_in client_addr;
                socklen_t len = sizeof(client_addr);
                int conn_fd = accept(sock_fd, (sockaddr *)&client_addr, &len);
                add_fd_into_epoll_fds(epoll_fd, conn_fd);
            }
            else if (events[i].events & EPOLLIN)
            {
                // message read ready
                char buf[1024];
                memset(buf, '\0', sizeof(buf));
                ret = recv(sock_fd, buf, sizeof(buf) - 1, 0);
                if (ret < 0)
                {
                    close(sock_fd);
                    break;
                }
                else if (ret == 0)
                {
                    break;
                }
                else
                {
                    printf("client data:%s\n", buf);
                }
            }
            else
            {
                printf("something else happened.\n");
            }
        }
    }
    close(listen_fd);
    return 0;
}

int main()
{
    ContactManager cm("ContactDat.dat");
    while (true)
    {
        std::cout << "input function:" << std::endl;
        std::cout << "1.new" << std::endl;
        std::cout << "2.search" << std::endl;
        int func_type;
        std::cin >> func_type;
        switch (func_type)
        {
        case 1:
        {
            PersonInfoWithResume full_info;
            PersonInfo info;
            std::cout << "name:";
            std::cin >> info.name;
            std::cout << "age:";
            std::cin >> info.age;
            std::cout << "gender:";
            std::cin >> info.gender;
            std::cout << "phonenumber:";
            std::cin >> info.phonenumber;
            std::cout << "email:";
            std::cin >> info.email;
            std::cout << "resume:";
            char resume_tmp[128];
            std::cin >> resume_tmp;
            full_info.resume = resume_tmp;
            std::cout << "save to dat" << std::endl;
            full_info.person_info = info;
            full_info.resume_length = sizeof(full_info.resume);
            std::cout << "save to dat" << std::endl;
            cm.SaveData(full_info);
        }
        break;
        case 2:
        {
            std::cout << "search name:";
            string in_name;
            std::cin >> in_name;

            std::cout << "search phone number:";
            string in_phonenum;
            std::cin >> in_phonenum;

            std::cout << "search email:";
            string in_email;
            std::cin >> in_email;

            list<PersonInfoWithResume> results = cm.SearchFromDat(in_name.c_str(), in_phonenum.c_str(), in_email.c_str());
            for (auto iter = results.begin(); iter != results.end(); iter++)
            {
                std::cout << "name:" << iter->person_info.name << " "
                          << "age:" << iter->person_info.age << " "
                          << "phonenumber" << iter->person_info.phonenumber << " "
                          << "email" << iter->person_info.email << std::endl;
            }
        }
        break;
        default:
            break;
        }
    }
}