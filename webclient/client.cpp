#include "client.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include<pthread.h>
#include <string>
#include <signal.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include <errno.h>
#include <stdio.h>
#include<stdlib.h>
#include "rio.h"
#include<arpa/inet.h>
#include<netinet/in.h>
#include<stdio.h>
#include <ctime>
#include<wait.h>
rio_t rio;
int play = 0;
static char fifo[]="./fifoname";
int fifofd; //有名管道描述符
int pipe_fd[2];
pthread_t readbuf;


#define MAXLINE 1000
#define MAXBUF 8192
client::client()
{

}

void client::getResponseHeader(rio_t *rio, int &len){
    char response[MAXBUF];
    char name[MAXLINE];
    while(rio_readlineb(rio, response, MAXLINE)!=-1 && strcmp(response, "\r\n")!=0){
        if(strstr(response, "Content-length:")!=NULL){
            sscanf(response, "%s %d", name, &len);
        }
        printf("%s\n", response);
    }
}

void client::showmusiclist(rio_t *rio, int len){
   printf("\r\n\033[35m-------------------------------------------\033[0m\r\n");
   char list[MAXBUF], *p,*pnext=list;
   rio_readn(sockfd, list, len);
   list[len] = 0;
   int i =1;
   int l = 0;
   while((p = index(pnext, '~'))!=NULL){
       *p = 0;
       if(l++ ==0){
            printf("\033[35m-        %d:  %s\033[0m\t", i++, pnext);
       }else if(l==4){
           printf("\033[35m%d:  %s\t-\033[0m", i++, pnext);
           l = 0;
       }else{
           printf("\033[35m%d:  %s\033[0m\t", i++, pnext);
       }
       fflush(stdout);
       pnext = p+1;
   }
   printf("\r\n\033[35m-------------------------------------------\033[0m\r\n\r\n");
}

void client::sendcmd(char *ctl,char *command, rio_t &rio){
    char cmd[MAXLINE];
    sprintf(cmd, "GET %s HTTP/1.0\r\n",command);
    rio_writen(sockfd, cmd, strlen(cmd));
    rio_writen(sockfd, "\r\n", 2);
    if(strstr(ctl, "showlist")){
        int len = 0;
        getResponseHeader(&rio, len);
        showmusiclist(&rio, len);
    }else if(strstr(ctl, "download")){
        int len = 0;
        getResponseHeader(&rio, len);
        downloadmusic(len);
    }else if(strcmp(ctl, "play")==0){
        playmusic(command);
    }else if(strcmp(ctl, "quit")==0){
        exit(0);
    }
}

void client::downloadmusic(long len){
    char buf[MAXBUF];
    char name[100];
    int fd,rn;
    time_t t;
    time(&t);
    sprintf(name, "./%d.mp3", t);
    if((fd = open(name, O_CREAT|O_RDWR, 0777))<0){
        printf("failed\n");
    }

    long int total = 0;
    long needread = MAXBUF;

    do{
        rn = rio_readnb(&rio, buf,needread);
        rio_writen(fd, buf, rn);
        total+=rn;
        long h = len-total;
        if(h<MAXBUF){
            needread = h;
        }
        printf("\n\033[1A%d%", (total / len)*100 );

    }while(total<len);

    close(fd);
    printf("\ndownlaod success\n");
}


void sig_handle(int num){
    int status;
    pid_t pid;
    while((pid = waitpid(-1, &status, WNOHANG))>0){
        if(WIFEXITED(status)){
            printf("child process revoke: %d", pid);
        }else{
            printf("process revoke .but ....");
        }
    }
    play = 0;
}

void stop(int n){
    char buf[]="GET /quit HTTP/1.0\r\n";
    rio_writen(rio.rio_fd,buf, strlen(buf));
    exit(0);
}

void stopplay(int num){
    if(play){
        write(fifofd, "stop\n",5);
        play = 0;
        pthread_cancel(readbuf);
    }else{
        signal(SIGINT, stop);
    }
}

void showinfomation (){
    printf("\t\033[36m----------------------------------------------------------------\n");
    printf("\t-退出：    mplayer stop  \t\t静音：mplayer mute 1\n");
    printf("\t-开启声音: mplayer mute 0\t\t暂停：mplayer pause\n");
    printf("\t-播放：    mplayer resume\n");
    printf("\t------------------------------------------------------------------\n\033[0m");
}

