
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "log.h"
#include "cJSON.h"
#include <pthread.h>
#include "rex_export_gateway.h"
#include "callback.h"
#include "rexCommand.h"

/*快速入网设备信息结构体*/
typedef struct fastJoinInfo
{
	char address[18];  //设备地址
	int result;       //设备入网结果 1成功，0失败
	int reportCount;  //下发搜索次数，当==2次时，不再搜索；
	//int reportCount;        //结果上报次数，当等于三次时，换下一批地址下发
	struct fastJoinInfo *next;
}FastJoinInfo;


static FastJoinInfo *headFastDevice = NULL;  /*快速入网设备信息链表头指针*/
static FastJoinInfo *tailFastDevice = NULL;  /*快速入网设备信息链表尾指针*/

//static pthread_mutex_t mutexFastJoinTable;/*设备快速加入网关链表线程锁*/

int closeFastJoinFalg = 0;    /*允许关闭快速入网功能标志位*/

static int sendFastJoinCommand = 0; /*允许下发快速入网指令，1-允许，0-不允许*/

/********************************************************
函数名：createNewFastJoinInfoNode
描  述：创建快速加入设备信息结构体节点
参  数：@char *address：设备地址，16个字节
返  回：成功返回FastJoinInfo *指针，失败NULL
*********************************************************/
static FastJoinInfo * createNewFastJoinInfoNode(char *address)
{
	FastJoinInfo *newNode;
	newNode= (FastJoinInfo*)malloc(sizeof(FastJoinInfo));
	if (newNode == NULL)
		return NULL;
	sprintf(newNode->address, "%s", address);
	//newNode->ckeckCount = 0;
	newNode->reportCount = 0;
	newNode->result = 0;
	newNode->next = NULL;
	return newNode;
}


/********************************************************
函数名：addNewFastJoinNodeToList
描  述：加入设备信息链表
参  数：@FastJoinInfo *newNode：待加入快速入网设备信息链表的节点指针
返  回：空
*********************************************************/
static void addNewFastJoinNodeToList(FastJoinInfo *newNode)
{
	//pthread_mutex_lock(&mutexFastJoinTable);
	FastJoinInfo *currentNode=headFastDevice;
	while(currentNode!=NULL)
	{
		if (strcmp(newNode->address,currentNode->address)==0)
		{
			WARN("[ZIGBEE] FastJoin command receive the same address :%s\n\n",currentNode->address);
			free(newNode);
			//pthread_mutex_unlock(&mutexFastJoinTable);
			return ;
		}
		currentNode=currentNode->next;
	}

	if (headFastDevice == NULL)
	{
		headFastDevice = newNode;
		tailFastDevice = newNode;

	}
	else
	{
		tailFastDevice->next = newNode;
		tailFastDevice = newNode;
	}
	//pthread_mutex_unlock(&mutexFastJoinTable);
}


/********************************************************
函数名：updateFastJoinNodeStatus
描  述：更新快速入网设备信息链表中设备的入网状态
参  数：@char *shortAddress：设备地址后4个字符；
        @int result：是否入网，1已入网，0未入网
返  回：空
********************************************************
void updateFastJoinNodeStatus(char *shortAddress, int result)
{
	FastJoinInfo *pCurrentNode = headFastDevice;
	while (pCurrentNode != NULL)
	{
		if (strncmp(shortAddress, 11 + pCurrentNode->address, 4) == 0)
		{
			pCurrentNode->result = result;
			break;
		}
		else
			pCurrentNode = pCurrentNode->next;
	}
}
*/

/********************************************************
函数名：deleteFastJoinNodeList
描  述：删除整个快速入网设备信息链表
参  数：
返  回：空
*********************************************************/
static void deleteFastJoinNodeList()
{
	//pthread_mutex_lock(&mutexFastJoinTable);
	FastJoinInfo *pCurrentNode = headFastDevice;
	FastJoinInfo *pDelteNode = NULL;
	headFastDevice = NULL;
	tailFastDevice = NULL;

	while (pCurrentNode != NULL)
	{
		pDelteNode = pCurrentNode;
		pCurrentNode = pCurrentNode->next;
		free(pDelteNode);
	}
	//pthread_mutex_unlock(&mutexFastJoinTable);
}

