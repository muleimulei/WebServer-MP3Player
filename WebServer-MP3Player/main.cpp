#include <iostream>
#include "server.h"
using namespace std;
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include <errno.h>
#include <stdio.h>
#include<stdlib.h>
#include<stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <dirent.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <stdlib.h>
#include <sys/wait.h>
#include "clientthread.h"
#include <signal.h>
#define MAXSIZE 8192

#define MAXLINE 100

void readMusicslist(char *buf, char currentdir[MAXLINE]){
        char ss[100];
        getcwd(ss, 100);
        printf("%s", ss);
    memset(buf, 0, sizeof(buf));
    DIR *cur;
    if((cur = opendir(currentdir))==NULL){
         printf("文件夹不存在\n");
         fflush(stdout);
         exit(0);
    }
    struct dirent *dir;
    while((dir = readdir(cur))!=NULL){
//        printf("%s\n",dir->d_name);
        if(strcmp(dir->d_name, ".")!=0 && strcmp(dir->d_name, "..")!=0){
            if(!strlen(buf)){
                sprintf(buf,"%s~",dir->d_name);
            }else{
                sprintf(buf,"%s%s~",buf,  dir->d_name);
            }
        }
    }
    buf[strlen(buf)] = 0;
}

int main(int argc, char **argv)
{
    char hostname[MAXLINE], port[MAXLINE];
    int listenfd,*p;

    char musiclist[100];
    signal(SIGPIPE, SIG_IGN);

    sockaddr_storage clientaddr;
    socklen_t clientlen;

    //检查命令行参数
    if(argc!=2){
        fprintf(stderr, "usage: %s <port>\n", argv[0]);
        exit(1);
    }

    readMusicslist(musiclist, "./media");
    Server server;
    listenfd = server.open_listenfd(argv[1]);

    while(1){
        clientlen = sizeof(sockaddr_storage);
        p = (int *)malloc(sizeof(int));
        *p = accept(listenfd, (sockaddr *)&clientaddr, &clientlen);
        getnameinfo((struct sockaddr *)&clientaddr, clientlen,hostname, MAXLINE, port, MAXLINE, 0);
        printf("ACCEPT connection from (%s, %s)\n", hostname, port);
        (new ClientThread(p, musiclist))->start();
    }

    return 0;
}

