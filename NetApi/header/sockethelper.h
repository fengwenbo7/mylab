#ifndef _SOCKET_HELPER
#define _SOCKET_HELPER

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string>
#include <arpa/inet.h>

class SocketHelper{
public:
    int CreateSocket();
    int BindSocket();
    int ListenSocket();
    int AcceptSocket();
    int ConnectSocket(const char* _cp);
    int HandleSocketMessage();
private:
    int sfd;//server socket instance
    int cfd;//client socket instance
    struct sockaddr_in saddr;//server address
    struct sockaddr_in caddr;//client address
};

#endif