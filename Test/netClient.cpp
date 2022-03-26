#include "NetApi/NetSDK/NetClient.h"

int main(){
    NetClient* client=new NetClient();
    client->CreateSocketPeer("127.0.0.1");
}