#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include <sys/uio.h>
#include <sys/sendfile.h>

#define BUFFER_SIZE 1024

static const char* status_line[2]={"200 OK","500 Internal server error"};

class IOFuncUtil{
public:
    static void WriteConcentratedFunc(int conn_fd,const char* file_name){
        //save state,header,cache for http response
        char header_buf[BUFFER_SIZE];
        memset(header_buf,'\0',sizeof(header_buf));
        //save target file
        char* file_buf;
        struct stat file_stat;//properties for target file,such as size and so on
        bool valid=true;
        int len=0;//used size in header_buf
        if(stat(file_name,&file_stat)<0){
            valid=false;
        }
        else{
            if(S_ISDIR(file_stat.st_mode)){
                valid=false;
            }
            else if(file_stat.st_mode&S_IROTH){//current user has access to read
                int fd=open(file_name,O_RDONLY);
                //donymic assign cahce file_buf by st_size
                file_buf=new char[file_stat.st_size+1];
                memset(file_buf,'\0',file_stat.st_size+1);
                if(read(fd,file_buf,file_stat.st_size)<0){
                    valid=false;
                }
            }
            else{
                valid=false;
            }
        }
        if(valid){
            //organize http response,header,empty line
            int ret=snprintf(header_buf,BUFFER_SIZE-1,"%s %s\r\n","HTTP/1.1",status_line[0]);
            len+=ret;
            ret=snprintf(header_buf+len,BUFFER_SIZE-1-len,"Content-Length:%d\r\n",(int)file_stat.st_size);
            len+=ret;
            ret=snprintf(header_buf+len,BUFFER_SIZE-len-1,"%s","\r\n");
            //writev
            struct iovec iv[2];
            iv[0].iov_base=header_buf;
            iv[0].iov_len=strlen(header_buf);
            iv[1].iov_base=file_buf;
            iv[1].iov_len=file_stat.st_size;
            writev(conn_fd,iv,2);
        }
        else{
            int ret=snprintf(header_buf,BUFFER_SIZE-1,"%s %s\r\n","HTTP/1.1",status_line[1]);
            len+=ret;
            ret=snprintf(header_buf+len,BUFFER_SIZE-len-1,"%s","\r\n");
            send(conn_fd,header_buf,strlen(header_buf),0);
        }
        delete[] file_buf;
    }

    //zero copy by sendfile call
    static void SendFileFunc(int conn_fd,const char* file_name){
        int file_fd=open(file_name,O_RDONLY);
        assert(file_fd>0);
        struct stat stat_buf;
        fstat(file_fd,&stat_buf);
        sendfile(conn_fd,file_fd,NULL,stat_buf.st_size);
    }

    //zero copy by splice call
    static void ReflectSpliceFunc(int conn_fd){
        int pipefd[2];
        int ret=pipe(pipefd);
        //redirect the socket to pipe outsream
        ret=splice(conn_fd,NULL,pipefd[1],NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ret!=-1);
        //redirect the pipe outstream to socket
        ret=splice(pipefd[0],NULL,conn_fd,NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ret!=-1);
    }

    //redirect stand input to file and stand output
    static void PutoutDataToSTDOUTandFILE(const char* file_name){
        int file_fd=open(file_name,O_CREAT|O_WRONLY|O_TRUNC,0666);
        assert(file_fd>0);
        printf("open file\r\n");

        int pipefd_stdout[2];
        int ret=pipe(pipefd_stdout);//create pipe for tee
        assert(ret!=-1);
        printf("pipe_stdout\r\n");

        int pipefd_file[2];
        ret=pipe(pipefd_file);//create pipe for tee
        assert(ret!=-1);
        printf("pipe_file\r\n");

        //stand input->pipe_stdout->pipe_file->file
        //                        ->stand output
        
        //redirect stand input to pipefd_stdout
        ret=splice(STDIN_FILENO,NULL,pipefd_stdout[1],NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        printf("splice ret:%d\r\n",ret);
        assert(ret!=-1);
        printf("splice stdout->pipe_stdout\r\n");

        //redirect pipefd_stdout to pipefd_file
        ret=tee(pipefd_stdout[0],pipefd_file[1],32768,SPLICE_F_NONBLOCK);
        assert(ret!=-1);
        printf("tee pipe_stdout->pipe_file\r\n");

        //redirect pipefd_file to filefd
        ret=splice(pipefd_file[0],NULL,file_fd,NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ret!=-1);
        printf("splice pipe_file->file\r\n");

        //redirect pipefd_stdout to stand output
        ret=splice(pipefd_stdout[0],NULL,STDOUT_FILENO,NULL,32768,SPLICE_F_MORE|SPLICE_F_MOVE);
        assert(ret!=-1);
        printf("splice pipe_stdout->stdout\r\n");

        return;
        close(file_fd);
        close(pipefd_stdout[0]);
        close(pipefd_stdout[1]);
        close(pipefd_file[0]);
        close(pipefd_file[1]);
    }

    //server runs background
    static bool Daemonize(){
        //create sub process and close parent process
        pid_t pid=fork();
        if(pid<0){
            return false;
        }
        else if(pid>0){
            exit(0);//parent process exit and sub process runs
        }
        umask(0);
        pid_t sid=setsid();//craete session
        if(sid<0){
            return false;
        }
        if(chdir("/")<0){//change working directory to root directory
            return false;
        }

        //close operation
        close(STDIN_FILENO);
        close(STDOUT_FILENO);
        close(STDERR_FILENO);
        open( "/dev/null", O_RDONLY );
        open( "/dev/null", O_RDWR );
        open( "/dev/null", O_RDWR );

        return true;
    }
};