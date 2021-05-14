#include <sys/socket.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include <ctype.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/ioctl.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "cJSON.h"
#include "hy_device_manage.h"
#include "pthreadPool.h"
#include "log.h"

#define GATEWAY_VERSION		"V1.2.9"
#define DATA_MAX_LENGTH		(2048)
#define OUT_MAX_LEMGTH		(16000)

int recvFd=-1;

static pthread_mutex_t mutex;
static pthread_mutex_t mutex_TcpFd;
static int listenFdNum = 0;

/*消息命令结构体，用于传递从客户端接收到的命令*/
typedef struct Message
{
	//char address[20];/*被控设备的地址，实际为16个字符*/
	//unsigned short commandType;/*命令类型代码*/
	//unsigned char endpoint;/*被控设备第几路*/
	char command[1024];/*命令类型对应的实际命令字符串为JSON格式*/
	int fd;/*客户端对应的文件描述符*/
}Message;

/*已和网关连接的客户端文件描述符链表的节点结构体*/
typedef struct TcpFd
{
	int fd;
	struct timeval tv; /*最后一次通信的时间，用于检查客户端是否断网*/
	char ip[16];
	struct TcpFd *next;
}TcpFd;

TcpFd *pTcpFdHead = NULL;
TcpFd *pTcpFdTail = NULL;

void addTcpFd(TcpFd *p)
{
	//int res;
	//printf("[Debug] Add a new client fd info into TCP list\n\n");
	pthread_mutex_lock(&mutex_TcpFd);
	//printf("[Debug] pthread_mutex_lock(&mutex_TcpFd)=%d\n\n", res);
	if (pTcpFdHead == NULL)
	{
		pTcpFdHead = p;
		pTcpFdTail = p;
	}
	else
	{
		pTcpFdTail->next = p;
		pTcpFdTail = p;
	}
	pthread_mutex_unlock(&mutex_TcpFd);
	//printf("[Debug] pthread_mutex_unlock(&mutex_TcpFd)=%d\n\n", res);
}

int deleteTcpFd(int fd)
{
	TcpFd *currentFd;
	TcpFd *preFd;
	TcpFd *deteFd;


	pthread_mutex_lock(&mutex_TcpFd);
	//printf("[Debug] Delete tcp fd: %d\n", res);
	preFd = pTcpFdHead;
	currentFd = pTcpFdHead;

	while (currentFd != NULL)
	{
		if (currentFd->fd == fd)
		{
			if (currentFd == pTcpFdHead && currentFd == pTcpFdTail)
			{
				pTcpFdHead = NULL;
				pTcpFdTail = NULL;
				deteFd = currentFd;
			}
			else if (currentFd == pTcpFdHead && currentFd != pTcpFdTail)
			{
				pTcpFdHead = currentFd->next;
				deteFd = currentFd;
			}
			else if (currentFd != pTcpFdHead && currentFd == pTcpFdTail)
			{
				preFd->next = NULL;
				pTcpFdTail = preFd;
				deteFd = currentFd;
			}
			else
			{
				preFd->next = currentFd->next;
				deteFd = currentFd;
			}
			free(deteFd);
			//printf("[Debug] Delet 1 client fd from TCP list\n");
			pthread_mutex_unlock(&mutex_TcpFd);
			//printf("[Debug] pthread_mutex_unlock(&mutex_TcpFd)=%d\n\n", res);;
			return 0;
		}
		else
		{
			preFd = currentFd;
			currentFd = currentFd->next;
		}
	}
	//printf("[Debug] Delet 0 client fd from TCP list\n");
	pthread_mutex_unlock(&mutex_TcpFd);
	//printf("[Debug]pthread_mutex_unlock(&mutex_TcpFd)=%d\n\n", res);
	return -1;
}

