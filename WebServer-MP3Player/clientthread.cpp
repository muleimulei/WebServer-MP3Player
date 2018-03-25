#include "clientthread.h"
#include <stdio.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include<pthread.h>
#include "rio.h"
#include <signal.h>


#define MAXLINE 3000

int clientfd;

int ClientThread::fakemfd;
ClientThread  *ClientThread::fakethis = NULL;
ClientThread::ClientThread(int *p, char *ptr)
{
    m_fd = *p;

    this->ptr = ptr;
    ClientThread::fakemfd = m_fd;
    clientfd = m_fd;
    ClientThread::fakethis = this;
    delete p;
}

void ClientThread::start(){
    if(pthread_create(&this->tid,NULL,run,this)<0){
        fprintf(stderr, "Thread create failed\n");
        return;
    }else{
        pthread_detach(this->tid);
        fprintf(stdout, "Thread create sucessful\n");
    }
}

void* ClientThread::run(void *arg){
    ClientThread *client = (ClientThread *)arg;
    client->service();

    delete client;
    fflush(stdout);
    return NULL;
}

void ClientThread::destroy(int){
    printf("客户端退出\n");
    //close(fakemfd);
    delete ClientThread::fakethis;
    return;
}

void sig_alarm(int){
    rio_writen(clientfd, "\r\n", 2);
    printf("%d\t", clientfd);
    alarm(5);
}

void ClientThread::service(){
    signal(SIGPIPE, SIG_IGN);
    signal(SIGSEGV, destroy);
//    signal(SIGALRM, sig_alarm);
//    alarm(5);



    rio_t rio;
    rio_readinitb(&rio, m_fd);

    char buf[MAXLINE], method[MAXLINE], uri[MAXLINE], version[MAXLINE];

    memset(buf, 0, sizeof(buf));
    memset(method, 0, sizeof(method));
    memset(uri, 0, sizeof(uri));
    memset(version, 0, sizeof(version));


    while(1){
        rio_readlineb(&rio, buf, MAXLINE);
        if(strcmp(buf,"\r\n")==0){
            continue;
        }
        sscanf(buf, "%s %s %s", method, uri, version);
        if(strstr(method, "GET")==NULL){
            badrequestmethod(method, 501, "Not implemented", "This Web doexn't support this method");
        }else{
            //rio_readlineb(&rio, buf, MAXLINE);
            //printf("%s\n", buf);
//            while(strcmp(buf,"\r\n")){
//                rio_readlineb(&rio, buf, MAXLINE);
//                printf("%s\n", buf);
//            }
            if(strstr(uri, "/showlist")!=NULL){
                showmusiclist();
            }else if(strstr(uri, "quit")!=0){
                break;
            }else if(strstr(uri, ".mp3")!=NULL){
                char filename[MAXLINE];
                sprintf(filename, "./media%s", uri);
                struct stat filestate;
                if(stat(filename,&filestate)<0){
                    badrequestmethod(filename, 404,"Not found", "This Web can't find this file");
                }else{
                    sendmp3file(filename, filestate.st_size);
                }
            }else{
                printf("error\n");
            }
        }
        memset(buf, 0, sizeof(buf));
        memset(method, 0, sizeof(method));

    }
}


void ClientThread::sendmp3file(char *filename, int filesize){
    char buf[MAXLINE];
    sprintf(buf, "HTTP/1.0 200 ok\r\n");
    sprintf(buf, "%sServer: Tiny Web Server\r\n", buf);
    sprintf(buf, "%sConnection: close\r\n", buf);
    sprintf(buf, "%sContent-type: audio/mp3\r\n", buf);
    sprintf(buf, "%sContent-length: %d\r\n\r\n", buf, filesize);
    rio_writen(m_fd, buf, strlen(buf));

    int srcfd = open(filename, O_RDONLY, 0);
    char *srcp = (char *)mmap(0, filesize, PROT_READ, MAP_PRIVATE, srcfd, 0);
    if((void *)srcp == (void *)-1) {
        badrequestmethod(filename, 500, "服务器内部错误", "This Web crashed");
        close(m_fd);
        return;
    }
    close(srcfd);
    int n = rio_writen(m_fd, srcp, filesize);
    munmap(srcp, filesize);
    printf("%d\n", n);
}


void ClientThread::showmusiclist(){
    char buf[MAXLINE];
    sprintf(buf, "HTTP/1.0 200 OK\r\n");
    sprintf(buf, "%sContent-length: %d\r\n",buf,strlen(ptr));
    sprintf(buf, "%sContent-Type: text/plain\r\n\r\n", buf);
    rio_writen(m_fd, buf, strlen(buf));
    rio_writen(m_fd, ptr, strlen(ptr));
}

void ClientThread::badrequestmethod(char *cause, int  errnum, char shortmsg[], char *longmsg){
    char buf[MAXLINE], body[MAXLINE];
    // 构建
    sprintf(body, "<html><title>Tiny Error</title>");
    sprintf(body,"%s<body bgcolor='#fff'>\r\n", body);
    sprintf(body,"%s%d: %s\r\n", body, errnum, shortmsg);
    sprintf(body, "%s<p>%s: %s\r\n", body, longmsg, cause);
    sprintf(body, "%s<hr><em>The Tiny Web server</em>\r\n", body);

    // print Http response
    sprintf(buf, "HTTP/1.0 %d %s\r\n", errnum, shortmsg);
    rio_writen(m_fd, buf, strlen(buf));

    sprintf(buf, "Content-Type: text/html\n"
                 "\r");
    rio_writen(m_fd, buf, strlen(buf));

    sprintf(buf, "Content-length: %ld\r\n\r\n", strlen(body));
    rio_writen(m_fd, buf, strlen(buf));
    rio_writen(m_fd, body, strlen(body));
}