/********************************************************
函数名： parseFastJoinCommand
描  述：解析快速加入指令
参  数：@char *commandStr：快速入网指令字符串
返  回：-1 失败 ，0成功，1全部接收完成；
*********************************************************/
static int parseFastJoinCommand(char *commandStr)
{
	cJSON *root;
	root = cJSON_Parse(commandStr);
	if (root == NULL)
	{
		ERROR("[ZIGBEE] Parse fast join command failed\n\n");
		return -1;
	}

	cJSON *data = cJSON_GetObjectItem(root, "Data");
	if (data==NULL)
	{
		ERROR("[ZIGBEE] Parse fast join command \"Data\" failed\n\n");
		cJSON_Delete(root);
		return -1;
	}
	int arraySize = cJSON_GetArraySize(data);
		

	int i;
	cJSON *item;
	cJSON *totalNum;
	cJSON *endNum;
	cJSON *deviceId;
	for (i = 0; i < arraySize; i++)
	{
		item = cJSON_GetArrayItem(data, i);
		totalNum= cJSON_GetObjectItem(item, "TotalNum");
		endNum = cJSON_GetObjectItem(item, "EndNum");
		deviceId= cJSON_GetObjectItem(item, "DeviceId");
		if (totalNum == NULL || endNum == NULL || deviceId==NULL)
		{
			ERROR("[ZIGBEE] Parse \"totalNum\"  or \"EndNum\" or \"DeviceId\"failed\n\n");
			cJSON_Delete(root);
			return -1;
		}
		if ((atoi(totalNum->valuestring)) < (atoi(endNum->valuestring)))
		{
			ERROR("[ZIGBEE] TotalNum can not be lower than EndNum. Error\n\n");
			cJSON_Delete(root);
			return -1;
		}

		int devNum = cJSON_GetArraySize(deviceId);
		cJSON *itemDevId;
		int j;
		for (j = 0; j < devNum; j++)
		{
			itemDevId= cJSON_GetArrayItem(deviceId, j);
			FastJoinInfo *newNode=createNewFastJoinInfoNode(itemDevId->valuestring);
			if (newNode == NULL)
			{
				WARN("[ZIGBEE] Create fast join info node failed\n\n");
				continue;
			}
			addNewFastJoinNodeToList(newNode);
		}	

	}
	cJSON_Delete(root);

	if (atoi(totalNum->valuestring) == atoi(endNum->valuestring))
		return 1;
	return 0;

}


/********************************************************
函数名： packFastJoinCommandResponse
描  述：打包快速入网指令的响应JSON字符串
参  数：@char *commandStr：快速入网指令字符串；
        @int result：响应结果，1成功，0失败；
		@char *respBuff：响应JSON字符串缓存地址
返  回：-1 失败 ，0成功.
*********************************************************/
static int packFastJoinCommandResponse(char *commandStr, int result,char *respBuff)
{
	cJSON *root;
	root = cJSON_Parse(commandStr);
	if (root == NULL)
		return -1;

	cJSON *command = cJSON_GetObjectItem(root, "Command");
	if (command != NULL)
		sprintf(command->valuestring,"Report");
	else
		return -1;

	cJSON *data = cJSON_GetObjectItem(root, "Data");
	if (data == NULL)
	{
		cJSON_Delete(root);
		return -1;
	}
	int arraySize = cJSON_GetArraySize(data);


	int i;
	cJSON *item;
	for (i = 0; i < arraySize; i++)
	{
		item = cJSON_GetArrayItem(data, i);
		cJSON_DeleteItemFromObject(item, "DeviceId");
		char tmp[2] = {0};
		sprintf(tmp, "%d", result);
		cJSON_AddStringToObject(item, "Result", tmp);
	}

	char *jsonStr;
	jsonStr = cJSON_Print(root);
	sprintf(respBuff, "%s", jsonStr);

	cJSON_Delete(root);
	if (!jsonStr)
		free(jsonStr);
	return 0;
}









/********************************************************
函数名： closeFastJoinFunction
描  述：关闭快速入网功能；
参  数：
返  回：0成功，-1失败
*********************************************************/
/*
static int closeFastJoinFunction()
{
	int res;
	res=sendCommandCloseFastJoin();
	//sleep(1);
	//sendCommandClearGatewayNetInfo();
	//sleep(1);
	//sendCommandBuildNetwork();
	//sleep(1);
	//res=sendCommandGetZigbeeNetworkInfo();

	return res;
}

*/



