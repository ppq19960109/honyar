#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h> /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "socket.h"

void perr_exit(const char *str)
{
    perror(str);
    // exit(1);
}

int Socket(int domain, int type)
{
    int sockfd = 0;
    sockfd = socket(domain, type, 0);
    if (sockfd < 0)
    {
        perr_exit("socket error\n");
    }

    return sockfd;
}

int Bind(int sockfd, const struct sockaddr *net_addr, socklen_t addrlen)
{
    if (!net_addr)
    {
        perr_exit("when binding, net_addr is NULL\n");
        return -1;
    }

    if (bind(sockfd, net_addr, addrlen) < 0)
    {
        perr_exit("fail to bind ipaddr\n");
        return -1;
    }

    return 0;
}

int Listen(int sockfd, int listenNum)
{
    if (listen(sockfd, listenNum) < 0)
    {
        perr_exit("fail to listen socket\n");
        return -1;
    }

    return 0;
}

int Connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    if (connect(sockfd, addr, addrlen) < 0)
    {
        perr_exit("connect error\n");
        return -1;
    }

    return 0;
}

int Accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    int ret;

again:
    ret = accept(sockfd, addr, addrlen);
    if (ret < 0)
    {
        if (errno == EINTR || errno == ECONNABORTED)
            goto again;
        else
            perr_exit("accept error\n");
    }
    return ret;
}
int Close(int fd)
{
    if (fd == 0)
    {
        return -1;
    }
    int n;
    if ((n = close(fd)) == -1)
        perr_exit("close error\n");

    return n;
}

ssize_t Read(int fd, void *ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ((n = read(fd, ptr, nbytes)) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            goto again;
        else
            return -1;
    }

    return n;
}

ssize_t Recv(int fd, void *ptr, size_t nbytes, int flag)
{
    ssize_t n;

again:
    if ((n = recv(fd, ptr, nbytes, flag)) == -1)
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK || errno == EINTR)
            goto again;
        else
            return -1;
    }

    return n;
}

ssize_t Write(int fd, const void *ptr, size_t nbytes)
{
    ssize_t n;

again:
    if ((n = write(fd, ptr, nbytes)) == -1)
    {
        if (errno == EINTR || errno == EAGAIN || errno == EWOULDBLOCK)
            goto again;
        else
            return -1;
    }
    return n;
}

/*??????: ????????????????????????*/ //socket 4096  readn(cfd, buf, 4096)   nleft = 4096-1500
ssize_t Readn(int fd, void *vptr, size_t n)
{
    size_t nleft;  //usigned int ???????????????????????????
    ssize_t nread; //int ????????????????????????
    char *ptr;

    ptr = (char *)vptr;
    nleft = n; //n ??????????????????

    while (nleft > 0)
    {
        if ((nread = read(fd, ptr, nleft)) < 0)
        {
            if (errno == EINTR) //??????????????????????????????
                nread = 0;
            else
                return -1; //?????? ?????? -1
        }
        else if (nread == 0) //???0????????? ????????????
            break;
        /*
         fd ?????????????????????vptr????????????????????????????????????????????????
         ???????????????????????????
         */
        nleft -= nread; //nleft = nleft - nread ????????????????????????
        ptr += nread;   //???????????????????????????
    }
    return n - nleft;
}

//?????????????????????
ssize_t Writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = (char *)vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0;
            else
                return -1;
        }
        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
} //?????????????????????????????????

int setNonBlock(int sockfd)
{
    int opt = 0;
    opt = fcntl(sockfd, F_GETFL);
    if (opt < 0)
    {
        printf("fcntl(sock,GETFL) failed\n");
        return -1;
    }

    opt = opt | O_NONBLOCK;
    if (fcntl(sockfd, F_SETFL, opt) < 0)
    {
        printf("fcntl(sock,SETFL,opts) failed\n");
        return -1;
    }

    return 0;
}