void updateTcpListTimeFlag(int fd)
{
	TcpFd *currentFd;
	//printf("[Debug] update TCP list node time info: fd=%d\n",fd);
	//int res;
	pthread_mutex_lock(&mutex_TcpFd);
	//printf("[Debug] pthread_mutex_lock(&mutex_TcpFd)=%d\n", res);
	currentFd = pTcpFdHead;
	while (currentFd != NULL)
	{
		if (currentFd->fd == fd)
		{
			gettimeofday(&currentFd->tv, NULL);
			break;
		}
		else
		{
			currentFd = currentFd->next;
		}
	}
	pthread_mutex_unlock(&mutex_TcpFd);
	//printf("[Debug] pthread_mutex_unlock(&mutex_TcpFd)=%d\n\n", res);
}


int sendTcp(int fd,char *out)
{
	int res = 0;
	int count = 0;
	char buf[OUT_MAX_LEMGTH]={0};
	int i;
	int num=0;
	buf[num]=0x02;
	num++;
	int outLen;
	outLen=strlen(out);
	for(i=0;i<outLen;i++)
	{
		if (*out!='\n' && *out!='\r' &&*out!='\t' && *out!=' ' && *out!=0x09)
		{
			buf[num]=*out;
			num++;
		}
		out++;
	}
	buf[num]=0x03;
	buf[num+1]='\0';
	if (fd == -1)
		res = 0;
	else
	{
		while (count < 3)
		{
			res = send(fd, buf, num+1, 0);
			if (res == -1)
				count++;
			else
			{
				DEBUG("\033[1;33m[Report]\033[0m to client: %s\n\n",buf);
				break;
			}
				
		}
	}
	return res;
}

void sendTcpToAllClient(char *out)
{
	int res;
	res = pthread_mutex_lock(&mutex_TcpFd);
	//printf("[Debug] sendTcpToAllClient: pthread_mutex_lock(&mutex_TcpFd)=%d\n\n", res);
	TcpFd *current = pTcpFdHead;
	TcpFd *preFd = pTcpFdHead;
	TcpFd *deteFd;
	if (listenFdNum != 0)
	{
		while (current != NULL)
		{
			res=sendTcp(current->fd, out);
			if (res <= 0)
			{

				if (current == pTcpFdHead && current == pTcpFdTail)
				{
					pTcpFdHead = NULL;
					pTcpFdTail = NULL;
					preFd=NULL;
					deteFd = current;
					current = NULL;
				}
				else if (current == pTcpFdHead && current != pTcpFdTail)
				{
					pTcpFdHead = current->next;
					preFd=current->next;
					deteFd = current;
					current = current->next;
				}
				else if (current != pTcpFdHead && current == pTcpFdTail)
				{
					preFd->next = NULL;
					pTcpFdTail = preFd;
					deteFd = current;
					current = NULL;
				}
				else
				{
					preFd->next = current->next;
					deteFd = current;
					current = current->next;
				}
				close(deteFd->fd);
				free(deteFd);
			}
			else
			{
				preFd = current;
				current = current->next;
			}
		}
	}
	res = pthread_mutex_unlock(&mutex_TcpFd);
	//printf("[Debug] sendTcpToAllClient: pthread_mutex_unlock(&mutex_TcpFd)=%d\n\n", res);
}

int sendTcpToClient(int fd,char *out)
{
	
	if (fd <0 )
	{
		sendTcpToAllClient(out);
		return 0;
	}
	else
	{
		TcpFd *currentFd,*preFd,*deteFd;
		pthread_mutex_lock(&mutex_TcpFd);
		currentFd = pTcpFdHead;
		preFd=pTcpFdHead;
		while (currentFd != NULL)
		{
			if (currentFd->fd == fd)
			{
				if (sendTcp(currentFd->fd, out) <= 0)
				{
					if (currentFd == pTcpFdHead && currentFd == pTcpFdTail)
					{
						pTcpFdHead = NULL;
						pTcpFdTail = NULL;
						preFd=NULL;
						deteFd = currentFd;
						currentFd = NULL;
					}
					else if (currentFd == pTcpFdHead && currentFd != pTcpFdTail)
					{
						pTcpFdHead = currentFd->next;
						preFd=currentFd->next;
						deteFd = currentFd;
						currentFd = currentFd->next;
					}
					else if (currentFd != pTcpFdHead && currentFd == pTcpFdTail)
					{
						preFd->next = NULL;
						pTcpFdTail = preFd;
						deteFd = currentFd;
						currentFd = NULL;
					}
					else
					{
						preFd->next = currentFd->next;
						deteFd = currentFd;
						currentFd = currentFd->next;
					}
					close(deteFd->fd);
					free(deteFd);
				}
				else
				{
					preFd = currentFd;
					currentFd = currentFd->next;
				}
			}
			else
			{
				preFd = currentFd;
				currentFd = currentFd->next;
			}

		}
		pthread_mutex_unlock(&mutex_TcpFd);
	}
	return 0;
}



