#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>


class ClientThread
{
private:
    pthread_t tid;
    int m_fd;
    ~ClientThread(){
        close(m_fd);
//        pthread_cancel(tid);
        printf("destroy success: %ld\n", tid);
    }
    char *ptr;
    static int fakemfd;
    static ClientThread *fakethis;
public:
    ClientThread(int *p, char *);
    void start();
    static void* run(void *arg);
    void service();
    void sendmusiclist();  //向客户端发送音乐列表
    void badrequestmethod(char *cause, int  errnum, char msg[], char *detail);
    void showmusiclist();
    void sendmp3file(char *,int);
    static void destroy(int);
//    void read_requestheader(rio_t &rio);
};

#endif // CLIENTTHREAD_H