static void packageFastJoinCloseReport(char *outBuff)
{
	cJSON *root;
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "FastJoinResult");
	cJSON *list = cJSON_CreateArray();
	cJSON_AddItemToObject(root, "Data", list);

	char *tmp = NULL;
	tmp = cJSON_Print(root);
	sprintf(outBuff, "%s", tmp);
	cJSON_Delete(root);
	if (!tmp)
		free(tmp);
}

/********************************************************
函数名： runFastJoinGateway
描  述：启动快速入网下发功能
参  数：@char *command：下发地址数组JSON
返  回：0成功，-1失败
*********************************************************/
static void runFastJoinGateway()
{
	int res=0;
	sendCommandSetJoinNetWorkMode(2);
	//pthread_mutex_lock(&mutexFastJoinTable);
	FastJoinInfo *pCurrentNode = headFastDevice;
	while (1)
	{
		sendFastJoinCommand=0;
		int count = 0;
		cJSON *root;
		root = cJSON_CreateObject();
		cJSON *addresses;
		cJSON_AddItemToObject(root,"Addresses", addresses= cJSON_CreateArray());
		char shortAddress[10] = { 0 };
		while (pCurrentNode!=NULL)
		{
			
			if (pCurrentNode->result == 0 && pCurrentNode->reportCount < 2)
			{
				sprintf(shortAddress, "%s", 10 + pCurrentNode->address);
				cJSON_AddItemToArray(addresses, cJSON_CreateString(shortAddress));
				count++;
				if (count == 20)
					break;
			}
			pCurrentNode = pCurrentNode->next;
		}

		if (count == 0)/*关闭快速入网*/
		{
			sendCommandCloseFastJoin();
			res=1;
			break;
		}
		else/*下发快速入网*/
		{
			char *tmp = cJSON_Print(root);
			sendCommandOpenFastJoin(tmp);
			free(tmp);
		}

		cJSON_Delete(root);

		pCurrentNode = headFastDevice;
		while (sendFastJoinCommand==0)
		{
			sleep(5);
		}
	}
	//pthread_mutex_unlock(&mutexFastJoinTable);

	if (res==1) // 全部发送完毕
		deleteFastJoinNodeList();

	/*上报快速入网结束*/
	char outBuff[300];
	packageFastJoinCloseReport(outBuff);
	messageReportFun(outBuff);

}


/********************************************************
函数名： threadExecuteFastJoin
描  述：执行快速入网线程
参  数：
返  回：0成功，-1失败
*********************************************************/
int threadExecuteFastJoin()
{
	int rc;
	pthread_t pid;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	rc = pthread_create(&pid, &attr, (void *)runFastJoinGateway, NULL);
	pthread_attr_destroy(&attr);
	return rc;
}

/********************************************************
函数名： handlFastJoinCommand
描  述：快速入网指令处理函数
参  数：@char *commandStr：快速入网指令字符串；

返  回：-1 失败 ，0成功.
*********************************************************/
extern int handleFastJoinCommand(char *commandStr)
{
	int res=-1;
	res=parseFastJoinCommand(commandStr);

	char respBuff[1024] = {0};
	if (res>=0)
		packFastJoinCommandResponse(commandStr, 0, respBuff);
	messageReportFun(respBuff);

	if (res >= 1)
	{
		threadExecuteFastJoin();
		res = 0;
	}

	return res;
}




/********************************************************
函数名： matchDeviceId
描  述：匹配设备Id,设置入网标志位
参  数：@char *shortAddress：设备地址的后4位字符；
        @int result：设置结果，1成功，0失败；
		@char *fullAddress:输出完整地址；
返  回：-1未匹配上，3已上报3次，0正常
*********************************************************/
static int matchDeviceId(char *shortAddress,int result,char *fullAddress)
{
	int res = -1;
	FastJoinInfo *pCurrentNode = headFastDevice;
	while (pCurrentNode != NULL)
	{
		if (strcmp(10 + pCurrentNode->address, shortAddress) == 0)
		{
			pCurrentNode->result = result;
			pCurrentNode->reportCount++;
			res = 0;
			/*

			if (pCurrentNode->reportCount == 3)
			{
				res = 3;
				pCurrentNode->reportCount = 0;
				pCurrentNode->ckeckCount++;
			}
			*/
			sprintf(fullAddress, "%s", pCurrentNode->address);
				
			break;
		}
		else
			pCurrentNode = pCurrentNode->next;
	}
	return res;
}

