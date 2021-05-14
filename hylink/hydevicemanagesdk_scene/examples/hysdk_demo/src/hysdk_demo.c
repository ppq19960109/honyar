#include "hy_device_manage.h"
#include <stdio.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include<sys/time.h>

int g_iConnectFd = -1;

/*定义上报回调函数*/
int testReport(char *buff)  
{
	printf("[Debug] testReport: %s\n\n",buff);	
	/*将数据发送到tcp套接字中*/
	if(g_iConnectFd > 0)
	{
		send(g_iConnectFd, buff, strlen(buff) + 1, 0);
	}
	
	return 0;
}

/*定义复位按键长按8s事件回调函数，若不使用则返回0*/
int testButten8s()
{
	return 0;
}
int tcp_test_server(void)
{
	int iRet = 0;

	char buf[65536];
	char buf_out[65536];
	int outLen = 0;
    int listenfd;  /* socket descriptors */
    struct sockaddr_in server; /* server's address information */
    struct sockaddr_in client; /* client's address information */
    socklen_t addrlen;


	 /* Create TCP socket */
	if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		/* handle exception */
		perror("socket() error. Failed to initiate a socket");
		exit(1);
	}
 
	/* set socket option */
	int opt = SO_REUSEADDR;
	setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));

	bzero(&server, sizeof(server));

	server.sin_family = AF_INET;
	server.sin_port = htons(8888);
	server.sin_addr.s_addr = htonl(INADDR_ANY);
	if(bind(listenfd, (struct sockaddr *)&server, sizeof(server)) == -1)
	{
		/* handle exception */
		perror("Bind() error.");
		exit(1);
	}
	
	if(listen(listenfd, 10) == -1)
	{
		perror("listen() error. \n");
		exit(1);
	}

	addrlen = sizeof(client);
	while(1){
		if((g_iConnectFd=accept(listenfd,(struct sockaddr *)&client, &addrlen))==-1)
		  {
			perror("accept() error. \n");
			exit(1);
		  }

		struct timeval tv;
		gettimeofday(&tv, NULL);
		printf("You got a connection from client's ip %s, port %d at time %ld.%ld\n",inet_ntoa(client.sin_addr),htons(client.sin_port), tv.tv_sec,tv.tv_usec);
		printf("g_iConnectFd: %d\n\n",g_iConnectFd);	
		int iret=-1;
		while(1)
		{
			memset(buf, 0x0, 65535);
			iret = recv(g_iConnectFd, buf, 65536, 0);
			if(iret > 0)
			{
				char *pstr = buf;
				char *pStrtokPtr = NULL;
				char *pDate = NULL;
				while((pDate = strtok_r(pstr, "\r\n", &pStrtokPtr))!= NULL)
				{
					struct timeval stNow;
					gettimeofday(&stNow, NULL);
					printf("[%lu.%lu]recv: %s\n",stNow.tv_sec, stNow.tv_usec / 1000, pDate);
					outLen = 0;
					memset(buf_out, 0x0, 65535);
					iRet = hyMsgProcess_SyncQuery(pDate, strlen(pDate) + 1, buf_out, 65536, &outLen);
					if(0 == iRet)
					{
						if(0 == outLen)
						{
							strcpy(buf_out, "OK");
							outLen = strlen(buf_out) + 1;
							send(g_iConnectFd, buf_out, outLen, 0);
						}
						else
						{
							send(g_iConnectFd, buf_out, outLen, 0);
						}
					}
					else
					{
						sprintf(buf_out, "ERROR, iRet = %d", iRet);
						outLen = strlen(buf_out) + 1;
						send(g_iConnectFd, buf_out, outLen, 0);
					}
					pstr = NULL;
				}
			}
			else
			{
				close(g_iConnectFd);
				g_iConnectFd = -1;
				break;
			}
			/* print client's ip and port */
			 /* send to the client welcome message */
		}
	}
	close(listenfd); /* close listenfd */
	
	return 0;
}
int main(int argc, char *argv[])
{
	//printf("argc = %d\n", argc);
	//printf("argv = %s\n", argv[1]);
	int ulQueryMinInterval = 0;
	int iPrintFlag = 0;
	if(argc >= 2)
	{
		ulQueryMinInterval = (unsigned long)atol(argv[1]);
	}
	printf("ulQueryMinInterval = %d\n", ulQueryMinInterval);
	if(argc >= 3)
	{
		iPrintFlag = (unsigned long)atol(argv[2]);
	}
	printf("iPrintFlag = %d\n", iPrintFlag);
	/*初始化场景*/
	hyLocalSceneInit("/etc/gateway/scene_conf.db", 0);
	/*注册上报回调函数*/
	hySetReportFunHandler(testReport);
	/*注册复位按键长按8s事件回调函数*/
	hySetResetButen8sFunHandler(testButten8s);
	/*设置设备管理数据库路径*/
	hySetDeviceManageDataBasePath("/etc/gateway/");
	/*设置设备配置文件路径*/
	hySetConfigurFilePath("/etc/gateway/configure");
	/*设置COO下载升级文件路径*/
	hySetFileDownLoadPath("/tmp/");
	/*设置请求最小时间间隔*/
	hyQueryMinIntervalSet(ulQueryMinInterval);
	
	if(!iPrintFlag)
	{
		hySetLogInit(5, "/tmp/log_hysdk");
	}
	
	/*启动网关*/
	hyGatewayRun();
	/*插入开发者操作*/
	while(1)
	{
		tcp_test_server();
	}
	return 0;
}

