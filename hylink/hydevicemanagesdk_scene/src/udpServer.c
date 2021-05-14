/*
 * udpServer.c
 *
 *  Created on: Oct 9, 2018
 *      Author: jerry
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <arpa/inet.h>
#include <errno.h>
#include "cJSON.h"
#include <sys/ioctl.h>
#include <net/if.h>
#include "tool.h"
#include "deviceList.h"
#include "log.h"

int sockListen;
struct sockaddr_in clientAddr;
socklen_t addrLen = sizeof(struct sockaddr_in);

#define SERVER_PORT   "7000"
#define RECEIVE_BUFF_LENGTH    (500)
#define UDP_COMMAND_REQUEST_TCP  (1)
#define UDP_COMMAND_OPEN_LOG  (2)
#define UDP_COMMAND_CLOSE_LOG  (3)

int getIP(char *eth, char *ipaddr)
{
	int sock_fd;
	struct sockaddr_in my_addr;
	struct ifreq ifr;

	if ((sock_fd = socket(PF_INET, SOCK_DGRAM, 0)) == -1)
	{
		perror("socket");
		close(sock_fd);
		return -1;
	}

	/* Get IP Address */
	strncpy(ifr.ifr_name, eth, IF_NAMESIZE);
	ifr.ifr_name[IFNAMSIZ - 1] = '\0';

	if (ioctl(sock_fd, SIOCGIFADDR, &ifr) < 0)
	{
		close(sock_fd);
		return -1;
	}

	memcpy(&my_addr, &ifr.ifr_addr, sizeof(my_addr));
	sprintf(ipaddr, "%s", inet_ntoa(my_addr.sin_addr));
	close(sock_fd);
	return 0;
}

int getNetworkCard(char *eth, char *output)
{
	struct ifreq ifreq;
	int sock = 0;
	sock = socket(AF_INET,SOCK_STREAM,0);

	if(sock < 0)
	{
        perror("error sock");
		return -1;
	}

	strcpy(ifreq.ifr_name,eth);
	if(ioctl(sock,SIOCGIFHWADDR,&ifreq) < 0)
	{
		perror("error ioctl");
		return 3;
	}

	int i = 0;
	for(i = 0; i < 6; i++){
        sprintf(output+2*i, "%02X", (unsigned char)ifreq.ifr_hwaddr.sa_data[i]);
	}
	output[strlen(output)] = 0;
	//printf("MAC: %s\n", output);
	close(sock);
	return 0;
}



void respCommandReuqestTcp()
{
	cJSON *root;
    root=cJSON_CreateObject();
    cJSON_AddStringToObject(root, "Command", "Response");
    cJSON_AddStringToObject(root, "Port", SERVER_PORT);
    char tmp[18]={0};
    getIP("eth0.2",tmp);
    if (tmp[0]=='\0')
    	 getIP("apcli0",tmp);
    cJSON_AddStringToObject(root, "Ip", tmp);

    procedure_core_get(tmp);
    cJSON_AddStringToObject(root, "Code", tmp);
    procedure_model_get(tmp);
    cJSON_AddStringToObject(root, "Model", tmp);
    procedure_ver_get(tmp);
    cJSON_AddStringToObject(root, "Ver", tmp);
    getCooMac(tmp);
    cJSON_AddStringToObject(root, "Mac", tmp);
    getCooVer(tmp);
    cJSON_AddStringToObject(root, "CooVer", tmp);


    getNetworkCard("eth0.2",tmp);
    cJSON_AddStringToObject(root, "HWaddr", tmp);

    char *jsonStr;
    jsonStr=cJSON_Print(root);

    sendto(sockListen, jsonStr, strlen(jsonStr), 0,(struct sockaddr *)&clientAddr, addrLen);
    INFO("[ZIGBEE] Report to client: %s\n\n",jsonStr);
    cJSON_Delete(root);
	if (jsonStr)
		free(jsonStr);
}


int checkRecvMessage(char *recvBuff)
{
	cJSON *root;
	root=cJSON_Parse(recvBuff);
	int res=-1;

	if (root!=NULL)
	{
		cJSON *command;
		command=cJSON_GetObjectItem(root, "Command");
		if (command!=NULL)
		{
			if (command->type==cJSON_String)
			{
				if (strcmp(command->valuestring,"RequestTcp")==0)
				{
					res=UDP_COMMAND_REQUEST_TCP;
				}
				else if(strcmp(command->valuestring,"OpenLog")==0)
		        {
					res=UDP_COMMAND_OPEN_LOG;
		        }
				else if(strcmp(command->valuestring,"CloseLog")==0)
		        {
					res=UDP_COMMAND_CLOSE_LOG;
		        }
				else
					res=-1;
			}
			else
			{
				res=-1;
			}
		}
		else
			res=-1;
	}
	else
		res=-1;
	cJSON_Delete(root);

	return res;
}



int udpServer()
{

	if((sockListen = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
	{
		ERROR("[ZIGBEE] Creat UDP socket fail\n");
		return -1;
	}
	int set = 1;
	setsockopt(sockListen, SOL_SOCKET, SO_BROADCAST, &set, sizeof(int));
	struct sockaddr_in serverAddr;
	memset(&serverAddr, 0, sizeof(struct sockaddr_in));
	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8000);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	    // 必须绑定，否则无法监听
	if(bind(sockListen, (struct sockaddr *)&serverAddr, sizeof(struct sockaddr)) == -1)
	{
		ERROR("[ZIGBEE] UDP server bind fail\n");
		return -1;
	}
	int recvbytes;
	char recvbuf[RECEIVE_BUFF_LENGTH];

	while(1)
	{
		if((recvbytes = recvfrom(sockListen, recvbuf, RECEIVE_BUFF_LENGTH, 0,(struct sockaddr *)&clientAddr, &addrLen))!= -1)
		{
	        recvbuf[recvbytes] = '\0';
	        INFO("[ZIGBEE] UDP server receive a broadcast messgse:%s\n\n", recvbuf);
	        switch(checkRecvMessage(recvbuf))
	        {
	        case UDP_COMMAND_REQUEST_TCP:
	        	respCommandReuqestTcp();
	        	break;
	        case UDP_COMMAND_OPEN_LOG:
	        	SetUdpBroadcastFalg(1);
	        	break;
	        case UDP_COMMAND_CLOSE_LOG:
	        	SetUdpBroadcastFalg(0);
	        	break;
	        default:
	        	break;

	        }
		}
		else
		{
			WARN("[ZIGBEE] UDP server recvfrom fail\n");
			if (errno == EINTR)
				continue;
		}

	}
	close(sockListen);
	return 0;
}

