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

//parse line from status mechine
LINE_STATUS parse_line(char* buffer,int& checked_index,int& read_index){

}