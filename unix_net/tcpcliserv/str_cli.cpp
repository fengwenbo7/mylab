#include "../unp.h"

void str_cli(FILE* fp,int sock_fd){
    char sendline[MAXLINE],recvline[MAXLINE];
    while(Fgets(sendline,MAXLINE,fp)!=NULL){//read text one line
        Writen(sock_fd,sendline,strlen(sendline));//write the text to std_out

        if(Readline(sock_fd,recvline,MAXLINE)==0)
            err_quit("");
        Fputs(recvline,stdout);
    }
}