int setnonblocking(int fd)
{
	int old_option = fcntl(fd, F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd, F_SETFL, new_option);
	return old_option;
}

void addfd(int epollfd, int fd)
{
	struct epoll_event ev;
	ev.data.fd = fd;
	ev.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
	setnonblocking(fd);
}

void threadTcpHeartBeat(void)
{
	TcpFd *currentFd;
	TcpFd *preFd;
	TcpFd *deteFd;

	//int res;
	struct timeval check;
	while (1)
	{
		sleep(60);
		if (listenFdNum != 0)
		{
			pthread_mutex_lock(&mutex_TcpFd);
			//printf("[Debug] threadTcpHeartBeat: pthread_mutex_lock(&mutex_TcpFd)=%d\n", res);

			preFd = pTcpFdHead;
			currentFd = pTcpFdHead;

			if (pTcpFdHead != NULL)
			{
				gettimeofday(&check, NULL);
				while (currentFd != NULL)
				{
					if (check.tv_sec - currentFd->tv.tv_sec > 70)
					{
						if (currentFd == pTcpFdHead && currentFd == pTcpFdTail)
						{
							pTcpFdHead = NULL;
							pTcpFdTail = NULL;
							deteFd = currentFd;
							preFd = NULL;
							currentFd = NULL;
						}
						else if (currentFd == pTcpFdHead && currentFd != pTcpFdTail)
						{
							pTcpFdHead = currentFd->next;
							deteFd = currentFd;
							preFd = currentFd->next;
							currentFd = currentFd->next;
						}
						else if (currentFd != pTcpFdHead && currentFd == pTcpFdTail)
						{
							preFd->next = NULL;
							pTcpFdTail = preFd;
							deteFd = currentFd;
							currentFd = NULL;
						}
						else
						{
							preFd->next = currentFd->next;
							deteFd = currentFd;
							currentFd = currentFd->next;
						}


						//deleteComFeedbackFd(deteFd->fd);
						listenFdNum--;
						DEBUG("deletes 1 offline client: %s, %d left now\n\n", deteFd->ip,listenFdNum);
						close(deteFd->fd);
						free(deteFd);
					}
					else
					{
						preFd = currentFd;
						currentFd = currentFd->next;
					}
				}
			}
			pthread_mutex_unlock(&mutex_TcpFd);
			//printf("[Debug] threadTcpHeartBeat: pthread_mutex_unlock(&mutex_TcpFd)=%d\n\n", res);
		}

	}
}

int handler_8sButten()
{
	return 0;
}

void handler_reportMessage(char *inBuff)
{
	DEBUG(">>%s\n\n",inBuff);
	sendTcpToAllClient(inBuff);

	cJSON *root;
	root=cJSON_Parse(inBuff);
	cJSON *type;
	if (root!=NULL)
	{
		type=cJSON_GetObjectItem(root,"Type");
		if (type!=NULL)
			if(strcmp("ReFactory",type->valuestring)==0)
			{
				sync();
				reboot(RB_AUTOBOOT);
			}

	}
}

int handleTcpBeatHeartResponse(int fd)
{
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Command","BeatHeartResponse");
	char *josnStr;
	josnStr=cJSON_Print(root);

	sendTcp(fd,josnStr);
	cJSON_Delete(root);
	if (josnStr!=NULL)
		free(josnStr);
	return 0;
}