/********************************************************
函数名： packageFastJoinResultReport
描  述：打包快速入网结果上报字符串
参  数：@cJSON *result：结果JSON结构体指针；
        @char *outBuff：输出上报字符串缓存指针；
返  回：空
*********************************************************/
static void packageFastJoinResultReport(cJSON *result,char *outBuff)
{
	char *tmpRes=cJSON_Print(result);
	cJSON *rest=cJSON_Parse(tmpRes);
	cJSON *root;
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "FastJoinResult");
	cJSON_AddItemToObject(root, "Data", rest);

	char *tmp = NULL;
	tmp = cJSON_PrintUnformatted(root);
	sprintf(outBuff, "%s", tmp);
	cJSON_Delete(root);
	if (!tmp)
		free(tmp);
}


//static int resultCount = 0;/*结果上报次数*/
/********************************************************
函数名： dealFastJoinResultReport
描  述：解析处理快速入网结果上报指令
参  数：@char *reportState：快速入网指令字符串；
返  回：-1 失败 ，0成功.
*********************************************************/
extern int dealFastJoinResultReport(char *reportState)
{
	cJSON *root;
	root = cJSON_Parse(reportState);
	if (root == NULL)
		return -1;

	cJSON *result = cJSON_GetObjectItem(root, "Results");

	int arraySize = cJSON_GetArraySize(result);
	cJSON *item;
	cJSON *address;
	cJSON *res;
	int i = 0;
	//int resp = 1;
	//int resultCount=3;

	cJSON *reportData = cJSON_CreateArray();
	cJSON *js_list;
	for (i = 0; i < arraySize; i++)
	{
		item = cJSON_GetArrayItem(result, i);
		address = cJSON_GetObjectItem(item, "Address");
		res = cJSON_GetObjectItem(item, "Result");
		//if (0 == atoi(res->valuestring))
		//	resp = 0;

		/*匹配白名单设备Id*/
		char tmp[18] = { 0 };
		//int rt;
		matchDeviceId(address->valuestring, atoi(res->valuestring), tmp);
		//if (rt == -1)
		//	continue;
			
		cJSON_AddItemToArray(reportData, js_list = cJSON_CreateObject());
		cJSON_AddStringToObject(js_list, "DeviceId", tmp);
		cJSON_AddStringToObject(js_list, "Result", res->valuestring);
	}

	/*此处插入上报结果*/
	char respJsonString[1024];
	packageFastJoinResultReport(reportData, respJsonString);
	messageReportFun(respJsonString);


	/*当本次全部回复结果都为1时，标志位sendFastJoinCommand置为1，允许下发搜索新的20个地址*/
	//printf(">>>>>>>>>>>>>>resp=%d,resultCount=%d\n\n",resp,resultCount);
	//if (resp == 1 || resultCount == 3)
	//{
		sendFastJoinCommand = 1;
	//	resultCount = 0;
	//}
	cJSON_Delete(reportData);
	//printf(">>>>>>>>>>>>>>1324\n\n");
	cJSON_Delete(root);
	//printf(">>>>>>>>>>>>>>111\n\n");
	
	return 0;
}

/*
int deleteFastJoinNodeFromList(char *address)
{
	FastJoinInfo *pCurrentNode = headFastDevice;
	FastJoinInfo *pDelteNode = NULL;
	FastJoinInfo *pPreviousNode = headFastDevice;

	while (pCurrentNode != NULL)
	{
		if (strncmp(address, pCurrentNode->address, 4) == 0)
		{
			if (pCurrentNode == headFastDevice && pCurrentNode == tailFastDevice)
			{
				pDelteNode = pCurrentNode;
				headFastDevice = NULL;
				tailFastDevice = NULL;
			}
			else if (pCurrentNode == headFastDevice && pCurrentNode != tailFastDevice)
			{
				pDelteNode = pCurrentNode;
				headFastDevice = pCurrentNode->next;
			}
			else if (pCurrentNode != headFastDevice && pCurrentNode == tailFastDevice)
			{
				pDelteNode = pCurrentNode;
				pPreviousNode->next = NULL;
				tailFastDevice = pPreviousNode;
			}
			else
			{
				pDelteNode = pCurrentNode;
				pPreviousNode->next = pCurrentNode->next;
			}
			//pCurrentNode=pCurrentNode->next;
			free(pDelteNode);
			return 0;
		}
		else
			pCurrentNode = pCurrentNode->next;
	}
	return -1;
}*/
