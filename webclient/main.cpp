#include <iostream>
#include "client.h"
#include<stdio.h>
using namespace std;


int main(int argc, char **argv)
{
    if(argc!=3){
        fprintf(stderr, "usage: <hostname> <port>\n");
        return -1;
    }
    client cli;
    cli.open_clientfd(argv[1], argv[2])->startrequest();
}
