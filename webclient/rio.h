#ifndef RIO_H
#define RIO_H

#include<sys/types.h>
#include<sys/socket.h>
#include<netdb.h>
#include<string.h>
#include<unistd.h>
#include <errno.h>
#include <stdio.h>
#include<stdlib.h>
#include<stdio.h>

#define RIO_BUFSIZE 8192
typedef struct rio_t{
    int rio_fd;
    int rio_cnt;
    char *rio_bufptr;
    char rio_buf[RIO_BUFSIZE];
} rio_t;

void rio_readinitb(rio_t *rp, int fd){
    rp->rio_fd = fd;
    rp->rio_cnt = 0;
    rp->rio_bufptr = rp->rio_buf;
}

ssize_t rio_read(rio_t *rp, char *usrbuf, size_t n){
    int cnt;
    while(rp->rio_cnt<=0){
        rp->rio_cnt = read(rp->rio_fd, rp->rio_buf, sizeof(rp->rio_buf));
        if(rp->rio_cnt < 0){
            if(errno!=EINTR) return -1;
        }
        else if (rp->rio_cnt ==0 ) return 0; //eof
        else rp->rio_bufptr = rp->rio_buf;
    }

    cnt = n;
    if(rp->rio_cnt < n){
        cnt = rp->rio_cnt;
    }
    memcpy(usrbuf, rp->rio_bufptr, cnt);
    rp->rio_bufptr += cnt;
    rp->rio_cnt -= cnt;
    return cnt;
}


ssize_t rio_readlineb(rio_t *rp, char *usrbuf, size_t maxlen)
{
    int n, rc;
    char c, *bufp = usrbuf;
    for(n = 1; n<maxlen; n++){
        if((rc = rio_read(rp, &c,1))==1){
            *bufp++ = c;
            if(c=='\n'){
                n++;
                break;
            }
        }else if(rc==0){
                if(n==1){
                    return 0;
                }else{
                    break;
                }
        }else{
            return -1;
        }
    }
    *bufp = 0;
    return n-1;

}

ssize_t rio_readnb(rio_t *rp, char *usrbuf, size_t n){
    size_t nleft = n;
    ssize_t nread;
    char *bufp = usrbuf;

    while(nleft>0){
        if((nread = rio_read(rp, bufp, nleft))<0){
            return -1;
        }else if(nread==0){
            break;
        }
        nleft -= nread;
        bufp+=nread;
    }
    return n - nleft;
}


ssize_t rio_readn(int fd, char *usrbuf, size_t n){
    size_t nleft = n;
    size_t nread;
    char *bufp = usrbuf;
    while(nleft>0){
        if((nread = read(fd, bufp, nleft))<0){
            if(errno == EINTR){
                nread = 0; // 被中断
            }else{
                return -1;
            }
        }else if(nread == 0){
            break;
        }
        nleft -= nread;
        bufp += nread;
    }
    return (n - nleft);
}

ssize_t rio_writen(int fd, char *usrbuf, size_t n){
    size_t nleft = n;
    ssize_t nwritten;
    char *bufp = usrbuf;

    while(nleft>0){
        if((nwritten = write(fd, bufp, nleft))<=0){
            if(errno == EINTR){
                nwritten = 0; // 被中断
            }else{
                return -1;
            }
        }
        nleft -= nwritten;
        bufp += nwritten;
    }
    return n;
}



#endif // RIO_H
