#ifndef SERVER_H
#define SERVER_H
#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include <errno.h>
#include <stdio.h>
#include<stdlib.h>
#include<stdio.h>
class Server{
public:
    int open_listenfd(char *port);
};


#endif // SERVER_H
