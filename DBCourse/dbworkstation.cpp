#include <sys/socket.h>
#include <arpa/inet.h>
#include <strings.h>
#include <iostream>
#include <poll.h>
#include <unistd.h>
#include <fcntl.h>
#include <memory.h>
#include <sys/epoll.h>
#include <assert.h>
#include <sys/wait.h>

#define MAX_EVENT_NUM 1024
#define BUFFER_SIZE 1024
#define PIPE_BUFFER_SIZE 32768
#define PORT 8099

void setnonblock(int fd)
{
    int old_option = fcntl(fd, F_GETFL);
    int new_option = old_option | O_NONBLOCK;
    fcntl(fd, F_SETFL, new_option);
}

void addepollfd(int epollfd, int fd)
{
    epoll_event event;
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &event);
    setnonblock(fd);
}

int main()
{
    int conn_fd = socket(PF_INET, SOCK_STREAM, 0);
    struct sockaddr_in client_address;
    bzero(&client_address, sizeof(client_address));
    client_address.sin_family = AF_INET;
    client_address.sin_port = htons(PORT);
    inet_pton(AF_INET, "127.0.0.1", &client_address.sin_addr.s_addr);
    socklen_t len = sizeof(client_address);
    int ret = connect(conn_fd, (const sockaddr *)&client_address, len);
    if (ret < 0)
    {
        perror("connect error.\n");
        close(conn_fd);
        return -1;
    }

    epoll_event events[MAX_EVENT_NUM];
    int epoll_fd = epoll_create(5);
    assert(epoll_fd > 0);
    addepollfd(epoll_fd, conn_fd);

    int std_pipefd[2];
    ret = pipe(std_pipefd);
    assert(ret != -1);
    addepollfd(epoll_fd, STDIN_FILENO);
    printf("call server success.you can type [Connect] to connect with service\n");
    char buf[BUFFER_SIZE];
    while (true)
    {
        ret = epoll_wait(epoll_fd, events, MAX_EVENT_NUM, -1);
        if (ret < 0)
        {
            perror("epoll_wait error.");
            break;
        }
        for (int i = 0; i < ret; i++)
        {
            int sock_fd = events[i].data.fd;
            if (sock_fd == conn_fd && (events[i].events & EPOLLIN))
            { // data
                memset(buf, '\0', BUFFER_SIZE);
                printf("receive data from server");
                int len = recv(sock_fd, buf, BUFFER_SIZE - 1, 0);
                printf("receive [%d]data from server:", len);
                if (len > 0)
                {
                    printf("%s", buf);
                }
                else if (len < 0)
                {
                    printf("%d", len);
                    if (errno == EAGAIN || errno == EWOULDBLOCK)
                    {
                        printf("read later.\n");
                        break;
                    }
                }
                else
                {
                    close(sock_fd);
                }
            }
            else if (sock_fd == STDIN_FILENO && (events[i].events & EPOLLIN))
            {
                splice(STDIN_FILENO, NULL, std_pipefd[1], NULL, PIPE_BUFFER_SIZE, SPLICE_F_MORE | SPLICE_F_MOVE); // get stand input
                splice(std_pipefd[0], NULL, conn_fd, NULL, PIPE_BUFFER_SIZE, SPLICE_F_MORE | SPLICE_F_MOVE);
            }
        }
    }
    return 0;
}