int checkBeatHeart(char *inBuff)
{
	int res=-1;
	cJSON *root;
	root=cJSON_Parse(inBuff);
	if (root)
	{
		cJSON *command=cJSON_GetObjectItem(root,"Command");
		if (command)
		{
			if (command->type==cJSON_String)
			{
				if (strcmp(command->valuestring,"TcpBeatHeart")==0)
					res=0;
			}
		}
	}
	return res;
}
/*
void printfHex(unsigned char *src,int len)
{
	if(src==NULL)
	{
		return;
	}

	if(len>(1024*1024*3-1))
		return;

	char x[1024*1024*3]={0};
	int i=0;
	for (i=0;i<len;i++)
	{
		char tmp[10]={0};
		if(isprint(src[i]))
		{
			snprintf(tmp,8,"%c",src[i]);
			strcat(x,tmp);
		}
		else
		{
			snprintf(tmp,8,"(%X)",src[i]);
			strcat(x,tmp);
		}
	}
	printf("%s\n\n",x);
	return ;
}
*/
void service()
{
	int recvFd = -1;
	int epollFd;

	char recvBuf[DATA_MAX_LENGTH];

	struct sockaddr_in serverAddr;
	struct sockaddr_in clientAddr;

	bzero(&serverAddr, sizeof(serverAddr));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	serverAddr.sin_port = htons(7000);

	int fd;
	int opt = 1;
	fd = socket(AF_INET, SOCK_STREAM, 0);
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	int res;
	while(1)
	{
		res=bind(fd, (struct sockaddr*)&serverAddr, sizeof(struct sockaddr_in));
		if (res==0)
			break;
		else
		{
			DEBUG("\033[0;32;31m<Error>\033[m Bing socket error:");
			sleep(3);
		}
	}


	if (listen(fd, 20) == -1)
	{
		DEBUG("\033[0;32;31m<Error>\033[m %s\n\n", strerror(errno));
		return;
	}

	epollFd = epoll_create(20);
	struct epoll_event  events[20];
	addfd(epollFd, fd);

	int fdNumber;
	int i;

	
	pthread_mutex_init(&mutex, NULL);
	pthread_mutex_init(&mutex_TcpFd, NULL);

	int dataNum;
	//pool_init(4);

	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t pTcpHeartBeart;
	res = pthread_create(&pTcpHeartBeart, &attr, (void *)threadTcpHeartBeat, NULL);
	pthread_attr_destroy(&attr);
	if (res != 0)
	{
		DEBUG("\033[0;32;31m<Error>\033[0m Open TCP heartbeat thread error\n\n");
		return;
	}


	while (1)
	{
		fdNumber = epoll_wait(epollFd, events, 20, 5000);
		switch (fdNumber)
		{
		case -1:
			DEBUG("\033[0;32;31m<Error>\033[m epoll_waite error\n\n");
			continue;
		case 0:
			//printf("[Debug] time out...\n");
			usleep(50000);
			continue;
		}

		for (i = 0; i < fdNumber; i++)
		{
			if ((events[i].events & EPOLLERR) ||
				(events[i].events & EPOLLHUP) ||
				(!(events[i].events & EPOLLIN)))
			{
				recvFd = events[i].data.fd;
				//printf("\033[0;32;31m<Error>\033[m Epoll error: events[%d].data.fd=%d\n\n", i, events[i].data.fd);
				if (deleteTcpFd(recvFd) == 0)
				{
					listenFdNum--;
					DEBUG("Delete 1 client fd from TCP list, %d left now!\n", listenFdNum);
				}
				else
				{
					//printf("\033[0;32;31m<Warning>\033[m Delete 0 client fd from TCP list, %d left now!\n\n", listenFdNum);
				}
				//deleteComFeedbackFd(recvFd);

				//int tr;
				close(recvFd);
				DEBUG("Close 1 client fd= %d\n\n", recvFd);
				continue;
			}
			else if (events[i].data.fd == fd)/*TCP*/
			{

				int sizeAddr;
				sizeAddr = sizeof(struct sockaddr_in);

				while (1)
				{
					recvFd = accept(fd, (struct sockaddr*)&clientAddr, (socklen_t *)&sizeAddr);
					//printf("[Debug] hygateway server accept(fd)=%d\n\n", recvFd);
					if (recvFd == -1)
					{
						if ((errno == EAGAIN) || (errno == EWOULDBLOCK))
						{
							break;
						}
						else
						{
							break;
						}
					}
					addfd(epollFd, recvFd);
					listenFdNum++;
					INFO("Accept a new client: %s, now total %d clients\n", inet_ntoa(clientAddr.sin_addr), listenFdNum);

					TcpFd *newFd;
					newFd = (TcpFd*)malloc(sizeof(TcpFd));
					memset(newFd,0,sizeof(TcpFd));
					sprintf(newFd->ip,"%s",inet_ntoa(clientAddr.sin_addr));
					gettimeofday(&newFd->tv, NULL);

					newFd->fd = recvFd;
					newFd->next = NULL;
					addTcpFd(newFd);/*加入到已连接的client fd 链表中*/
				}
			}
			else if (events[i].events & EPOLLIN)
			{
				if ((recvFd = events[i].data.fd) < 0)
					continue;
				while(1)
				{
				memset(recvBuf, 0, DATA_MAX_LENGTH);
				dataNum = recv(recvFd, recvBuf, DATA_MAX_LENGTH, 0);
				if (dataNum < 0)
				{
					if (errno == EAGAIN)
					{
						//printf("\033[0;32;31m<Error>\033[m TCP read  error:EAGAIN\n");
						break;
					}
					else if (errno == ECONNRESET)
						ERROR("\033[0;32;31m<Error>\033[m TCP Read error: ECONNRESET!\n");
					else
						ERROR("\033[0;32;31m<Error>\033[m TCP read error!\n");

					if (deleteTcpFd(recvFd) == 0)
					{
						listenFdNum--;
						INFO("Delete 1 client fd=%d from TCP list,%d left now!\n\n",recvFd, listenFdNum);
					}
					else
					{
						//printf("[Debug] Delete 0 TCP fd from list!\n\n");
					}
					//deleteComFeedbackFd(recvFd);
					close(recvFd);
					break;
				}
				else if (dataNum == 0)
				{
					DEBUG("\033[0;32;31m[Warning]\033[m TCP Receive  data: 0. Client active close connection\n\n");
					if (deleteTcpFd(recvFd) == 0)
					{
						listenFdNum--;
						DEBUG("Delete 1 client fd=%d from TCP list,%d left now!\n\n", recvFd,listenFdNum);
					}
					//else
					//{
						//printf("[Debug] Delete 1 client fd from TCP list,%d left now!\n\n", listenFdNum);
					//}
					//deleteComFeedbackFd(recvFd);
					close(recvFd);
					break;
				}
				else
				{
					updateTcpListTimeFlag(recvFd);
					DEBUG("\033[1;33m[From]\033[0m client %s receives %d data\n\n", inet_ntoa(clientAddr.sin_addr), dataNum);
					
					int i = 0;
					char commandBuf[2048];
					while (1)
					{
						if (recvBuf[i] == 0x02)
						{
							memset(commandBuf, 0, 2048);
							i++;
							int j = 0;
							while (recvBuf[i] != 0x03 && recvBuf[i] != '\0')
							{
								commandBuf[j] = recvBuf[i];
								j++;
								i++;
							}
							commandBuf[j] = '\0';
							DEBUG("\033[1;33m[Receive]\033[0m command from  %s >>>>>>>>>%d\n",inet_ntoa(clientAddr.sin_addr),strlen(commandBuf));
							DEBUG(" %s\n\n", commandBuf);

							if (0==checkBeatHeart(commandBuf))
							{
								handleTcpBeatHeartResponse(recvFd);
							}
							else
							{

								char outBuff[OUT_MAX_LEMGTH]={0};
								int outLen=0;

								//printfHex((unsigned char *)commandBuf,strlen(commandBuf));

								hyMsgProcess_SyncQuery(commandBuf,strlen(commandBuf)+1,outBuff,OUT_MAX_LEMGTH,&outLen);
								//hyMsgProcess_AsynQuery(commandBuf,strlen(commandBuf)+1);
								if (outLen>0)
									sendTcpToClient(recvFd,outBuff);

							}
							if (recvBuf[i] == '\0')
								break;
						}
						else if (recvBuf[i] == '\0')
						{

							break;
						}
						else
						{

							i++;
						}
					}
				}
				}
			}
		}
	}
}
