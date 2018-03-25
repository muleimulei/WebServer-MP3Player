#ifndef CLIENT_H
#define CLIENT_H

struct rio_t;
class client
{
private:
    int sockfd;
    char host[40];
    char port[10];

public:
    client();
    client *open_clientfd(char *, char *);
    void startrequest();
    void getResponseHeader(rio_t *rio, int&);
    void showmusiclist(rio_t *rio, int);
    void sendcmd(char *,char *, rio_t &);
    void showhowtocontrol();
    int checkformat(char *cmd, char *name);
    void playmusic(char *);
    void downloadmusic(long);
};

#endif // CLIENT_H
