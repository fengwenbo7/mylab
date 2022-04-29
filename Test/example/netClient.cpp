#include "../NetApi/NetSDK/NetClient.hpp"
#include <string>

int main(){
    NetClient* client=new NetClient();
    char ipstr[]="127.0.0.1";//"172.16.116.139";
    client->CreateSocketPeer(ipstr,strlen(ipstr));
    return 0;
}