void * writectl(void *){
    showinfomation();

    char cmd[MAXLINE];
    while(1){
//        printf("hello world\n");
        if(play){
            gets(cmd);
            if(strcmp(cmd, "mplayer stop")==0){
                play = 0;
                write(fifofd, "stop\n", 5);
            }else if(strcmp(cmd, "mplayer mute 1")==0){
                write(fifofd, "mute 1\n", 7);
            }else if(strcmp(cmd, "mplayer mute 0")==0){
                write(fifofd, "mute 0\n", 7);
            }else if(strcmp(cmd, "mplayer pause")==0){
                write(fifofd, "pause\n", 6);
            }else if(strcmp(cmd, "mplayer resume")==0){
                write(fifofd, "help\n",5);
            }
        }else{
            pthread_exit(NULL);
        }
    }
}

void *readmsg(void *){
    char cmd[MAXLINE];
    while(1){
        if(play){
            if(read(pipe_fd[0], cmd, MAXLINE)>0){
                printf("%s\n", cmd);
            }
        }else{
            pthread_exit(NULL);
        }
    }
}

void client::playmusic(char *name){
    char url[MAXLINE];
    char fifoname[100];
//    strcpy(fifoname, fifo);
    sprintf(fifoname, "file=%s", fifo);

    sprintf(url, "http://%s:%s%s", host,port,name);

    if(access(fifo, F_OK|W_OK) < 0){
        mkfifo(fifo, S_IFIFO|0666);
    }
    fifofd = open(fifo, O_RDWR); // 创建有名管道
    if(pipe2(pipe_fd,O_NONBLOCK)<0){
        printf("%s\n", strerror(errno));
    }
    play =1;
    if(fork()==0){
        close(pipe_fd[0]);
        dup2(pipe_fd[1], 1);
//        execlp ("mplayer", "mplayer", "-slave", "-quiet", "-loop", "0", "-input", fifoname, url, NULL);
        execlp("mplayer","mplayer","-slave","-quiet","-input",fifoname,url,NULL);
    }else{
        sleep(5);
        close(pipe_fd[1]);
        pthread_create(&readbuf,NULL,writectl, NULL);
        pthread_create(&readbuf,NULL,readmsg, NULL);
    }
    wait(NULL);
}



void client::showhowtocontrol(){
    printf("\t\033[32m播放音乐：play <musicname>\t下载音乐：download <musicname>\t退出：quit\033[0m\r\n");
}

int client::checkformat(char *cmd, char *name){
    if(strcmp(cmd, "quit")==0){
        return 1;
    }
    if((strcmp(cmd, "download")==0 || strcmp(cmd, "play")==0) && strstr(name, ".mp3")!=NULL){
        return 1;
    }else{
        return 0;
    }
}

void destroy(int){
    printf("未链接成功\n");
    exit(0);
}

void client::startrequest(){
    //signal(SIGSEGV, destroy);
    //signal(SIGCHLD, sig_handle);
    signal(SIGINT, stopplay);

    char cmd[MAXLINE], content[20];
    char command[MAXLINE];
    rio_readinitb(&rio, sockfd);

    sendcmd("showlist","/showlist", rio);
    showhowtocontrol();

    while(1){
        printf("\n\033[34m>>>\033[0m");
        fflush(stdout);
        scanf("%s %s\n", cmd, content);
        int ret = checkformat(cmd, content);
        if(ret){
            sprintf(command, "/%s", content);
            if(strcmp(cmd, "play")==0){
                sendcmd("play",command, rio);
            }else if(strcmp(cmd, "quit")==0){
                sendcmd("quit", command, rio);
            }else if(strcmp(cmd, "download")==0){
                sendcmd("download", command, rio);
            }
        }else{
            printf("\033[5;41m命令有误\033[0m\r\n");
            showhowtocontrol();
            continue;
        }
    }
}

client * client::open_clientfd(char *hostname,  char *port){
    if((sockfd = socket(AF_INET, SOCK_STREAM, 0))==-1){
        fprintf(stderr, "%s\n", strerror(errno));
        return NULL;
    }
    printf("prepare for address\n");
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(port));
    addr.sin_addr.s_addr = inet_addr(hostname);
    int ret;
    if((ret = connect(sockfd, (struct sockaddr *)&addr, sizeof(addr)))==-1){
        fprintf(stderr, "%s\n", strerror(errno));
        return NULL;
    }else{
        char hostname[MAXLINE], port[MAXLINE];
        getnameinfo((struct sockaddr *)&addr, sizeof(addr), hostname, MAXLINE, port, MAXLINE, NI_NUMERICHOST|NI_NUMERICSERV);
        printf("connect success: hostname: %s  port: %s\n", hostname, port);
    }
    strcpy(this->host, hostname);
    strcpy(this->port, port);
    return this;
}
