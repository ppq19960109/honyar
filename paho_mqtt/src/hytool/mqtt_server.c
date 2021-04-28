#include "main.h"

#include "frameCb.h"

#define USE_EPOLL
#ifdef USE_EPOLL
#include "epollServer.h"
static struct EpollTcpEvent hylink_myevents;
#else
#include "threadServer.h"
ThreadTcpServer hylink_threadTcpServer;
#endif

static int hylink_recv(char *data, unsigned int len)
{
    printf("---hylink_recv:%d,%s ...\n", len, data);
    // hylinkDispatch(data, len);
    runTransferCb(data, len, TRANSFER_CLIENT_WRITE);
    return 0;
}

static int hylink_disconnect(void)
{
    printf("---disconnect ...\n");
    return 0;
}

static int hylink_connect(void)
{
    printf("---hylink_connect ...\n");
    runSystemCb(CMD_DEVSINFO);
    return 0;
}

static int hylink_send(void *data, unsigned int len)
{
#ifdef USE_EPOLL
    return epollServerSend(&hylink_myevents, data, len);
#else
    return threadServerSend(&hylink_threadTcpServer, data, len);
#endif
}

int mqtt_serverClose(void)
{
#ifdef USE_EPOLL
    epollServerClose();
#else
    threadServerClose();
#endif
    return 0;
}

#define TCP_ADDR "0.0.0.0" //"127.0.0.1"
int mqtt_serverOpen(void)
{
    registerTransferCb(hylink_send, TRANSFER_SERVER_HYLINK_WRITE);

#ifdef USE_EPOLL
    epollTcpEventSet(&hylink_myevents, TCP_ADDR, LOCAL_TCP_PORT, hylink_recv, hylink_disconnect, hylink_connect, 1);
    epollServerOpen(2000);
#else
    tcpEventServerSet(&hylink_threadTcpServer, TCP_ADDR, LOCAL_TCP_PORT, hylink_recv, hylink_disconnect, hylink_connect, 2);
    threadServerOpen();
#endif
    return 0;
}
