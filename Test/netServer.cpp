#include "NetApi/NetSDK/NetServer.h"

int main(){
    NetServer* server=new NetServer();
    server->CreateSocketPeer();
}