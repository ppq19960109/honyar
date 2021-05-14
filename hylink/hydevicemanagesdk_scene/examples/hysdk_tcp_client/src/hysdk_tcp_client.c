#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include <sys/time.h>

#include "pthread_tool.h"
#include "error_no.h"
#include "test.h"
#include "epoll_api.h"
#include "log.h"

typedef struct client_s
{
	char acIp[16];
	int iPort;
}client_t;


int g_iClientNum = 0;
client_t g_astClient[16] = {0};
epoll_fd_t * g_pstClientFd = NULL;

int g_iClientFd = -1;


static int tcp_test_client()
{
	int i = 0;
	int iRet = 0;
	char buf[65536];
	int iEventNum = 0;
	epoll_event_t astEpollEven[16] = {0};
	for(i = 0; i < g_iClientNum; i++)
	{
		int iFd = -1;
		//创建套接字,即创建socket 
		iFd = socket(AF_INET, SOCK_STREAM, 0); 
		if(iFd < 0)
		{
			//创建失败
			printf("socket error\n");
			return -1;
		}

		//绑定信息，即命名socket
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_port = htons(g_astClient[i].iPort);
		addr.sin_addr.s_addr = inet_addr(g_astClient[i].acIp);

		//发起连接
		socklen_t addr_len = sizeof(addr);
		iRet = connect(iFd, (struct sockaddr*)&addr, addr_len);
		printf("g_iClientFd: %d\n", iFd);
		if(iRet < 0)
		{
			printf("connect error\n");
			return -1;
		}
		
		/*发送允许重复上报指令*/
		char tmp[] = "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"Ctrl\",\"Data\":[{\"DeviceId\":\"0000000000000000\",\"Key\":\"RepeatReport\",\"Value\":\"1\"}]}";
		send(iFd, tmp, strlen(tmp) + 1, 0);
		
		
		//创建监听事件
		iRet = g_pstClientFd->init_event(g_pstClientFd, iFd);
		if(iRet != 0)
		{
			printf("init_event error\n");
			return -1;
		}

		if(0 == i)
		{	
			g_iClientFd = iFd;
		}
	}
	
	while(1)
	{
		iEventNum = 16;
		g_pstClientFd->happen_event(g_pstClientFd, astEpollEven, &iEventNum, -1);
		if(iEventNum > 0)
		{
			for (i = 0; i < iEventNum; ++i)
			{
				memset(buf, 0x0, 65535);
				iRet = recv(astEpollEven[i].iFd, buf, 65536, 0);
				if(iRet > 0)
				{
					//printf("recv: %s\n", buf);
					testReportExec(buf);
				}
			}
		}
	}
}


static void* tcp_test(void *arg)
{
	sleep(30);
	while(1)
	{
		reset_all();
		scene_on_test();
		sleep(10);
	}

	return NULL;
}

int main(int argc, char *argv[])
{
	int i = 0;
	
	if(argc < 3 || 0 !=  ((argc - 1) % 2))
	{
		printf("Parameter error\n");
		return -1;
	}

	InitLog(LOG_DEBUG, "/tmp/aaa");
	LOG(LOG_DEBUG, "main\n");
	g_iClientNum = (argc - 1) / 2;
	for(i = 0; i < g_iClientNum; ++i)
	{
		strcpy(g_astClient[i].acIp, argv[2 * i + 1]);
		g_astClient[i].iPort = atoi(argv[2 * i + 2]);
	}

	for(i = 0; i < g_iClientNum; ++i)
	{
		printf("Client(%d) Ip = %s, Port = %d\n",i, g_astClient[i].acIp, g_astClient[i].iPort);
	}
	
	//创建测试线程
	int iTcpTestPid = 0;
	if(0 != pthread_detached_create(
		&iTcpTestPid, 
		tcp_test, 
		NULL)
	)
	{
		printf("tcp_test Error.\n");
        return -1;
	}

	//初始化epoll
	g_pstClientFd = new_epoll_fd(16);
	if(NULL == g_pstClientFd)
	{
		printf("new_epoll_fd error.\n");
		return -1;
	}
	//连接服务器
	
	tcp_test_client();
	
	
	return 0;
}

