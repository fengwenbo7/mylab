#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include "NetServer.hpp"
#define BUFFER_SIZE 4096//read buffer size
enum CHECK_STATE{
    CHECK_STATE_REQUESTLINE=0,//analysis line
    CHECK_STATE_HEADER//analysis header
};

enum LINE_STATUS{
    LINE_OK=0,//line is ok
    LINE_BAD,//line errors
    LINE_OPEN//line not complete
};

enum HTTP_CODE{
    NO_REQUEST,//request is not complete
    GET_REQUEST,//get complete customer request
    BAD_REQUEST,//grammar error from customer request
    FORBIDDEN_REQUEST,//not enough access
    INTERNAL_ERROR,//iner error is server
    CLOSED_CONNECTION//customer close the connect
};

static const char* szret[]={"I get a correct result\n","Something wrong\n"};

class HttpHandleUtil{
public:
static HttpHandleUtil* Instance(){
    static HttpHandleUtil instance;
    return &instance;
}

void MainFunc(){
    NetServer server;
    InitSocketServer(server);
}

void InitSocketServer(NetServer& server){
    server.CreateSocket();
    server.BindSocket();
    server.ListenSocket();
}

//parse line from follow-status-mechine
//checked_index point the word parsing int the buffer
//read_buffer point to the next word of customer data int the buffer
//the words from index 0 to index checked_index have been parsed
//the words from index checked_index to index read_index-1 are going to be parsed by the function blow
LINE_STATUS parse_line(char* buffer,int& checked_index,int& read_index){
    char temp;
    for(;checked_index<read_index;++checked_index){
        //current word to be parsed
        temp=buffer[checked_index];
        if(temp=='\r'){//read a complete line probably from '\r'

        }
        else if(temp=='\n'){//read a complete line probably from '\n'
            
        }
        return LINE_BAD;
    }
    return LINE_OPEN;//parse all the content but no '\n',which means we need continue to parse
}

//parse request line
HTTP_CODE parse_requestline(char* temp,CHECK_STATE& checkstate){
    char* url = strpbrk(temp," \t");
    if(!url){
        //word '\t' doesn't exist in the request line,which means error in the request line
        return BAD_REQUEST;
    }
    *url++='\0';

    char* method=temp;
    if(strcasecmp(method,"GET")==0){
        //only support get method
        printf("The request method is GET\n");
    }
    else{
        return BAD_REQUEST;
    }
}

//parse head segment
HTTP_CODE parse_headers(char* temp){

}

//parse entry point of http request
HTTP_CODE parse_content(char* buffer,int& checked_index,CHECK_STATE& checkstate,int& read_index,int& start_line){

}