#include "../NetApi/NetSDK/NetServer.hpp"

int main(){
    NetServer server=NetServer();
    server.CreateSocketPeer();
    return 0;
}