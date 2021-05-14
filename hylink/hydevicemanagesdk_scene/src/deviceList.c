
#include "sqlite3.h"
#include "cJSON.h"
#include "rex_export_gateway.h"
#include "rex_export_type.h"
#include "common.h"
#include "deviceConfigure.h"
#include "led.h"
#include "rexCommand.h"
#include "callback.h"
#include "hyprotocol.h"
#include "group.h"
#include "bindAirConditioning.h"
#include "udpServer.h"
#include "log.h"
#include "fastJoin.h"
#include "signature.h"
#include "specialDeviceJudge.h"
#include "misc.h"
#include "custom_attribute.h"
#include "log.h"
#include "full_scene.h"
#include "usr_sleep.h"
#include "shell_api.h"
#include "ty_device.h"
#include "ctrlSmartLock.h"

#ifdef MULTI_SWITCH_SYNC
#include "syncSwitch.h"
#endif

#ifdef CTRL_LOCK
#include "ctrlSmartLock.h"
#endif

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <sys/socket.h>  
#include <net/if.h>
#include <sys/ioctl.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/reboot.h>
#include <math.h>
#include <dirent.h>
#include <sys/stat.h>
#include <ctype.h>


#ifdef BOARD_WHITE_BOX_GATEWAY	
	#define  SERIAL_NAME_STRING  "/dev/ttyS0"
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	#define  SERIAL_NAME_STRING  "/dev/ttyS1"
#endif
#ifdef BOARD_U86_GATEWAY
	#define  SERIAL_NAME_STRING  "/dev/ttyS1"
#endif

#define  SERIAL_BAUD_RATE    (115200)
#define SQLITE_COMMAND_MAX_LENGTH (300)

#define OUTPUT_BUFF_MAX_LENGTH	(2048)

#define REPORT_DEV_LIST_LENGTH  (10000)
//#define REPORT_DEV_LIST_INFO_LENGTH

/***************************************静态变量-文件路径***********************************/
static char dataBasePath[DATA_BASE_PATH_MAX_LENGTH]; //数据库文件路径变量
char configurFilePath[DEV_CONFIGUR_FILE_PATH_MAX_LENGTH]; //设备配置文件路径变量
static char fileDownloadPath[FILE_DOWN_LOAD_PATH_MAX_LENGTH]; //COO文件下载和升级路径
static char otaConfigureFilePath[OTA_CONFIGURE_FILE_PATH_MAX_LENGTH]; //ota升级配置文件，该文件包括COO的信息

char cooFirmwarePath[100]={0};
char tmp_devFilePath[100]={0};/*子设备升级文件绝对路径*/

/********************************静态变量-COO信息**************************************/
static char cooChannel[4];	// COO通道
static char cooPanId[6];	//COO Pan id
static char cooExtPanId[20];
static char cooMac[20];		//COO mac地址
static char cooNetWorkKey[38];

static char cooVersion[100]={0};
static char cooCode[20]={0};

static char rexSdkVer[10]={0};

/********************************设备链表信息**************************************/
DevInfo *pHead = NULL;/*设备链表头指针*/
static DevInfo *pTail = NULL;/*设备链表尾指针*/

static DevOnlineInfo *pOnlineHead = NULL;/*设备在线链表头指针*/
static DevOnlineInfo *pOnlineTail = NULL;/*设备在线链表头指针*/

static sqlite3 *pDB = NULL;/*数据库指针*/
static char *zErrMsg = NULL;

static pthread_mutex_t mutexLinkTable;/*设备链表操作的线程锁*/
static pthread_mutex_t mutexOnlineTable;/*设备在线链表线程锁*/

int devcieNumber = 0;

int cooUpdateEnd = 0;

/**********************************************/
int repeatReport=0; /*是否允许重复上报*/


/*************************设置路径****************************/
int setDataBasePath(const char *path)
{
	int res = -1;
	int len;
	len = strlen(path);
	if (len >= DATA_BASE_PATH_MAX_LENGTH)
	{
		ERROR("[ZIGBEE] device mamange database path length >= %d characters\n\n", DATA_BASE_PATH_MAX_LENGTH);
		res = 0;
	}
	else
	{
		int num=strlen(path) ;
		if ('/'==path[num-1])
			sprintf(dataBasePath, "%s%s", path,"devInfo.db");
		else
			sprintf(dataBasePath, "%s/%s", path,"devInfo.db");
		res = 0;
	}
	return res;
}

int setConfigurFilePath(const char *path)
{
	int res = -1;
	int len;
	len = strlen(path);
	if (len >= DEV_CONFIGUR_FILE_PATH_MAX_LENGTH)
	{
		ERROR("[ZIGBEE] device configure files path length >= %d characters\n\n", DEV_CONFIGUR_FILE_PATH_MAX_LENGTH);
		res = 0;
	}
	else
	{
		sprintf(configurFilePath, "%s", path);
		res = 0;
	}
	return res;
}

int setOtaConfigureFilePath(const char *path)
{
	int res = -1;
	int len;
	len = strlen(path);
	if (len >= OTA_CONFIGURE_FILE_PATH_MAX_LENGTH)
	{
		ERROR("[ZIGBEE] ota configure file path length >= %d characters\n\n", OTA_CONFIGURE_FILE_PATH_MAX_LENGTH);
		res = 0;
	}
	else
	{
		sprintf(otaConfigureFilePath, "%s%s", path,"ota_configure");
		res = 0;
	}
	return res;
}


int setFileDownloadPath(const char *path)
{
	int res = -1;
	int len;
	len = strlen(path);
	if (len >= FILE_DOWN_LOAD_PATH_MAX_LENGTH)
	{
		ERROR("[ZIGBEE] Coo firmware file path length >= %d characters\n\n", FILE_DOWN_LOAD_PATH_MAX_LENGTH);
		res = 0;
	}
	else
	{
		int num=strlen(path) ;
		if ('/'==path[num-1])
			sprintf(fileDownloadPath, "%s", path);
		else
			sprintf(fileDownloadPath, "%s/", path);

		setOtaConfigureFilePath(fileDownloadPath);
		res = 0;
	}
	return res;
}




int getSubDevEndpointTotalNum(int devType)
{
	int endPointNum = 1;
	switch (devType)
	{
	case 1002:
	case 1102:
	case 1106:
	case 1308:
		endPointNum = 2;
		break;
	case 1003:
	case 1103:
	case 1107:
	case 1309:
		endPointNum = 3;
		break;
	case 1004:
	case 1104:
	case 1108:
	case 1310:
		endPointNum = 4;
		break;
	case 1311:
		endPointNum = 5;
		break;
	case 1312:
		endPointNum = 6;
		break;
	default:
		endPointNum = 1;
		break;
	}
	return endPointNum;
}

int setSubDevRegisterFlag(DevInfo *pCurrentNode, int flag)
{
	pCurrentNode->registerStatus = flag;

	char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
	sprintf(sqlStr, "UPDATE Table_device_list SET registerStatus=%d WHERE deviceId=\'%s\';", flag, pCurrentNode->deviceId);

	int res;
	res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] update device %s register flag %d failed: %s\n", pCurrentNode->deviceId, flag, zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	DEBUG("[ZIGBEE] update device %s register flag %d success!\n\n", pCurrentNode->deviceId, flag);

	return 0;
}


int setDevRegisterFlag(char *devId, int registerStatus)
{
	DevInfo *currentNode;

	pthread_mutex_lock(&mutexLinkTable);

	currentNode = pHead;
	int res = -1;
	while (currentNode != NULL)
	{
		if (devId==NULL)
		{
			res=setSubDevRegisterFlag(currentNode, registerStatus);
		}
		else if(strncmp(devId, currentNode->deviceId, 16) == 0)
		{

			res=setSubDevRegisterFlag(currentNode, registerStatus);
			pthread_mutex_unlock(&mutexLinkTable);
			return res;
		}

		currentNode = currentNode->next;
	}

	pthread_mutex_unlock(&mutexLinkTable);

	return res;
}



void freeDevInfoNode(DevInfo *pNode)
{
	if (pNode->onlineInfo != NULL)
	{
		pNode->onlineInfo->leaveGateway = 0;
		pNode->onlineInfo = NULL;
	}

	Attribute *currentNode = pNode->attrHead;
	pNode->attrHead = NULL;
	Attribute *deleteNode;

	while (currentNode != NULL)
	{
		deleteNode = currentNode;
		currentNode = currentNode->next;
		free(deleteNode);
	}
	free(pNode);
}


void reportAirChannelNum(char *devType, char *address,char *modelId)
{
	int typeNUm;
	typeNUm = atoi(devType);
	if ((typeNUm > 1306 && typeNUm < 1313) || (typeNUm > 1329 && typeNUm < 1340))
	{
		char outBuff[400];
		ReportAttrEventOnline reportAirChange;
		sprintf(reportAirChange.deviceId, "%s", address);
		sprintf(reportAirChange.modelId, "%s", modelId);
		sprintf(reportAirChange.key, "%s", "ChannelCount");
		reportAirChange.next = NULL;
		switch (typeNUm)
		{
		case 1307:
			sprintf(reportAirChange.value, "%s", "1");
			break;
		case 1308:
			sprintf(reportAirChange.value, "%s", "2");
			break;
		case 1309:
			sprintf(reportAirChange.value, "%s", "3");
			break;
		case 1310:
			sprintf(reportAirChange.value, "%s", "4");
			break;
		case 1311:
			sprintf(reportAirChange.value, "%s", "5");
			break;
		case 1312:
			sprintf(reportAirChange.value, "%s", "6");
			break;
		case 1330:
			sprintf(reportAirChange.value, "%s", "7");
			break;
		case 1331:
			sprintf(reportAirChange.value, "%s", "8");
			break;
		case 1332:
			sprintf(reportAirChange.value, "%s", "9");
			break;
		case 1333:
			sprintf(reportAirChange.value, "%s", "10");
			break;
		case 1334:
			sprintf(reportAirChange.value, "%s", "11");
			break;
		case 1335:
			sprintf(reportAirChange.value, "%s", "12");
			break;
		case 1336:
			sprintf(reportAirChange.value, "%s", "13");
			break;
		case 1337:
			sprintf(reportAirChange.value, "%s", "14");
			break;
		case 1338:
			sprintf(reportAirChange.value, "%s", "15");
			break;
		case 1339:
			sprintf(reportAirChange.value, "%s", "16");
			break;
		default:
			break;
		}
		packageReportMessageToJson(REPORT_TYPE_ATTRIBUTE, (void*)&reportAirChange, outBuff);
		messageReportFun(outBuff);
	}
}

int c2i(char ch)
{
	if (isdigit(ch))
		return ch - 48;
	if (ch < 'A' || (ch > 'F' && ch < 'a') || ch > 'z')
		return -1;
	if (isalpha(ch))
		return isupper(ch) ? ch - 55 : ch - 87;
	return -1;
}

int hex2dec(char *hex)
{
	int len;
	int num = 0;
	int temp;
	int bits;
	int i;
	len = strlen(hex);

	for (i = 0, temp = 0; i < len; i++, temp = 0)
	{
		temp = c2i(*(hex + i));
		bits = (len - i - 1) * 4;
		temp = temp << bits;
		num = num | temp;
	}
	return num;
}

static struct timeval tm;
static char lastJoinDevice[18]={0};



void num2hex(unsigned int num,char *hexbufff)
{
	int i;
	unsigned int tmp;

	unsigned int mask = 0xf;            //掩码0x 0000 000f

	//char hexbufff[9] = {0};  //存储十六进制字符串

	//为其余元素赋值
	for (i = 5; i >= 0; i--)
	{
		tmp = num & mask;
		hexbufff[i] = (tmp >= 10) ? ((tmp - 10) + 'a') : (tmp + '0');
		num = num >> 4;
	}
}



void unpackEvent0307(char *event,char *outModelID,char *reportTime)
{
	cJSON *root;
	root = cJSON_Parse(event);

	cJSON *item = cJSON_GetObjectItem(root, "AuthenticationInfo");
	cJSON *modelId= cJSON_GetObjectItem(item, "ModelID");
	num2hex(atoi(modelId->valuestring), outModelID);
	cJSON *time=cJSON_GetObjectItem(item, "Supervision");
	sprintf(reportTime,"%s",time->valuestring);
	cJSON_Delete(root);
}


void ctrlDevReturnStates(void *deviceId)
{
	char devId[18]={0};
	char *tmp=(char *)deviceId;
	sprintf(devId,"%s",tmp);
	sleep(3);
	sendCommandGetDeviceState(devId, 255);
}

int deviceEventJoin(char *address, char *event)
{
	struct timeval now;
	gettimeofday(&now,NULL);
	if (strcmp(lastJoinDevice,address)==0 && (now.tv_sec-tm.tv_sec)<5)
	{
		sprintf(lastJoinDevice,"%s",address);
		gettimeofday(&tm,NULL);
		return 0;
	}


	char modelId[DEVICE_MODEL_ID_LENGTH] = { 0 };
	char reportDuration[6]={0};
	unpackEvent0307(event, modelId,reportDuration);
	INFO("[ZIGBEE] a device modelId %s request join gateway\n\n", modelId);
	if (strncmp(modelId, "ffffff",6) == 0)
		return 0;

	int find = 0;
	DevInfo *currentNode;
	pthread_mutex_lock(&mutexLinkTable);
	currentNode = pHead;
	while (currentNode != NULL)
	{
		if (strncmp(address, currentNode->deviceId, 16) == 0)
		{
			find = 1;
			break;
		}
		else
		{
			currentNode = currentNode->next;
		}
	}
	pthread_mutex_unlock(&mutexLinkTable);


	if (find == 0)
	{
		DevInfo *newNode;
		newNode = ZHA_creatDevInfoStruct(modelId);
		if (newNode == NULL)
			return -1;

		sprintf(newNode->realModelId,"%s",modelId);
		sprintf(lastJoinDevice,"%s",address);
		gettimeofday(&tm,NULL);

		sprintf(newNode->deviceId, "%s", address);
		sprintf(newNode->onlineInfo->deviceId, "%s", address);
		//newNode->subDevType = hex2dec(type);
		newNode->onlineInfo->online = 1;
		//newNode->onlineInfo->reportTime=atoi(reportDuration);
		sprintf(newNode->softwareVersion, "20180101");

		pthread_mutex_lock(&mutexLinkTable);
		devcieNumber++;
		if (pHead == NULL)
		{
			pHead = newNode;
			pTail = newNode;
		}
		else
		{
			pTail->next = newNode;
			pTail = newNode;
		}
		pthread_mutex_unlock(&mutexLinkTable);

		pthread_mutex_lock(&mutexOnlineTable);
		if (pOnlineHead == NULL && pOnlineTail == NULL)
		{
			pOnlineHead = newNode->onlineInfo;
			pOnlineTail = newNode->onlineInfo;
		}
		else
		{
			pOnlineTail->next = newNode->onlineInfo;
			pOnlineTail = newNode->onlineInfo;
		}
		pthread_mutex_unlock(&mutexOnlineTable);

		ReportRegister reportReg;
		memset(&reportReg,0,sizeof(ReportRegister));
		sprintf(reportReg.deviceId, "%s", address);
		sprintf(reportReg.modelId, newNode->modelId);
		sprintf(reportReg.secret, "%s", newNode->secret);
		sprintf(reportReg.realModelId, "%s", newNode->realModelId);
		//sprintf(reportReg.subDevType, "%d", newNode->subDevType);//hex2dec(type);
		reportReg.next = NULL;

		char outBuff[400];
		packageReportMessageToJson(REPORT_TYPE_REGISTER, (void*)&reportReg, outBuff);

		int rt = 0;
		/*上报设备入网信息*/
		rt = messageReportFun(outBuff);
		int regFlag = 1;
		if (rt == -1) //注册失败删除
		{
			newNode->registerStatus = 0;
			WARN("[ZIGBEE] New device register failed,deviceId:%s, modelId:%s, type:%d\n\n", address, newNode->modelId, newNode->subDevType);
			regFlag = 0;
		}
		else//注册成功
		{
			newNode->registerStatus = 1;
			INFO("[ZIGBEE] New device register success,deviceId:%s, modelId:%s, type:%d,\n\n", address, newNode->modelId, newNode->subDevType);
		}

		int res = 0;
		char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
		sprintf(sqlStr, "REPLACE INTO Table_device_list (deviceId,subType,name,modelId,hardwareVersion,\
		softwareVersion,registerStatus,secret,reportTime) VALUES (\'%s\',%d,\'%s\',\'%s\',\'%s\',\'%s\',%d,\'%s\',\'%d\');", \
			address, newNode->subDevType, newNode->name, modelId, "1.0.0", "20180101", regFlag, newNode->secret,newNode->onlineInfo->reportTime);

		res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
		if (res != SQLITE_OK)
		{
			ERROR("[ZIGBEE] inserter new device %s information into database failed: %s\n\n", address, zErrMsg);
			sqlite3_free(zErrMsg);
			freeDevInfoNode(newNode);
			return -1;
		}
		/*
		pthread_attr_t attr;
		pthread_attr_init (&attr);
		pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
		pthread_t pCallDevResponse;
		res = pthread_create(&pCallDevResponse,  &attr, (void *)ctrlDevReturnStates, (void *)newNode->deviceId);
		pthread_attr_destroy (&attr);
		 */
		//char outBuff[400];
		/*
		ReportAttrEventOnline online;
		sprintf(online.deviceId, "%s", newNode->deviceId);
		sprintf(online.modelId, "%s", newNode->modelId);
		sprintf(online.key, "%s", "Online");
		sprintf(online.value, "%s", "1");
		online.next = NULL;
		packageReportMessageToJson(REPORT_TYPE_ONOFF, (void*)&online, outBuff);
		messageReportFun(outBuff);
		*/

	}
	else
	{
		sprintf(lastJoinDevice,"%s",address);
		gettimeofday(&tm,NULL);
		ReportRegister reportReg;
		memset(&reportReg,0,sizeof(ReportRegister));
		sprintf(reportReg.deviceId, "%s", address);
		sprintf(reportReg.modelId, currentNode->modelId);
		sprintf(reportReg.secret, "%s", currentNode->secret);

		//sprintf(reportReg.subDevType, "%d", currentNode->subDevType);//hex2dec(type);
		reportReg.next = NULL;

		char outBuff[400];
		packageReportMessageToJson(REPORT_TYPE_REGISTER, (void*)&reportReg, outBuff);

		int rt = 0;
		rt = messageReportFun(outBuff);
		//printf("[Debug] device %s added to the gateway,repeatedly\n\n", address);
		if (0 == rt)
			setSubDevRegisterFlag(currentNode,1); //注册成功，则置标志位=1
		else
			setSubDevRegisterFlag(currentNode,0); //注册不成功，则置标志位=0

		currentNode->onlineInfo->online = 1;
		currentNode->onlineInfo->offCount = 0;
		currentNode->onlineInfo->leaveGateway = 1;
		gettimeofday(&currentNode->onlineInfo->lastUpdateTime, NULL);

		//char outBuff[400];
		ReportAttrEventOnline online;
		sprintf(online.deviceId, "%s", currentNode->deviceId);
		sprintf(online.modelId, "%s", currentNode->modelId);
		sprintf(online.key, "%s", "Online");
		sprintf(online.value, "%s", "1");
		online.next = NULL;
		packageReportMessageToJson(REPORT_TYPE_ONOFF, (void*)&online, outBuff);
		/*此处插入上报函数*/
		messageReportFun(outBuff);
	}
	return 0;
}

DevInfo* getDevModelId(char *devId, char *outbuff)
{
	DevInfo* currentNode;
	currentNode = pHead;
	//int res = -1;
	while (currentNode != NULL)
	{
		if (strncmp(devId, currentNode->deviceId, 16) == 0)
		{
			sprintf(outbuff, "%s", currentNode->modelId);
			//res = 0;
			break;
		}
		else
			currentNode = currentNode->next;
	}
	return currentNode;
}

int getDevRexType(char *devId)
{
	DevInfo* currentNode;
	currentNode = pHead;
	while (currentNode != NULL)
	{
		if (strncmp(devId, currentNode->deviceId, 16) == 0)
		{
			return currentNode->subDevType;
		}
		else
			currentNode = currentNode->next;
	}
	return -1;
}


int checkAirConditionerChannel(int subType,Attribute *attriHead)
{
	int res=0;
	Attribute *curentNode;
	curentNode=attriHead;

	while(curentNode!=NULL)
	{
		if(strcmp(curentNode->key,"ChannelCount")==0)
			break;
		else
			curentNode=curentNode->next;
	}
	if (curentNode==NULL)
		return -1;
	switch(subType)
	{
	case 0x1307:
		sprintf(curentNode->value,"%d",1);
		break;
	case 0x1308:
		sprintf(curentNode->value,"%d",2);
		break;
	case 0x1309:
		sprintf(curentNode->value,"%d",3);
		break;
	case 0x1310:
		sprintf(curentNode->value,"%d",4);
		break;
	case 0x1311:
		sprintf(curentNode->value,"%d",5);
		break;
	case 0x1312:
		sprintf(curentNode->value,"%d",6);
		break;
	case 0x1330:
		sprintf(curentNode->value,"%d",7);
		break;
	case 0x1331:
		sprintf(curentNode->value,"%d",8);
		break;
	case 0x1332:
		sprintf(curentNode->value,"%d",9);
		break;
	case 0x1333:
		sprintf(curentNode->value,"%d",10);
		break;
	case 0x1334:
		sprintf(curentNode->value,"%d",11);
		break;
	case 0x1335:
		sprintf(curentNode->value,"%d",12);
		break;
	case 0x1336:
		sprintf(curentNode->value,"%d",13);
		break;
	case 0x1337:
		sprintf(curentNode->value,"%d",14);
		break;
	case 0x1338:
		sprintf(curentNode->value,"%d",15);
		break;
	case 0x1339:
		sprintf(curentNode->value,"%d",16);
		break;
	default:
		res=-1;
		break;
	}
	return res;
}


/*必须返回0,否则瑞瀛SDK不会进一步读取设备的相关信息*/
int device_join_cb(char *address, unsigned short reporting, char *type, char *modelId)
{
	INFO("[ZIGBEE]\033[40;33m[REX_REPORT]\033[0m A new device=%s, type=%s, reportDuration=%d modelID=%s request join gateway \n\n", \
			address, type, reporting,modelId);
	struct timeval now;

	pthread_mutex_lock(&mutexLinkTable);
	gettimeofday(&now,NULL);

	if (strcmp(lastJoinDevice,address)==0 && (now.tv_sec-tm.tv_sec)<5)
	{

		char tmpModelId[10]={0};
		DevInfo* currentNode;
		currentNode=getDevModelId(address, tmpModelId);
		if (currentNode!=NULL)
		{
			int res=0;
			res=checkAirConditionerChannel(hex2dec(type),currentNode->attrHead);
			if (res==0)
			{
				char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
				sprintf(sqlStr, "UPDATE Table_device_list SET subType=%d WHERE deviceId=\'%s\';",hex2dec(type),address);
				res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
				if (res != SQLITE_OK)
				{
					ERROR("[ZIGBEE] inserter new device %s information into database failed: %s\n\n", address, zErrMsg);
					sqlite3_free(zErrMsg);
				}
				reportAirChannelNum(type, address, tmpModelId);
			}
		}
		sprintf(lastJoinDevice,"%s",address);
		gettimeofday(&tm,NULL);
		pthread_mutex_unlock(&mutexLinkTable);
		return 0;
	}
	sprintf(lastJoinDevice,"%s",address);
	gettimeofday(&tm,NULL);

	int find = 0;
	DevInfo *currentNode;

	currentNode = pHead;
	while (currentNode != NULL)
	{
		if (strncmp(address, currentNode->deviceId, 16) == 0)
		{
			find = 1;
			break;

		}
		else
		{
			currentNode = currentNode->next;
		}
	}
	
	if (find == 0)
	{
		DevInfo *newNode=NULL;
		char tmp[DEVICE_MODEL_ID_LENGTH]={0};
		if(strlen(modelId)==8 && modelId[0]=='0'&& modelId[1]=='0')
			sprintf(tmp,"%s",modelId+2);
		else if (strlen(modelId)==7 && modelId[0]=='0')
			sprintf(tmp,"%s",modelId+1);
		else
			sprintf(tmp,"%s",modelId);

		if (1==isTYDeviceFirmware(tmp))
			newNode=NULL;
		else
			newNode = ZHA_creatDevInfoStruct(tmp);

		if (newNode == NULL)
		{
			char buff[300];
			packageUnknowDeviceInfo(address, tmp,type,buff);
			messageReportFun(buff);
			pthread_mutex_unlock(&mutexLinkTable);
			return 0;
		}

		sprintf(newNode->realModelId,"%s",tmp);
		sprintf(newNode->deviceId, "%s", address);
		sprintf(newNode->onlineInfo->deviceId, "%s", address);
		checkAirConditionerChannel(hex2dec(type),newNode->attrHead);
		newNode->onlineInfo->online = 1;
		sprintf(newNode->softwareVersion,"20180101");
		devcieNumber++;
		if (pHead == NULL)
		{
			pHead = newNode;
			pTail = newNode;
		}
		else
		{
			pTail->next = newNode;
			pTail = newNode;
		}
		pthread_mutex_unlock(&mutexLinkTable);

		pthread_mutex_lock(&mutexOnlineTable);
		if (pOnlineHead == NULL && pOnlineTail == NULL)
		{
			pOnlineHead = newNode->onlineInfo;
			pOnlineTail = newNode->onlineInfo;
		}
		else
		{
			pOnlineTail->next = newNode->onlineInfo;
			pOnlineTail = newNode->onlineInfo;
		}
		pthread_mutex_unlock(&mutexOnlineTable);



		ReportRegister reportReg;
		memset(&reportReg,0,sizeof(ReportRegister));
		sprintf(reportReg.deviceId, "%s", address);
		sprintf(reportReg.modelId, newNode->modelId);
		sprintf(reportReg.secret, "%s", newNode->secret);
		sprintf(reportReg.realModelId, "%s", newNode->realModelId);
		sprintf(reportReg.subDevType, "%d", newNode->subDevType);//hex2dec(type);
		reportReg.next = NULL;

		char outBuff[400];
		packageReportMessageToJson(REPORT_TYPE_REGISTER, (void*)&reportReg, outBuff);

		int rt = 0;
		/*上报设备入网信息*/
		rt = messageReportFun(outBuff);

		/*如果是多路空调协议装换器则上报空调路数*/
		reportAirChannelNum(type, address, newNode->modelId);

		int regFlag = 1;
		if (rt == -1) //注册失败删除
		{
			newNode->registerStatus = 0;
			WARN("[ZIGBEE] New device register failed,deviceId:%s, modelId:%s, type:%s\n\n", address, newNode->modelId, type);
			regFlag = 0;
		}
		else//注册成功
		{
			newNode->registerStatus = 1;
			INFO("[ZIGBEE] New device register success,deviceId:%s, modelId:%s, type:%s,\n\n", address,newNode->modelId,type);
		}
		int res = 0;
		char sqlStr[SQLITE_COMMAND_MAX_LENGTH];



		sprintf(sqlStr, "REPLACE INTO Table_device_list (deviceId,subType,name,modelId,hardwareVersion,\
		softwareVersion,registerStatus,secret,reportTime) VALUES (\'%s\',%d,\'%s\',\'%s\',\'%s\',\'%s\',%d,\'%s\',\'%d\');", \
		address, newNode->subDevType, newNode->name, tmp, "1.0.0", "20180101", regFlag, newNode->secret,newNode->onlineInfo->reportTime);


		res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
		if (res != SQLITE_OK)
		{
			ERROR("[ZIGBEE] inserter new device %s information into database failed: %s\n\n", address, zErrMsg);
			sqlite3_free(zErrMsg);
			freeDevInfoNode(newNode);
			return 0;
		}

	}
	else if (currentNode->registerStatus==0)
	{
		pthread_mutex_unlock(&mutexLinkTable);
		ReportRegister reportReg;
		memset(&reportReg,0,sizeof(ReportRegister));
		sprintf(reportReg.deviceId, "%s", address);
		sprintf(reportReg.modelId, currentNode->modelId);
		sprintf(reportReg.secret, "%s", currentNode->secret);
		reportReg.next = NULL;

		char outBuff[400];
		packageReportMessageToJson(REPORT_TYPE_REGISTER, (void*)&reportReg, outBuff);

		int rt = 0;
		rt = messageReportFun(outBuff);
		INFO("[ZIGBEE] device %s added to the gateway,repeatedly\n\n", address);
		if (0 == rt)
			currentNode->registerStatus = 1; //注册成功，则置标志位=1
		else
			currentNode->registerStatus = 0; //注册不成功，则置标志位=0
		currentNode->onlineInfo->online = 1;
		currentNode->onlineInfo->offCount = 0;
		currentNode->onlineInfo->leaveGateway = 1;
		gettimeofday(&currentNode->onlineInfo->lastUpdateTime, NULL);

	}
	else
		pthread_mutex_unlock(&mutexLinkTable);

	
	sendCommandGetDevVersion(address);

	return 0;
}


int device_leave_cb(char *address)
{
	INFO("[ZIGBEE]\033[40;33m[REX_REPORT]\033[0m Leave a device, deviceId = %s\n", address);
	int res = 0;

	DevInfo *pCurrentNode;
	DevInfo *pPreviousNode;
	DevInfo *pDelteNode;

	pthread_mutex_lock(&mutexLinkTable);
	pCurrentNode = pHead;
	pPreviousNode = pHead;

	if (pCurrentNode == NULL)
	{
		//printf("[Debug] device_leave_cb: Device information List is empty: %s\n\n", address);
		pthread_mutex_unlock(&mutexLinkTable);
		return 0;
	}


	while (pCurrentNode != NULL)
	{
		if (strncmp(pCurrentNode->deviceId, address, 16) != 0)
		{
			pPreviousNode = pCurrentNode;
			pCurrentNode = pCurrentNode->next;
		}
		else
		{
			char outBuff[400];
			packageReportMessageToJson(REPORT_TYPE_UNREGISTER, (void*)pCurrentNode->deviceId, outBuff);


			groupDeleteItem(address,NULL, NULL);
			bindDeleteAllItem(address);

			devcieNumber--;
			if (pCurrentNode == pHead && pCurrentNode == pTail)
			{
				pDelteNode = pCurrentNode;
				pHead = NULL;
				pTail = NULL;
			}
			else if (pCurrentNode == pHead && pCurrentNode != pTail)
			{
				pDelteNode = pCurrentNode;
				pHead = pCurrentNode->next;
			}
			else if (pCurrentNode != pHead && pCurrentNode == pTail)
			{
				pDelteNode = pCurrentNode;
				pPreviousNode->next = NULL;
				pTail = pPreviousNode;
			}
			else
			{
				pDelteNode = pCurrentNode;
				pPreviousNode->next = pCurrentNode->next;
			}
			pthread_mutex_unlock(&mutexLinkTable);
			freeDevInfoNode(pDelteNode);


			char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
			sprintf(sqlStr, "DELETE FROM Table_device_list WHERE deviceId='%s';", address);
			res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
			if (res != SQLITE_OK)
			{
				ERROR("[ZIGBEE] device_leave_cb: delete device %s infomation from database failed: %s\n", address, zErrMsg);
				sqlite3_free(zErrMsg);
			}
			/*插入离网上报*/
			messageReportFun(outBuff);
			return 0;
		}
	}

	pthread_mutex_unlock(&mutexLinkTable);
	return 0;
}


ReportAttrEventOnline*  buildAttributeReportStruct(char *devId, char *modelId, char *attriName, char *attriValue)
{
	ReportAttrEventOnline *reportAttr;
	reportAttr = (ReportAttrEventOnline *)malloc(sizeof(ReportAttrEventOnline));
	memset(reportAttr,0,sizeof(ReportAttrEventOnline));
	sprintf(reportAttr->deviceId, "%s", devId);
	sprintf(reportAttr->modelId, "%s", modelId);
	sprintf(reportAttr->key, "%s", attriName);
	sprintf(reportAttr->value, "%s", attriValue);
	reportAttr->next = NULL;
	return reportAttr;
}

void insertNodeToAttributeReportList(ReportAttrEventOnline* node, ReportAttrEventOnline *reportAttrHead, ReportAttrEventOnline *reportAttrTail)
{
	if (reportAttrHead == NULL)
	{
		reportAttrHead = node;
		reportAttrTail = node;
	}
	else
	{
		reportAttrTail->next = node;
		reportAttrTail = node;
	}
}

void addNewAttributeReportNodeToAttributeReportList(char *devId, char *modelId, char *key, char *value, ReportAttrEventOnline **reportAttrHead, ReportAttrEventOnline **reportAttrTail)
{

	if (*reportAttrHead == NULL)
	{
		*reportAttrHead = buildAttributeReportStruct(devId, modelId, key, value);
		*reportAttrTail = *reportAttrHead;
	}
	else
	{
		(*reportAttrTail)->next = buildAttributeReportStruct(devId, modelId, key, value);
		*reportAttrTail = (*reportAttrTail)->next;
	}

}

int reportFunction(const char *reportType, char *devId, char *modelId, char *attriName, char *attriValue)
{
	int res = 0;
	char outBuff[400];
	ReportAttrEventOnline *reportAttr;
	reportAttr = buildAttributeReportStruct(devId, modelId, attriName, attriValue);
	packageReportMessageToJson(reportType, (void*)reportAttr, outBuff);
	/*此处插入上报接口*/
	res = messageReportFun(outBuff);
	free(reportAttr);
	return res;
}

int reportBindListFunction(char *deviceId,char *modelId,cJSON *rexBindTable)
{
	int res=0;
	char outBuff[500];
	packageBindTalbeToJson(deviceId, modelId, rexBindTable,outBuff);
	DEBUG("[ZIGBEE] The report string length of bind list: %d\n\n",strlen(outBuff));
	res = messageReportFun(outBuff);
	return res;
}

int parseAppVersion(char *appVersion,char *outBuff)
{
	if (NULL==appVersion)
		return -1;
	unsigned char bin=atoi(appVersion);
	int ver1=bin>>6;
	int ver2=0x03 & (bin>>4);
	int ver3=0x0F & bin;
	sprintf(outBuff,"%d.%d.%d",ver1,ver2,ver3);
	return 0;
}

#define DELAY_TIME  (3)
void parseDeviceReportMessage(DevInfo *node, unsigned char endpoint_id, unsigned short stateType, cJSON *root)
{
	char outBuff[400] = { 0 };
	struct timeval now;
	gettimeofday(&now,NULL);
	switch (stateType)
	{
	case 0x0201:/*设备供电信息*/
	{
		cJSON *batPercentate = cJSON_GetObjectItem(root, "BatPercentage");
		cJSON *batVotage = cJSON_GetObjectItem(root, "BatVoltage");
		if (batVotage !=NULL)
			if (strcmp(batVotage->valuestring,"0.0")==0 )
				break;

		Attribute *currentAttr;
		currentAttr = node->attrHead;
		if (batPercentate != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_BATTERY_PERCENTAGE) == 0)
				{
					if (strcmp(currentAttr->value,batPercentate->valuestring)!=0 || \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", batPercentate->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}

	break;
	}
	case 0x0202:/*电能计量信息*/
	{
		Attribute *currentAttr;
		if (endpoint_id == 1)
		{
			cJSON *voltage = cJSON_GetObjectItem(root, "Voltage");
			if (voltage != NULL)
			{
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_RMS_VOLTAGE) == 0)
					{
						if (strcmp(currentAttr->value,voltage->valuestring)!=0 || \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", voltage->valuestring);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);

						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *current = cJSON_GetObjectItem(root, "Current");
			if (current != NULL)
			{
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_RMS_CURRENT) == 0)
					{
						if ( strcmp(currentAttr->value,current->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", current->valuestring);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}

						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *consumption = cJSON_GetObjectItem(root, "Consumption");
			if (consumption != NULL)
			{
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SUM_ELECTRIC) == 0)
					{
						if ((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME  || strcmp(currentAttr->value,consumption->valuestring)!=0)
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", consumption->valuestring);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}

						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *activPower = cJSON_GetObjectItem(root, "ActivePower");
			if (activPower != NULL)
			{
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_ACTIVE_POWER) == 0)
					{
						if ( strcmp(currentAttr->value,activPower->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", activPower->valuestring);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}

						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
		}

		cJSON *state = cJSON_GetObjectItem(root, "State");
		char keyName[ATTRIBUTE_KEY_MAX_LENGTH];
		if (state != NULL)
		{
			//printf("endpoint_id=%d;",endpoint_id);
			currentAttr = node->attrHead;
			switch (endpoint_id)
			{
			case 1:
				switch (node->subDevType)
				{
				case 0x1001:
					sprintf(keyName, "%s", ATTRIBUTE_NAME_SWITHCH);
					break;
				case 0x1002:
				case 0x1003:
				case 0x1004:
					if (strcmp(node->modelId,"HY0102")==0)
						sprintf(keyName, "%s", ATTRIBUTE_NAME_SWITHCH_1);
					else
						sprintf(keyName, "%s", ATTRIBUTE_NAME_SWITCH_MULTIPLE_1);
					break;
				default:
					break;
				}
				break;
			case 2:
				if (strcmp(node->modelId,"HY0102")==0)
					sprintf(keyName, "%s", ATTRIBUTE_NAME_SWITHCH_2);
				else
					sprintf(keyName, "%s", ATTRIBUTE_NAME_SWITCH_MULTIPLE_2);
				break;
			case 3:
			case 4:
				break;
			default:
				break;
			}
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, keyName) == 0)
				{

					if (strcmp(currentAttr->value,state->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", state->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}

		break;
	}
	case 0x0203:/*温度检测信息*/
	{
		cJSON *temperature = cJSON_GetObjectItem(root, "Temperature");
		Attribute *currentAttr;
		currentAttr = node->attrHead;
		if (temperature != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE) == 0)
				{
					if (strcmp(currentAttr->value,temperature->valuestring)!=0 || \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", temperature->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}
		break;
	}
	case 0x0204:/*相对湿度信息*/
	{
		cJSON *humidity = cJSON_GetObjectItem(root, "Humidity");
		Attribute *currentAttr;
		currentAttr = node->attrHead;
		if (humidity != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_HUMIDITY) == 0)
				{
					if (strcmp(currentAttr->value,humidity->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", humidity->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}
		break;
	}
	case 0x0205: /*光照度检测信息*/
	{
		switch (node->subDevType)
		{
		case REX_DEV_TYPE_THERMAL_RELEASE:
		case 0x1208:
		case 0x1303:
		{
			cJSON *illuminance = cJSON_GetObjectItem(root, "Illuminance");
			if (illuminance != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_LUMINANCE) == 0 \
						|| strcmp(currentAttr->key, ATTRIBUTE_NAME_LUMINANCE) == 0 \
						|| strcmp(currentAttr->key, ATTRIBUTE_NAME_MEASURE_LUMINANCE) == 0 \
						|| strcmp(currentAttr->key, ATTRIBUTE_NAME_ILLUMINANCE) == 0)
					{
						if (strcmp(currentAttr->value,illuminance->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", illuminance->valuestring);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
			break;
		}

		default:
			break;
		}
		break;
	}
	case 0x0206:/*安防传感器信息*/
	{
		switch (node->subDevType)
		{
		case REX_DEV_TYPE_HUMMAN_BODY_INDUCTION:
		case REX_DEV_TYPE_SECURITY_REMOTE_CONTROL:
		{

			Attribute *currentAttr;
			currentAttr = node->attrHead;
			//int isreport = 1;//report=1,no report=0;
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, "ProtectionStatus") == 0)
				{
					if (strncmp(currentAttr->value, "0", 1) == 0)
					{
						//isreport = 0;
					}
					break;
				}
				else
					currentAttr = currentAttr->next;
			}

			cJSON *alarm = cJSON_GetObjectItem(root, "Alarm");
			currentAttr = node->attrHead;
			if (alarm != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MOTION_ALARM) == 0 ||\
							strcmp(currentAttr->key, ATTRIBUTE_NAME_KEY_FOB_VALUE) == 0)
					{
						gettimeofday(&currentAttr->tv,NULL);
						if((strcmp(node->modelId,"4a0e0d")==0 || strcmp(node->modelId,"RH6010")==0))
						{
							if(strcmp(alarm->valuestring,"1")==0)
								sprintf(currentAttr->value, "%s", "4");
							else
								break;
						}
						else
							sprintf(currentAttr->value, "%s", alarm->valuestring);
						//if(isreport==1)
						//	reportFunction(REPORT_TYPE_EVENT, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *lowBatteryAlarm = cJSON_GetObjectItem(root, "BatteryLevel");
			currentAttr = node->attrHead;
			if (lowBatteryAlarm != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_LOW_BARRERY_ALARM) == 0)
					{
						if (strcmp(currentAttr->value,lowBatteryAlarm->valuestring)!=0)
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", lowBatteryAlarm->valuestring);


							ReportAttrEventOnline reportAttr;
							sprintf(reportAttr.deviceId, "%s", node->deviceId);
							sprintf(reportAttr.modelId, "%s", node->modelId);
							sprintf(reportAttr.key, "%s", currentAttr->key);
							sprintf(reportAttr.value, "%s", currentAttr->value);
							reportAttr.next = NULL;
							packageReportMessageToJson(REPORT_TYPE_EVENT, (void*)&reportAttr, outBuff);
							/*此处插入上报接口*/
							messageReportFun(outBuff);

						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			break;
		}
		case REX_DEV_TYPE_SMOKE_GAS_SENSOR:
		{
			cJSON *alarm = cJSON_GetObjectItem(root, "Alarm");
			Attribute *currentAttr;
			currentAttr = node->attrHead;
			int isreport = 1;//report=1,no report=0;
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, "ProtectionStatus") == 0)
				{
					if (strncmp(currentAttr->value, "0", 1) == 0)
					{
						isreport = 0;
					}
					break;
				}
				else
				currentAttr = currentAttr->next;
			}

			currentAttr = node->attrHead;
			if (alarm != NULL)
			{
				while (currentAttr != NULL)  //ATTRIBUTE_NAME_CO_ALARM
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SMOKE_ALARM) == 0 \
							||( endpoint_id==1 && strcmp(currentAttr->key, ATTRIBUTE_NAME_GAS_ALARM) == 0) \
							|| ( endpoint_id==2 && strcmp(currentAttr->key, ATTRIBUTE_NAME_CO_ALARM) == 0))
					{

						if (strcmp(currentAttr->value,alarm->valuestring)!=0)
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", alarm->valuestring);
							if(isreport==1)
								reportFunction(REPORT_TYPE_EVENT, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
			break;
		}
		case REX_DEV_TYPE_DOOR_MAGNETISM:
		case REX_DEV_TYPE_WATER_OVERFLOW_SENSOR:
		case REX_DEV_TYPE_THERMAL_RELEASE:
		{
			cJSON *alarm = cJSON_GetObjectItem(root, "Alarm");
			Attribute *currentAttr;
			currentAttr = node->attrHead;
			//int isreport = 1;//report=1,no report=0;
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, "ProtectionStatus") == 0)
				{
					if (strncmp(currentAttr->value, "0", 1) == 0)
					{
						//isreport = 0;
					}
					break;

				}
				else
					currentAttr = currentAttr->next;
			}

			currentAttr = node->attrHead;
			if (alarm != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CONTACT_ALARM) == 0 \
						|| strcmp(currentAttr->key, ATTRIBUTE_NAME_MOTION_ALARM) == 0 \
						|| strcmp(currentAttr->key, ATTRIBUTE_NAME_MOTION_CURTAIN_ALARM) == 0\
						|| strcmp(currentAttr->key, ATTRIBUTE_NAME_WATER_ALARM) == 0\
						|| strcmp(currentAttr->key,ATTRIBUTE_NAME_DIRECTION_ALARM)==0)
					{
						if (strcmp(currentAttr->value,alarm->valuestring)!=0 )
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", alarm->valuestring);

							ReportAttrEventOnline reportAttr;
							sprintf(reportAttr.deviceId, "%s", node->deviceId);
							sprintf(reportAttr.modelId, "%s", node->modelId);
							sprintf(reportAttr.key, "%s", currentAttr->key);
							sprintf(reportAttr.value, "%s", currentAttr->value);
							reportAttr.next = NULL;
							packageReportMessageToJson(REPORT_TYPE_EVENT, (void*)&reportAttr, outBuff);
						}

						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *secondAlarm = cJSON_GetObjectItem(root, "SecondAlarm");
			currentAttr = node->attrHead;
			if (secondAlarm != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_DIRECTION_ALARM) == 0)
					{
						if (strcmp(currentAttr->value,secondAlarm->valuestring)!=0)//||(now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME)
						{
							gettimeofday(&currentAttr->tv,NULL);
							if (1==isTwoInfraredCurtainSensor(node->modelId,secondAlarm->valuestring))
								sprintf(currentAttr->value, "%s", "2");
							else
								sprintf(currentAttr->value, "%s", secondAlarm->valuestring);


							ReportAttrEventOnline reportAttr;
							sprintf(reportAttr.deviceId, "%s", node->deviceId);
							sprintf(reportAttr.modelId, "%s", node->modelId);
							sprintf(reportAttr.key, "%s", currentAttr->key);
							sprintf(reportAttr.value, "%s", currentAttr->value);
							reportAttr.next = NULL;
							packageReportMessageToJson(REPORT_TYPE_EVENT, (void*)&reportAttr, outBuff);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *tamperAlarm = cJSON_GetObjectItem(root, "Tamper");
			currentAttr = node->attrHead;
			if (tamperAlarm != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TAMPER_ALARM) == 0)
					{
						if (strcmp(currentAttr->value,tamperAlarm->valuestring)!=0)//||(now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME)
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", tamperAlarm->valuestring);

							ReportAttrEventOnline reportAttr;
							sprintf(reportAttr.deviceId, "%s", node->deviceId);
							sprintf(reportAttr.modelId, "%s", node->modelId);
							sprintf(reportAttr.key, "%s", currentAttr->key);
							sprintf(reportAttr.value, "%s", currentAttr->value);
							reportAttr.next = NULL;
							packageReportMessageToJson(REPORT_TYPE_EVENT, (void*)&reportAttr, outBuff);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *lowBatteryAlarm = cJSON_GetObjectItem(root, "BatteryLevel");
			currentAttr = node->attrHead;
			if (lowBatteryAlarm != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_LOW_BARRERY_ALARM) == 0)
					{
						if (strcmp(currentAttr->value,lowBatteryAlarm->valuestring)!=0)//||(now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME)
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", lowBatteryAlarm->valuestring);

							ReportAttrEventOnline reportAttr;
							sprintf(reportAttr.deviceId, "%s", node->deviceId);
							sprintf(reportAttr.modelId, "%s", node->modelId);
							sprintf(reportAttr.key, "%s", currentAttr->key);
							sprintf(reportAttr.value, "%s", currentAttr->value);
							reportAttr.next = NULL;
							packageReportMessageToJson(REPORT_TYPE_EVENT, (void*)&reportAttr, outBuff);
						/*此处插入上报接口*/
							messageReportFun(outBuff);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}


			break;
		}
		default:
			break;
		}

		break;
	}
	case 0x0207:/*照明设备及灯具信息*/
	{

		switch (node->subDevType)
		{
		case 0x1101:
		case 0x1102:
		case 0x1103:
		case 0x1104:/*只上报开关状态*/
		case 0x1307:
		{
			cJSON *state = cJSON_GetObjectItem(root, "State");
			if (state != NULL)
			{

				Attribute *currentAttr;
				currentAttr = node->attrHead;

				switch (endpoint_id)
				{
				case 1:
				{
					while (currentAttr != NULL)
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_1) == 0 ||\
								strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH) == 0)
						{
							if ( strcmp(currentAttr->value,state->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{

								sprintf(currentAttr->value, "%s", state->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);

#ifdef MULTI_SWITCH_SYNC
								if(currentAttr->tv.tv_sec !=0 || (currentAttr->tv.tv_sec==0 && strcmp(state->valuestring,"0")==0))
									checkSyncSwitchReport(node->deviceId,currentAttr->key,endpoint_id,currentAttr->value);
#endif
								gettimeofday(&currentAttr->tv,NULL);
							}
							break;
						}
						else
						{
							currentAttr = currentAttr->next;
						}
					}
					break;
				}
				case 2:
				case 3:
				case 4:
				case 5:
				case 6:
				case 7:
				case 8:
				{
					char keyName[12]={0};
					sprintf(keyName,"%s%d","Switch_",endpoint_id);
					while (currentAttr != NULL)
					{
						if ( 0== strcmp(currentAttr->key, keyName)|| \
							(0==strcmp(node->realModelId, "HY0160") && \
							((0==strcmp(currentAttr->key, ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH_1) && 5==endpoint_id) ||\
							(0==strcmp(currentAttr->key, ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH_2) && 6==endpoint_id)||\
							(0==strcmp(currentAttr->key, ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH_3) && 7==endpoint_id)||\
							(0==strcmp(currentAttr->key, ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH_4)&& 8==endpoint_id))))
						{
							if (strcmp(currentAttr->value,state->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								sprintf(currentAttr->value, "%s", state->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
#ifdef MULTI_SWITCH_SYNC
								if(currentAttr->tv.tv_sec !=0 || (currentAttr->tv.tv_sec==0 && strcmp(state->valuestring,"0")==0))
									checkSyncSwitchReport(node->deviceId,currentAttr->key,endpoint_id,currentAttr->value);
#endif
								gettimeofday(&currentAttr->tv,NULL);
							}
							break;
						}
						else
						{
							currentAttr = currentAttr->next;
						}
					}
					break;
				}

				default:
					break;
				}
			}
			break;
		}
		case 0x1105:
		case 0x1106:
		case 0x1107:
		case 0x1108:
		{
			break;
		}
		case 0x1109:
		{
			cJSON *state = cJSON_GetObjectItem(root, "State");
			if (state != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH) == 0)
					{
						if (strcmp(currentAttr->value,state->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", state->valuestring);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}

				}
			}
			cJSON *level = cJSON_GetObjectItem(root, "Level");
			if (level != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_LUMINANCE) == 0)
					{
						char tmp[4]={0};

						int lev = atoi(level->valuestring);

						if (lev <= 1)
						{
							sprintf(tmp, "%s", "1");
						}
						else
						{
							sprintf(tmp, "%s", level->valuestring);
						}
						if (strcmp(currentAttr->value,tmp)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", tmp);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *temperature = cJSON_GetObjectItem(root, "Temperature");
			if (temperature != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_COLOR_TEMPERATURE) == 0)
					{
						char tmpTemper[6]={0};
						int temperInt = atoi(temperature->valuestring);
						if (temperInt <= 2700)
						{
							sprintf(tmpTemper, "%s", "2700");
						}
						else if (temperInt > 2700 && temperInt < 6500)
						{
							int tmp1 = temperInt / 100;
							float tmp2 = temperInt % 100;
							if (tmp2 >= 50)
							{
								sprintf(tmpTemper, "%d", (tmp1 + 1) * 100);
							}
							else
							{
								sprintf(tmpTemper, "%d", tmp1 * 100);
							}

						}
						else
						{
							sprintf(tmpTemper, "%s", "6500");
						}

						if ( strcmp(currentAttr->value,tmpTemper)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", tmpTemper);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
			break;
		}
		case 0x1110:
		{
			cJSON *state = cJSON_GetObjectItem(root, "State");
			if (state != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH) == 0)
					{
						sprintf(currentAttr->value, "%s", state->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}

				}
			}


			cJSON *level = cJSON_GetObjectItem(root, "Level");
			if (level != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_LIGHTNESS) == 0 \
						|| strcmp(currentAttr->key, ATTRIBUTE_NAME_LUMINANCE) == 0)
					{
						char tmp[4]={0};

						int lev = atoi(level->valuestring);
						if (lev <= 1)
						{
							sprintf(tmp, "%s", "1");
						}
						else
						{
							sprintf(tmp, "%s", level->valuestring);
						}
						if ( strcmp(currentAttr->value,tmp)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", tmp);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *temperature = cJSON_GetObjectItem(root, "Temperature");
			if (temperature != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_COLOR_TEMPERATURE) == 0)
					{
						char tmpT[5]={0};
						int temperInt = atoi(temperature->valuestring);
						if (temperInt <= 2700)
						{
							sprintf(tmpT, "%s", "2700");
						}
						else if (temperInt > 2700 && temperInt < 6500)
						{
							int tmp1 = temperInt / 100;
							float tmp2 = temperInt % 100;
							if (tmp2 >= 50)
							{
								sprintf(tmpT, "%d", (tmp1 + 1) * 100);
							}
							else
							{
								sprintf(tmpT, "%d", tmp1 * 100);
							}

						}
						else
						{
							sprintf(tmpT, "%s", "6500");
						}

						if ( strcmp(currentAttr->value,tmpT)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", tmpT);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *hue = cJSON_GetObjectItem(root, "HUE");
			if (hue != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_HUE) == 0)
					{
						int realhue = atoi(hue->valuestring);
						if (realhue > 254)
						{
							WARN("[ZIGBEE] hue value %d is error, max is 254\n", realhue);
						}
						else
						{
							realhue = (realhue * 360) / 254;
							sprintf(currentAttr->value, "%d", realhue);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *saturation = cJSON_GetObjectItem(root, "Saturation");
			if (saturation != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SATURATION) == 0)
					{
						int realsatu = atoi(saturation->valuestring);
						if (realsatu > 254)
						{
							WARN("[ZIGBEE] saturation value %d is error ,max is 254\n", realsatu);
						}
						else
						{

							realsatu = (realsatu * 100) / 254;
							sprintf(currentAttr->value, "%d", realsatu);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			break;
		}
		case 0x1111:
		{
			break;
		}
		case 0x1208:
		{
			cJSON *state = cJSON_GetObjectItem(root, "State");
			if (state != NULL)
			{
				Attribute *currentAttr;
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH) == 0)
					{
						if ( strcmp(currentAttr->value,state->valuestring)!=0)
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", state->valuestring);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case 0x0208:
	{
		cJSON *state = cJSON_GetObjectItem(root, "State");
		Attribute *currentAttr;

		if (state != NULL)
		{
			currentAttr = node->attrHead;
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_LOCK_STATE) == 0)
				{
					if (strcmp(state->valuestring, "0") == 0)/*未完全上锁*/
					{
						sprintf(currentAttr->value, "%d", 0);
					}
					else if (strcmp(state->valuestring, "1") == 0)/*上锁*/
					{
						sprintf(currentAttr->value, "%d", 1);
					}
					else if (strcmp(state->valuestring, "2") == 0)/*解锁*/
					{
						sprintf(currentAttr->value, "%d", 2);
					}
					else if (strcmp(state->valuestring, "3") == 0)/*内部锁*/
					{
						sprintf(currentAttr->value, "%d", 3);
					}

					reportFunction(REPORT_TYPE_EVENT, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}

		cJSON *remoteUnlock = cJSON_GetObjectItem(root, "RemoteOpenResult");
		if (remoteUnlock)
		{
			char *strRes=cJSON_PrintUnformatted(root);
			reportFunction(REPORT_TYPE_EVENT, node->deviceId, node->modelId, EVENT_REMOTE_UNLOCK_NOTIFICATION, strRes);
			free(strRes);
			break;
		}
		cJSON *addKey = cJSON_GetObjectItem(root, "AddKey");
		if (addKey)
		{
			char *strRes=cJSON_PrintUnformatted(root);
			reportFunction(REPORT_TYPE_EVENT, node->deviceId, node->modelId, EVENT_ADD_OPT_NOTIFICATION, strRes);
			free(strRes);
			break;
		}
		cJSON *setTimeRes = cJSON_GetObjectItem(root, "SetTimeResult");
		if(setTimeRes)
		{
			char *strRes=cJSON_PrintUnformatted(root);
			reportFunction(REPORT_TYPE_EVENT, node->deviceId, node->modelId, EVENT_KEY_TIME_NOTIFICATION, strRes);
			free(strRes);
			break;
		}
		cJSON *clearSchedule = cJSON_GetObjectItem(root, "ClearSchedule");
		if(clearSchedule)
		{
			char *strRes=cJSON_PrintUnformatted(root);
			reportFunction(REPORT_TYPE_EVENT, node->deviceId, node->modelId, EVENT_REMOVE_KEY_TIME_NOTIFICATION, strRes);
			free(strRes);
			break;
		}

		cJSON *clearTimeRes = cJSON_GetObjectItem(root, "ClearTimeRes");
		cJSON *setSchedule = cJSON_GetObjectItem(root, "SetSchedule");
		break;
	}
	case 0x0209:/*电动窗帘信息*/
	{
		cJSON *level = cJSON_GetObjectItem(root, "Level");
		Attribute *currentAttr;
		currentAttr = node->attrHead;
		if (level != NULL)
		{
			while (currentAttr != NULL)
			{
				if ((strcmp(currentAttr->key, ATTRIBUTE_NAME_CURTAIN_POSITION) == 0) \
					|| (strcmp(currentAttr->key, ATTRIBUTE_NAME_ACTUATOR_POSITION) == 0) \
					|| ((strcmp(currentAttr->key, ATTRIBUTE_NAME_CURTAIN_POSITION_1) == 0) && (endpoint_id==1)) \
					|| ((strcmp(currentAttr->key, ATTRIBUTE_NAME_CURTAIN_POSITION_2) == 0) && endpoint_id==2))
				{
					if (strcmp(currentAttr->value,level->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", level->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}

		cJSON *runMode = cJSON_GetObjectItem(root, "RunMode");
		currentAttr = node->attrHead;
		if (runMode != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_RUN_MODE) == 0 \
						|| (strcmp(currentAttr->key, ATTRIBUTE_NAME_RUN_MODE_1) == 0&& endpoint_id==1)\
						|| (strcmp(currentAttr->key, ATTRIBUTE_NAME_RUN_MODE_2) == 0&& endpoint_id==2))
				{
					if (strcmp(currentAttr->value,runMode->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", runMode->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}
		break;
	}
	case 0x020D:/*自动上报周期*/
		break;
	case 0x020E:
	{
		cJSON *dataCode = cJSON_GetObjectItem(root, "DateCode");
		if (dataCode != NULL)/* && 0!=strcmp(node->modelId,"HY0112") && 0!=strcmp(node->modelId,"HY0134"))*/
		{
			if (strcmp(node->softwareVersion, dataCode->valuestring) != 0)
			{
				if (strcmp(node->softwareVersion,dataCode->valuestring)!=0)
				{
					sprintf(node->softwareVersion, "%s", dataCode->valuestring);
					char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
					sprintf(sqlStr, "UPDATE Table_device_list SET softwareVersion=\'%s\' WHERE deviceId=\'%s\';", dataCode->valuestring, node->deviceId);
					int res = 0;
					res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
					if (res != SQLITE_OK)
					{
						ERROR("[ZIGBEE] Inserter new device information into database error: %s\n", zErrMsg);
						sqlite3_free(zErrMsg);
					}
					reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, "SofterVersion", dataCode->valuestring);
				}

			}
		}

		cJSON *appVersion = cJSON_GetObjectItem(root, "AppVersion");
		if (NULL!=appVersion && 1==isTYDeviceFirmware(node->realModelId))
		{
			char versoin[20];
			parseAppVersion(appVersion->valuestring,versoin);
			if (strcmp(node->softwareVersion,versoin)!=0)
			{
				sprintf(node->softwareVersion, "%s", versoin);
				char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
				sprintf(sqlStr, "UPDATE Table_device_list SET softwareVersion=\'%s\' WHERE deviceId=\'%s\';", node->softwareVersion, node->deviceId);
				int res = 0;
				res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
				if (res != SQLITE_OK)
				{
					ERROR("[ZIGBEE] Inserter new device information into database error: %s\n", zErrMsg);
					sqlite3_free(zErrMsg);
				}
				reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, "SofterVersion",node->softwareVersion);
			}
		}
		break;
	}
	case 0x020F:
		break;
	case 0x0216:/*新风系统信息*/
	{
		if (node->subDevType == 0x1305 ||node->subDevType == 0x1350)
		{
			cJSON *fanMode = cJSON_GetObjectItem(root, "FanMode");
			Attribute *currentAttr;
			currentAttr = node->attrHead;

			if (fanMode != NULL)
			{
				if (strcmp(fanMode->valuestring, "0") == 0 || strcmp(fanMode->valuestring, "4") == 0)
				{
					while (currentAttr != NULL)
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH) == 0)
						{
							char tmpValue[2];
							if (strcmp(fanMode->valuestring, "4") == 0)
							{
								sprintf(tmpValue, "%s", "1");
							}
							else
								sprintf(tmpValue, "%s", "0");

							if (strcmp(currentAttr->value,tmpValue)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", tmpValue);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;
						}
						else
						{
							currentAttr = currentAttr->next;
						}
					}
				}
				else if (strcmp(fanMode->valuestring, "1") == 0 || \
						strcmp(fanMode->valuestring, "2") == 0 || \
						strcmp(fanMode->valuestring, "3") == 0 ||\
						strcmp(fanMode->valuestring, "10") == 0)
				{
					while (currentAttr != NULL)
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED) == 0 ||\
								(strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_1) == 0 && 1==endpoint_id)||\
								(strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_2) == 0 && 2==endpoint_id))
						{
							char tmpValue[2];
							if (strcmp(fanMode->valuestring, "1") == 0)
							{
								sprintf(tmpValue, "%s", "2");
							}
							else if (strcmp(fanMode->valuestring, "2") == 0)
							{
								sprintf(tmpValue, "%s", "3");
							}
							else if (strcmp(fanMode->valuestring, "3") == 0)
							{
								sprintf(tmpValue, "%s", "4");
							}
							else if (strcmp(fanMode->valuestring, "10") == 0)
							{
								sprintf(tmpValue, "%s", "10");
							}


							if (strcmp(currentAttr->value,tmpValue)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", tmpValue);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;
						}
						else
						{
							currentAttr = currentAttr->next;
						}
					}
				}
			}


			cJSON *temperature = cJSON_GetObjectItem(root, "Temperature");
			currentAttr = node->attrHead;
			if (temperature!=NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMP) == 0)
					{
						if (strcmp(currentAttr->value,temperature->valuestring)!=0|| \
						((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", temperature->valuestring);

							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
		}
		break;
	}
	case 0x0217:/*加热制冷设备信息*/
	{
		switch (node->subDevType)
		{
		case REX_DEV_TYPE_HEART_COOL_DEVICE:
		{
			cJSON *temperature = cJSON_GetObjectItem(root, "Temperature");
			Attribute *currentAttr;
			if (temperature != NULL)
			{
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE) == 0)
					{
						if (strcmp(currentAttr->value,temperature->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", temperature->valuestring);

							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *targetTemperature = cJSON_GetObjectItem(root, "TargetTemperature");

			if (targetTemperature != NULL)
			{
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE) == 0)
					{
						if ( strcmp(currentAttr->value,targetTemperature->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", targetTemperature->valuestring);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			cJSON *workMode = cJSON_GetObjectItem(root, "AutoWorkMode");

			if (workMode != NULL)
			{
				currentAttr = node->attrHead;
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH) == 0)
					{
						char tmpValue[2];
						if (strcmp(workMode->valuestring, "0") == 0)
						{
							sprintf(tmpValue, "%s", "0");
						}
						else if (strcmp(workMode->valuestring, "10") == 0 || strcmp(workMode->valuestring, "4") == 0)
						{
							sprintf(tmpValue, "%s", "1");
						}
						if ( strcmp(currentAttr->value,tmpValue)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", tmpValue);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
			break;
		}
		default:
			break;
		}
		break;
	}
	case 0x0218:/*空调温控设备*/
	{
		if (node->subDevType == 0x1305 || node->subDevType == 0x1306 || node->subDevType == 0x1350)
		{
			return;
		}
		int find = 0;
		cJSON *temperature = cJSON_GetObjectItem(root, "Temperature");
		Attribute *currentAttr;
		if (temperature != NULL)
		{
			char tmp[8];
			currentAttr = node->attrHead;
			while (currentAttr != NULL)
			{
				switch (endpoint_id)
				{
				case 1:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMP) == 0 || strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_1) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 2:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_2) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 3:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_3) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 4:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_4) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 5:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_5) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 6:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_6) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 7:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_7) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 8:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_8) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 9:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_9) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 10:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_10) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 11:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_11) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 12:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_12) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						continue;
					}
					break;
				}
				case 13:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_13) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						continue;
					}
					break;
				}
				case 14:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_14) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						continue;
					}
					break;
				}
				case 15:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_15) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						continue;
					}
					break;
				}
				case 16:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_TEMPERATURE_16) == 0)
					{
						sprintf(tmp, "%s", temperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						continue;
					}
					break;
				}
				default:
					currentAttr = currentAttr->next;
				}
				if (find == 1)
				{
					if (strcmp(currentAttr->value,tmp)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s",tmp);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}

			}
		}

		cJSON *targetTemperature = cJSON_GetObjectItem(root, "TargetTemperature");
		if (targetTemperature != NULL)
		{
			char tmp[8];
			currentAttr = node->attrHead;
			while (currentAttr != NULL)
			{
				switch (endpoint_id)
				{
				case 1:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TEMPERATUR) == 0 || strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_1) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 2:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_2) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 3:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_3) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 4:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_4) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 5:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_5) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 6:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_6) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 7:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_7) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 8:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_8) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 9:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_9) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 10:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_10) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 11:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_11) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 12:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_12) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 13:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_13) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 14:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_14) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 15:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_15) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 16:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_16) == 0)
					{
						sprintf(tmp, "%s", targetTemperature->valuestring);
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				default:
					currentAttr = currentAttr->next;
				}
				if (find == 1)
				{
					if ( strcmp(currentAttr->value,tmp)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s",tmp);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						char targetAddr[18]={0};
						int targetEndpoint;
						if(0==bindCheck(node->deviceId,targetAddr,&targetEndpoint))
							sendCommandSetTemperature(targetAddr, targetEndpoint, tmp);
					}
					break;
				}

			}
		}

		cJSON *workMode = cJSON_GetObjectItem(root, "WorkMode");
		if (workMode != NULL)
		{
			if (strcmp(workMode->valuestring, "0") == 0 || strcmp(workMode->valuestring, "10") == 0 \
					|| strcmp(workMode->valuestring,"16") == 0)
			{
				currentAttr = node->attrHead;
				int state = 0;
				if (strcmp(workMode->valuestring, "0") == 0)
					state = 0;
				else
					state = 1;
				char tmp[2];
				while (currentAttr != NULL)
				{

					switch (endpoint_id)
					{
					case 1:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH) == 0 || strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_1) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 2:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_2) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 3:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_3) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 4:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_4) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 5:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_5) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 6:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_6) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 7:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_7) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 8:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_8) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 9:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_9) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 10:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_10) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 11:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_11) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 12:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_12) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 13:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_13) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 14:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_14) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 15:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_15) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 16:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITHCH_16) == 0)
						{
							sprintf(tmp, "%d", state);
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					default:
						currentAttr = currentAttr->next;
					}
					if (find == 1)
					{
						if ( strcmp(currentAttr->value,tmp)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s",tmp);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							char targetAddr[18]={0};
							int targetEndpoint;
							if (0==bindCheck(node->deviceId,targetAddr,&targetEndpoint))
								sendCommandWorkMode(targetAddr, targetEndpoint, workMode->valuestring);
						}
						break;
					}
				}
			}
			else
			{
				currentAttr = node->attrHead;
				int mode = 0;
				if (strcmp(workMode->valuestring, "1") == 0)
					mode = 0;
				else if (strcmp(workMode->valuestring, "3") == 0)
					mode = 1;
				else if (strcmp(workMode->valuestring, "4") == 0)
					mode = 2;
				else if (strcmp(workMode->valuestring, "7") == 0)
					mode = 3;
				else if (strcmp(workMode->valuestring, "8") == 0)
					mode = 4;
				char tmpvalue[2];
				sprintf(tmpvalue,"%d",mode);
				while (currentAttr != NULL)
				{

					switch (endpoint_id)
					{
					case 1:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE) == 0 || strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_1) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 2:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_2) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 3:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_3) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 4:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_4) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 5:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_5) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 6:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_6) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 7:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_7) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 8:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_8) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 9:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_9) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 10:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_10) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 11:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_11) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 12:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_12) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 13:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_13) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 14:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_14) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 15:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_15) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					case 16:
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_MODE_16) == 0)
						{
							find = 1;
						}
						else
						{
							currentAttr = currentAttr->next;
							find = 0;
						}
						break;
					}
					default:
						currentAttr = currentAttr->next;
					}
					if (find == 1)
					{
						if (strcmp(currentAttr->value,tmpvalue)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s",tmpvalue);
							reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							char targetAddr[18]={0};
							int targetEndpoint;
							if (0==bindCheck(node->deviceId,targetAddr,&targetEndpoint))
								sendCommandWorkMode(targetAddr, targetEndpoint, workMode->valuestring);

						}
						break;
					}

				}
			}

		}


		cJSON *fanMode = cJSON_GetObjectItem(root, "FanMode");
		if (fanMode != NULL)
		{
			currentAttr = node->attrHead;
			int mode = 0;
			if (strcmp(fanMode->valuestring, "0") == 0)
				mode = 0;
			else if (strcmp(fanMode->valuestring, "1") == 0)
				mode = 2;
			else if (strcmp(fanMode->valuestring, "2") == 0)
				mode = 3;
			else if (strcmp(fanMode->valuestring, "3") == 0)
				mode = 4;
			char tmp[2];
			sprintf(tmp,"%d",mode);
			while (currentAttr != NULL)
			{

				switch (endpoint_id)
				{
				case 1:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED) == 0 || strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_1) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 2:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_2) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 3:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_3) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 4:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_4) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 5:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_5) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 6:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_6) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 7:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_7) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 8:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_8) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 9:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_9) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 10:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_10) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 11:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_11) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 12:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_12) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 13:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_13) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 14:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_14) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 15:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_15) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				case 16:
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_WIND_SPEED_16) == 0)
					{
						find = 1;
					}
					else
					{
						currentAttr = currentAttr->next;
						find = 0;
					}
					break;
				}
				default:
					currentAttr = currentAttr->next;
				}
				if (find == 1)
				{
					if ( strcmp(currentAttr->value,tmp)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s",tmp);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
						char targetAddr[18]={0};
						int targetEndpoint;
						if (0==bindCheck(node->deviceId,targetAddr,&targetEndpoint))
							sendCommandFreshFanMode(targetAddr, targetEndpoint, fanMode->valuestring);
					}
					break;
				}

			}
		}

		break;
	}
	case 0x021C:/*空气检测信息*/
	{
		cJSON *pm25 = cJSON_GetObjectItem(root, "PM2.5");
		Attribute *currentAttr;
		currentAttr = node->attrHead;
		if (pm25 != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_PM25) == 0)
				{
					if (strcmp(currentAttr->value,pm25->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", pm25->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}

					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}

		cJSON *voc = cJSON_GetObjectItem(root, "VOC");
		currentAttr = node->attrHead;
		if (voc != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_TVOC) == 0 || strcmp(currentAttr->key, ATTRIBUTE_NAME_VOC) == 0)
				{
					if ( strcmp(currentAttr->value,voc->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", voc->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}

		cJSON *co2 = cJSON_GetObjectItem(root, "CO2");
		currentAttr = node->attrHead;
		if (co2 != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CO2) == 0)
				{
					if ( strcmp(currentAttr->value,co2->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", co2->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}

		cJSON *ch20 = cJSON_GetObjectItem(root, "CH2O");
		currentAttr = node->attrHead;
		if (ch20 != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_HCHO) == 0)
				{
					if ( strcmp(currentAttr->value,ch20->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", ch20->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}

		break;
	}
	case 0x0219:
	{
		cJSON *responseType = cJSON_GetObjectItem(root, "ResponseType");
		cJSON *result =cJSON_GetObjectItem(root, "Result");
		cJSON *groupid = NULL;
		groupid=cJSON_GetObjectItem(root, "GroupId");
		cJSON *member = cJSON_GetObjectItem(root, "Membership");
		char outBuff[500]={0};
		if (result!=NULL)
		{
			if (groupid!=NULL)
				packageGroupResult(node->deviceId, node->modelId, responseType->valuestring,groupid->valuestring, result->valuestring,endpoint_id,outBuff);
			else
				packageGroupResult(node->deviceId, node->modelId, responseType->valuestring,NULL, result->valuestring,endpoint_id,outBuff);
		}
		else if (member!=NULL)
			packageGroupNumber(node->deviceId, node->modelId, responseType->valuestring, member,endpoint_id,outBuff);
		messageReportFun(outBuff);
		break;
	}
	case 0x021A:
	{
		cJSON *responseType = cJSON_GetObjectItem(root, "ResponseType");

		char tmp[2];
		sprintf(tmp,"%s",responseType->valuestring);

		cJSON_DeleteItemFromObject(root,"ResponseType");
		char *result;
		result=cJSON_Print(root);
		char outBuff[500]={0};
		packageSceneResult(node->deviceId, node->modelId, tmp, result,outBuff);
		messageReportFun(outBuff);

		if (result!=NULL)
			free(result);
		break;
	}
	case 0x021D:
	{
		cJSON *responseType = cJSON_GetObjectItem(root, "ResponseType");
		cJSON *result = cJSON_GetObjectItem(root, "Result");
		if (responseType!=NULL && result!=NULL)
		{
			reportFunction(REPORT_TYPE_BIND, node->deviceId, node->modelId, responseType->valuestring, result->valuestring);
		}
		break;
	}
	case 0X021E:
	{
		reportBindListFunction(node->deviceId,node->modelId,root);
		break;
	}
	case 0x021F: /*信号强度*/
	{
		cJSON *rssi = cJSON_GetObjectItem(root, "LastMessageRSSI");
		Attribute *currentAttr;
		currentAttr = node->attrHead;
		if (rssi != NULL)
		{

			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_RSSI) == 0)
				{
					if ( strcmp(currentAttr->value,rssi->valuestring)!=0|| \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", rssi->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}
		break;
	}
	case 0x0222: /*气压参数上报*/
	{
		cJSON *pressure = cJSON_GetObjectItem(root, "Pressure");
		Attribute *currentAttr;
		currentAttr = node->attrHead;
		if (pressure != NULL)
		{
			while (currentAttr != NULL)
			{
				if (strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_PRESSURE) == 0)
				{
					if (strcmp(currentAttr->value,pressure->valuestring)!=0 || \
							((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
					{
						gettimeofday(&currentAttr->tv,NULL);
						sprintf(currentAttr->value, "%s", pressure->valuestring);
						reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
					}
					break;
				}
				else
				{
					currentAttr = currentAttr->next;
				}
			}
		}
		break;
	}
	case 0x02F0: /*童锁/ 风雨传感器*/
	{
		cJSON *attribute = cJSON_GetObjectItem(root, "Attributes");
		int listNum;
		listNum = cJSON_GetArraySize(attribute);
		int i = 0;
		cJSON *item;
		cJSON *sub1;
		cJSON *sub2;
		while (i < listNum)
		{
			item = cJSON_GetArrayItem(attribute, i);
			if (item != NULL)
			{
				sub1 = cJSON_GetObjectItem(item, "Name");
				sub2 = cJSON_GetObjectItem(item, "Value");
				if(sub1 != NULL && sub2 != NULL)
				{
					Attribute *currentAttr;
					currentAttr = node->attrHead;
					while(currentAttr!=NULL)
					{
						if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_SWITCH_CHILD_LOCK) == 0 && strcmp(currentAttr->key, ATTRIBUTE_NAME_SWITCH_CHILD_LOCK) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_WINRAINSENSOR) == 0 && strcmp(currentAttr->key, ATTRIBUTE_NAME_WINRAINSENSOR) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;
						}
						else if(strcmp(sub1->valuestring, ATTRIBUTE_NAME_CO) == 0 && strcmp(currentAttr->key, ATTRIBUTE_NAME_CO) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;
						}
						else if(strcmp(sub1->valuestring, ATTRIBUTE_NAME_LED_ENABLE) == 0&& strcmp(currentAttr->key, ATTRIBUTE_NAME_LED_ENABLE) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_POWER_OFF_PROTECTION) == 0 && strcmp(currentAttr->key, ATTRIBUTE_NAME_POWER_OFF_PROTECTION) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;							
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_LIGHT_MODE) == 0 && strcmp(currentAttr->key, ATTRIBUTE_NAME_LIGHT_MODE) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_ERROR_CODE) == 0&& strcmp(currentAttr->key, ATTRIBUTE_NAME_ERROR_CODE) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
							}
							break;							
						}
						else if(strncmp(sub1->valuestring, ATTRIBUTE_NAME_VOTAGE_OVER_LOAD_PROTECTION, \
									strlen(ATTRIBUTE_NAME_VOTAGE_OVER_LOAD_PROTECTION)) == 0)
						{
							if (((strcmp(currentAttr->key, ATTRIBUTE_NAME_VOTAGE_OVER_LOAD_PROTECTION_1) == 0 || \
									strcmp(currentAttr->key, ATTRIBUTE_NAME_VOTAGE_OVER_LOAD_PROTECTION) == 0)&& \
									endpoint_id==1) || (strcmp(currentAttr->key, ATTRIBUTE_NAME_VOTAGE_OVER_LOAD_PROTECTION_2) == 0 && \
									endpoint_id==2 ))
							{

								if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
								{
									gettimeofday(&currentAttr->tv,NULL);
									sprintf(currentAttr->value, "%s", sub2->valuestring);
									reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
								}
								break;
							}
							else
								currentAttr = currentAttr->next;					
						}
						else if (strncmp(sub1->valuestring, ATTRIBUTE_NAME_OVERLOAD_PROTECTION,strlen(ATTRIBUTE_NAME_OVERLOAD_PROTECTION)) == 0)
						{
							if (((strcmp(currentAttr->key, ATTRIBUTE_NAME_OVERLOAD_PROTECTION_1) == 0 && \
									strcmp(currentAttr->key, ATTRIBUTE_NAME_OVERLOAD_PROTECTION) == 0)&& \
							endpoint_id==1 ) || (strcmp(currentAttr->key,ATTRIBUTE_NAME_OVERLOAD_PROTECTION_2) == 0\
									&& endpoint_id==2 ))
							{

								if ( strcmp(currentAttr->value,sub2->valuestring)!=0 || \
												((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
								{
									gettimeofday(&currentAttr->tv,NULL);
									sprintf(currentAttr->value, "%s", sub2->valuestring);
									reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
								}
								break;
							}
							else
								currentAttr = currentAttr->next;							
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_POWER_CONSUMPTIONCLEAR) == 0&& \
							strcmp(currentAttr->key, ATTRIBUTE_NAME_POWER_CONSUMPTIONCLEAR) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, ATTRIBUTE_NAME_POWER_CONSUMPTIONCLEAR, currentAttr->value);
							}
							break;							
						}
						else if (strcmp(sub1->valuestring,"CurtainState") == 0 && \
								strncmp(currentAttr->key, ATTRIBUTE_NAME_CURTAIN_OPERATION,16) == 0)
						{
							if ((endpoint_id==1 &&(strcmp(currentAttr->key, ATTRIBUTE_NAME_CURTAIN_OPERATION)==0 ||
									strcmp(currentAttr->key, ATTRIBUTE_NAME_CURTAIN_OPERATION_1)==0)) ||\
									(endpoint_id==2 &&strcmp(currentAttr->key, ATTRIBUTE_NAME_CURTAIN_OPERATION_2)==0 ))
							{

								int tmp=atoi(sub2->valuestring);
								char tmpValue[4]={0};
								if (tmp==2)
									sprintf(tmpValue,"%d",0);
								else if(tmp==1)
									sprintf(tmpValue,"%d",1);
								else if (tmp==0)
									sprintf(tmpValue,"%d",2);
								if ( strcmp(currentAttr->value,tmpValue)!=0 || \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
								{
									sprintf(currentAttr->value,"%s",tmpValue);
									reportFunction(REPORT_TYPE_EVENT, node->deviceId, node->modelId, currentAttr->key, tmpValue);
								}
								gettimeofday(&currentAttr->tv,NULL);

								break;
							}
							else
								currentAttr = currentAttr->next;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_ATOMIZATION) == 0)
						{
							if (((strcmp(currentAttr->key, ATTRIBUTE_NAME_ATOMIZATION_1) == 0 || strcmp(currentAttr->key, ATTRIBUTE_NAME_ATOMIZATION) == 0) && \
									endpoint_id==1 ) || (strcmp(currentAttr->key,ATTRIBUTE_NAME_ATOMIZATION_2) == 0 && \
												endpoint_id==2 ))
							{

								if ( strcmp(currentAttr->value,sub2->valuestring)!=0 || \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
								{
									gettimeofday(&currentAttr->tv,NULL);
									sprintf(currentAttr->value, "%s", sub2->valuestring);
									reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, currentAttr->key, currentAttr->value);
								}
								break;
							}
							else
								currentAttr = currentAttr->next;
						}
						else if (strcmp(sub1->valuestring,ATTRIBUTE_NAME_KEYMODE) == 0 && \
								strcmp(currentAttr->key, ATTRIBUTE_NAME_KEYMODE) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
								((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);

								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, ATTRIBUTE_NAME_KEYMODE, currentAttr->value);
							}
							break;
						}
						else if (strcmp(sub1->valuestring,ATTRIBUTE_NAME_VOLTAGE_COFFICIENT) == 0 &&\
								strcmp(currentAttr->key, ATTRIBUTE_NAME_VOLTAGE_COFFICIENT) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
											ATTRIBUTE_NAME_VOLTAGE_COFFICIENT, sub2->valuestring);
							}
							break;
						}
						else if (strcmp(sub1->valuestring,ATTRIBUTE_NAME_CURRENT_COFFICIENT) == 0 &&\
								strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_COFFICIENT) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
									ATTRIBUTE_NAME_CURRENT_COFFICIENT, sub2->valuestring);
							}
							break;
						}
						else if (strcmp(sub1->valuestring,ATTRIBUTE_NAME_POWER_COFFICIENT) == 0 &&\
							strcmp(currentAttr->key, ATTRIBUTE_NAME_POWER_COFFICIENT) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
									ATTRIBUTE_NAME_POWER_COFFICIENT, sub2->valuestring);
							}
							break;
						} 
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_POWER_WASTER_COFFICIENT) == 0 &&\
							strcmp(currentAttr->key, ATTRIBUTE_NAME_POWER_WASTER_COFFICIENT) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
									ATTRIBUTE_NAME_POWER_WASTER_COFFICIENT, sub2->valuestring);
							}
							break;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_OVER_LOAD_PROTECT_ENABLE) == 0&&\
							strcmp(currentAttr->key, ATTRIBUTE_NAME_OVER_LOAD_PROTECT_ENABLE) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
									ATTRIBUTE_NAME_OVER_LOAD_PROTECT_ENABLE, sub2->valuestring);
							}
							break;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_VOLTAGE_OVER_LOAD) == 0&&\
							strcmp(currentAttr->key, ATTRIBUTE_NAME_VOLTAGE_OVER_LOAD) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
									ATTRIBUTE_NAME_VOLTAGE_OVER_LOAD, sub2->valuestring);
							}
							break;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_CURRENT_OVER_LOAD) == 0&&\
							strcmp(currentAttr->key, ATTRIBUTE_NAME_CURRENT_OVER_LOAD) == 0)
						{
							char tmp[10]={0};
							sprintf(tmp,"%0.3f",atof(sub2->valuestring)/1000);
							if ( strcmp(currentAttr->value,tmp)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", tmp);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
									ATTRIBUTE_NAME_CURRENT_OVER_LOAD, tmp);
							}
							break;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_DCDELAY) == 0&&\
							strcmp(currentAttr->key, ATTRIBUTE_NAME_DCDELAY) == 0)
						{
							if ( strcmp(currentAttr->value,sub2->valuestring)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", sub2->valuestring);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
									ATTRIBUTE_NAME_DCDELAY, sub2->valuestring);
							}
							break;
						}
						else if (strcmp(sub1->valuestring, ATTRIBUTE_NAME_LOCAL_CONFIG) == 0&&\
							strcmp(currentAttr->key, ATTRIBUTE_NAME_LOCAL_CONFIG) == 0)
						{
							unsigned char tmpvalue=(unsigned char)atoi(sub2->valuestring);
							int realValue=(tmpvalue>>4)&0x01;
							char strValue[2]={0};
							sprintf(strValue,"%d",realValue);

							if ( strcmp(currentAttr->value,strValue)!=0|| \
									((now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME && (currentAttr->tv.tv_sec==0||repeatReport==1)))
							{
								gettimeofday(&currentAttr->tv,NULL);
								sprintf(currentAttr->value, "%s", strValue);
								reportFunction(REPORT_TYPE_ATTRIBUTE, node->deviceId, node->modelId, \
									ATTRIBUTE_NAME_LOCAL_CONFIG, strValue);
							}
							break;
						}
						else
							currentAttr=currentAttr->next;
					}
				}
			}
			i++;
		}
		break;
	}					
	case 0X02F1:
		break;
	default:
		break;
	}
}

int sendCommandWriteCooUpgradeInfo(char *fileLength,char *version)
{
	int res;
	cJSON *root, *writeParam;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Action", "W");

	cJSON_AddItemToObject(root, "WriteParameter", writeParam = cJSON_CreateObject());
	cJSON_AddStringToObject(writeParam, "ManufactureCode", "4451");
	cJSON_AddStringToObject(writeParam, "ImageType", "1");
	cJSON_AddStringToObject(writeParam, "ImageVersion", version);
	cJSON_AddStringToObject(writeParam, "ImageLength", fileLength);
	cJSON_AddStringToObject(writeParam, "ReceivedLength", "0");
	cJSON_AddStringToObject(writeParam, "UpgradeType", "1");
	char *jsonString;
	jsonString = cJSON_Print(root);
	INFO("[ZIGBEE] Coo firmware informtion:\n");
	INFO("[ZIGBEE] %s\n\n", jsonString);

	res = rex_send_command_data("0000000000000000", 1, COMMAND_TYPE_DEVICE_OTA, jsonString);
	cJSON_Delete(root);
	if (jsonString)
	{
		free(jsonString);
	}

	if (res != 0)
	{
		WARN("[ZIGBEE] Send write Coo firmware informatoin command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIBGEE] Send coo upgrade command success\n\n");
		init_gpio_coo_update();
		return 0;
	}
}

int upgradeCoofirmware()
{
	DIR* dirptr;
	struct dirent* entry;
	unsigned long filesize = 0;
	char version[10]={0};


	if ((dirptr = opendir(fileDownloadPath)) == NULL)
	{
		ERROR("[ZIGBEE] Open coo download path failed: %s\n", fileDownloadPath);
		return 0;
	}

	for (entry = readdir(dirptr); NULL != entry; entry = readdir(dirptr))
	{

		if (strstr(entry->d_name, "4451") != NULL)
		{

			struct stat statbuff;

			memcpy(version,entry->d_name+7,8);
			sleep(10);
			sprintf(cooFirmwarePath, "%s%s", fileDownloadPath, entry->d_name);
			if (stat(cooFirmwarePath, &statbuff) != -1)
			{
				filesize = statbuff.st_size;
			}
			break;
		}
	}

	if (filesize != 0)
	{

		char fileOfSize[12];
		sprintf(fileOfSize, "%lu", filesize);
		sendCommandWriteCooUpgradeInfo(fileOfSize,version);
	}
	closedir(dirptr);
	return 0;
}

typedef struct Zibgee_Data
{
	char address[20];
	unsigned char endpoint_id;
	unsigned short stateType;
	char state[2000];
	struct Zibgee_Data* next;
}ZigeeData;

static ZigeeData *zigbeeDataHead=NULL;
static ZigeeData *zigbeeDataTail=NULL;
pthread_mutex_t mutex_device_state_lock;
int initDeviceStateReportLock()
{
	pthread_mutex_init(&mutex_device_state_lock,NULL);
	return 0;
}

int report_state_data_cb(char *address, unsigned char endpoint_id, unsigned short stateType, char *state)
{
	ZigeeData *newData;
	newData=(ZigeeData *)malloc(sizeof(ZigeeData));
	memset(newData,0,sizeof(ZigeeData));
	sprintf(newData->address,"%s",address);
	newData->endpoint_id=endpoint_id;
	newData->stateType=stateType;
	sprintf(newData->state,"%s",state);


	pthread_mutex_lock(&mutex_device_state_lock);
	if (zigbeeDataHead==NULL && zigbeeDataTail==NULL)
	{
		zigbeeDataHead=newData;
		zigbeeDataTail=newData;
	}
	else
	{
		zigbeeDataTail->next=newData;
		zigbeeDataTail=newData;
	}
	pthread_mutex_unlock(&mutex_device_state_lock);

	return 0;
}

int thread_delayQueryDeviceState(void *deviceId)
{
	char tmp[20]={0};
	sprintf(tmp,"%s",(char *)deviceId);
	sleep(5);
	return sendCommandGetDeviceState(tmp, 1);
}



#define DEVICE_MAC_MAX_LENGTH		(20)
int handle_report_state_data(char *address, unsigned char endpoint_id, unsigned short stateType, char *state)
{
	INFO("[ZIBGEE]\033[40;33m[REX_REPORT]\033[0m Device report: deviceId=%s, endpoint=%u, stateType=%04X\n", address, endpoint_id, stateType);
	INFO("[ZIBGEE]\033[40;33m[REX_REPORT]\033[0m Device report: %s\n\n", state);

	cJSON *root = NULL;
	root = cJSON_Parse(state);

	if (root == NULL)
	{
		ERROR("[ZIGBEE] Device report: parse json string failed!\n\n");
		return -1;
	}

	switch (stateType)
	{
	case 0x0200:
	{
		#ifdef BOARD_U86_GATEWAY
		int iZigbeeNetFlag = 1;
		#endif
		
		cJSON *channel;
		channel = cJSON_GetObjectItem(root, "Channel");
		if (channel)
		{
			sprintf(cooChannel, "%s", channel->valuestring);
			
			#ifdef BOARD_U86_GATEWAY
			if(!strcmp(cooChannel, ""))
			{
				iZigbeeNetFlag = 0;
			}
			#endif
		}
		cJSON *extpanid;
		extpanid = cJSON_GetObjectItem(root, "ExtPanId");
		if (extpanid)
		{
			sprintf(cooExtPanId, "%s", extpanid->valuestring);

			#ifdef BOARD_U86_GATEWAY
			if(!strcmp(cooExtPanId, ""))
			{
				iZigbeeNetFlag = 0;
			}
			#endif
		}

		cJSON *mac;
		mac = cJSON_GetObjectItem(root, "CooAddress");
		if (mac)
		{
			sprintf(cooMac, "%s", mac->valuestring);
			cJSON_AddStringToObject(root,"CooMac",mac->valuestring);

			#ifdef BOARD_U86_GATEWAY
			if(!strcmp(cooMac, ""))
			{
				iZigbeeNetFlag = 0;
			}
			#endif
		}
		cJSON_DeleteItemFromObject(root,"CooAddress");

		cJSON *panid;
		panid = cJSON_GetObjectItem(root, "PanId");
		if (panid)
		{
			sprintf(cooPanId, "%s", panid->valuestring);

			#ifdef BOARD_U86_GATEWAY
			if(!strcmp(cooPanId, ""))
			{
				iZigbeeNetFlag = 0;
			}
			#endif
		}

		cJSON *network;
		network = cJSON_GetObjectItem(root, "NetWorkKey");
		if (network)
		{
			sprintf(cooNetWorkKey, "%s", network->valuestring);

			#ifdef BOARD_U86_GATEWAY
			if(!strcmp(cooNetWorkKey, ""))
			{
				iZigbeeNetFlag = 0;
			}
			#endif
		}
		
		char *jsonstring;
		jsonstring=cJSON_Print(root);
		char outBuff[1024];
		packageReportMessageToJson(REPORT_TYPE_COO_INFO, (void*)jsonstring, outBuff);
		messageReportFun(outBuff);

		cJSON_Delete(root);
		if (jsonstring)
		{
			free(jsonstring);
		}

		#ifdef BOARD_U86_GATEWAY
		/*U86 控制ZIGBEE led*/
		if(iZigbeeNetFlag)
		{
			set_zigbee_led_flag(2);
		}
		#endif
		
		return 0;
		break;
	}
	case 0x020C:
	{
		char outBuff[1024]={0};
		packGatewaySignatureInfoReport(state,outBuff);
		messageReportFun(outBuff);
		cJSON_Delete(root);
		return 0;
		break;
	}
	case 0x0214:
	{
		cJSON *response;
		cJSON *status;
		cJSON *wp = cJSON_GetObjectItem(root, "WriteResponse");
		if(wp!=NULL)
		{
			char outbuff[500]={0};
			char modelId[20]={0};
			getDevModelId(address, outbuff);
			if(1==atoi(wp->valuestring))
				packageDeviceOtaResult(address,modelId,"0",outbuff);
			else
				packageDeviceOtaResult(address,modelId,"102",outbuff);
			messageReportFun(outbuff);
		}
		response = cJSON_GetObjectItem(root, "UpgradeResponse");
		if (response != NULL)
		{
			char outbuff[500]={0};
			status = cJSON_GetObjectItem(response, "Status");
			if (status != NULL)
			{
				int num;
				num = atoi(status->valuestring);
				switch (num)
				{
				case 1:
					break;
				case 2:
				case 3:
				{
					if (strcmp(address,"0000000000000000")!=0)
					{
						char modelId[20]={0};
						getDevModelId(address, outbuff);
						packageDeviceOtaResult(address,modelId,"101",outbuff);
						messageReportFun(outbuff);
						break;
					}
				}
				case 4:
				case 5:
				case 6:
				case 7:
				{
					if (strcmp(address,cooMac)!=0)
					{
						char modelId[20]={0};
						getDevModelId(address, outbuff);
						packageDeviceOtaResult(address,modelId,"102",outbuff);
						messageReportFun(outbuff);
					}
					else
					{
						closeCooLed();
						cooUpdateEnd = 0;

						int res;
						pthread_attr_t attr;
						pthread_attr_init(&attr);
						pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
						pthread_t pCooLed;
						res = pthread_create(&pCooLed, &attr, (void *)upgradeCoofirmware, NULL);
						pthread_attr_destroy(&attr);
						if (res != 0)
						{
							ERROR("[ZIBGEE] Open call upgradeCoofirmware thread error\n\n");
						}
					}
					break;
				}
				case 8:
				{
					if (strcmp(address,cooMac)!=0)
					{
						char modelId[20]={0};
						getDevModelId(address, outbuff);
						packageDeviceOtaResult(address,modelId,"1",outbuff);
						messageReportFun(outbuff);
						char command[200];
						memset(command, '\0', 200);
						sprintf(command, "rm %s", tmp_devFilePath);

						hy_system(command,NULL,0);
						//system(command);
					}
					else
					{
						closeCooLed();
						cooUpdateEnd = 1;
						char command[200];
						memset(command, '\0', 200);
						sprintf(command, "rm -f %s", cooFirmwarePath);
						hy_system(command,NULL,0);
						sendCommandSetAtHoldRPTOD();
					}
					break;
				}
				default:
					break;
				}
			}
		}
		cJSON_Delete(root);
		return 0;
		break;
	}
	case 0x020B:
	{
		if (cooUpdateEnd == 1)
		{
			char command[200];
			memset(command, '\0', 200);
			sprintf(command, "rm -f %s", cooFirmwarePath);
			hy_system(command,NULL,0);
		}
		cJSON *cooVerCode=cJSON_GetObjectItem(root,"VersionCode");
		if (cooVerCode==NULL)
		{
			cJSON_Delete(root);
			return 0;
		}
		sprintf(cooCode,"%s",cooVerCode->valuestring);

		cJSON *cooVer=cJSON_GetObjectItem(root,"Version");
		if (cooVer==NULL)
		{
			cJSON_Delete(root);
			return 0;
		}


		char *p1;
		char *p2;
		p1=strchr(cooVer->valuestring,':');
		p2=strchr(p1,',');
		snprintf(cooVersion,p2-p1,"%s",p1+1);
		cJSON_Delete(root);


		return 0;
		break;
	}
	case 0x0211:
	{
		cJSON_AddStringToObject(root, "DeviceId", address);
		char *jsonstring;
		jsonstring = cJSON_Print(root);
		char outBuff[2048];
		packageReportMessageToJson(REPORT_TYPE_NEIGHBOR_INFO, (void*)jsonstring, outBuff);
		messageReportFun(outBuff);
		cJSON_Delete(root);
		if (jsonstring)
		{
			free(jsonstring);
		}
		return 0;
		break;
	}
	case 0x0213:
	{
		cJSON_AddStringToObject(root, "DeviceId", address);
		char *jsonstring;
		jsonstring = cJSON_Print(root);
		char outBuff[2048];
		packageReportMessageToJson(REPORT_TYPE_CHILDREN_INFO, (void*)jsonstring, outBuff);
		messageReportFun(outBuff);
		cJSON_Delete(root);
		if (jsonstring)
		{
			free(jsonstring);
		}
		return 0;
		break;
	}
	case 0x02F6:
	{
		cJSON_Delete(root);
		dealFastJoinResultReport(state);
		return 0;
		break;
	}
	case 0x02F3:
	{
		char tmp[300]={0};
		unpackAndReportSetSignaturReport(root,tmp);
		messageReportFun(tmp);
		cJSON_Delete(root);
		return 0;
		break;
	}
	case 0X020A:
	{
		cJSON *sdkVer=cJSON_GetObjectItem(root,"Version");
		if (sdkVer!=NULL)
		{
			sprintf(rexSdkVer,"%s",sdkVer->valuestring);
		}
		cJSON_Delete(root);
		return 0;
		break;
	}
	case 0x020E:
	{
		
		cJSON *ModelId=cJSON_GetObjectItem(root,"ModelId");
		cJSON *manufacturer=cJSON_GetObjectItem(root,"Manufacturer");

		char realModel[DEVICE_MODEL_ID_LENGTH]={0};

		if (ModelId!=NULL)
		{
			if(1==isTYDeviceFirmware(ModelId->valuestring))
			{
				if(NULL!=manufacturer)
					sprintf(realModel,manufacturer->valuestring);
				else
					break;
			}
			else
			{
				if(strlen(ModelId->valuestring)==8 && ModelId->valuestring[0]=='0'&& ModelId->valuestring[1]=='0')
					sprintf(realModel,"%s",ModelId->valuestring+2);
				else
					sprintf(realModel,ModelId->valuestring);
			}

			int find = 0;

			pthread_mutex_lock(&mutexLinkTable);

			DevInfo *pCurrentNode = pHead;
			while (pCurrentNode != NULL)
			{
				if (strncmp(pCurrentNode->deviceId, address, 16) == 0)
				{
					find=1;
					break;
				}
				else
					pCurrentNode=pCurrentNode->next;
			}

			if (find==0 && 0!=get_led_ctrl_flag())
			{
				DevInfo *newNode=NULL;
				newNode = ZHA_creatDevInfoStruct(realModel);
				if (newNode == NULL)
				{
					pthread_mutex_unlock(&mutexLinkTable);
					char buff[300];
					packageUnknowDeviceInfo(address, realModel,"???",buff);
					messageReportFun(buff);
				}
				else
				{
					sendCommandGetDevVersion(address);
					sprintf(newNode->realModelId,"%s",realModel);
					sprintf(lastJoinDevice,"%s",address);
					DEBUG("lastJoinDevice=%s\n",lastJoinDevice);
					sprintf(newNode->deviceId, "%s", address);
					sprintf(newNode->onlineInfo->deviceId, "%s", address);
					newNode->onlineInfo->online = 1;

					devcieNumber++;
					if (pHead == NULL)
					{
						pHead = newNode;
						pTail = newNode;
					}
					else
					{
						pTail->next = newNode;
						pTail = newNode;
					}
					gettimeofday(&tm,NULL);
					pthread_mutex_unlock(&mutexLinkTable);

					pthread_mutex_lock(&mutexOnlineTable);
					if (pOnlineHead == NULL && pOnlineTail == NULL)
					{
						pOnlineHead = newNode->onlineInfo;
						pOnlineTail = newNode->onlineInfo;
					}
					else
					{
						pOnlineTail->next = newNode->onlineInfo;
						pOnlineTail = newNode->onlineInfo;
					}
					pthread_mutex_unlock(&mutexOnlineTable);
					ReportRegister reportReg;
					memset(&reportReg,0,sizeof(ReportRegister));
					sprintf(reportReg.deviceId, "%s", address);
					sprintf(reportReg.modelId, newNode->modelId);
					sprintf(reportReg.secret, "%s", newNode->secret);
					sprintf(reportReg.realModelId,"%s",newNode->realModelId);

					sprintf(reportReg.subDevType, "%d", newNode->subDevType);//hex2dec(type);
					reportReg.next = NULL;

					char outBuff[400];
					packageReportMessageToJson(REPORT_TYPE_REGISTER, (void*)&reportReg, outBuff);

					int rt = 0;
					/*上报设备入网信息*/
					rt = messageReportFun(outBuff);


					int regFlag = 1;
					if (rt == -1) //注册失败删除
					{
						newNode->registerStatus = 0;
						WARN("[ZIGBEE] New device register failed,deviceId:%s, modelId:%s, type:%d\n\n", address, newNode->modelId, newNode->subDevType);
						regFlag = 0;
					}
					else//注册成功
					{
						newNode->registerStatus = 1;
						INFO("[ZIGBEE] New device register success,deviceId:%s, modelId:%s, type:%d,\n\n", address,newNode->modelId,newNode->subDevType);
					}
					int res = 0;
					char sqlStr[SQLITE_COMMAND_MAX_LENGTH];

					sprintf(sqlStr, "REPLACE INTO Table_device_list (deviceId,subType,name,modelId,hardwareVersion,\
					softwareVersion,registerStatus,secret,reportTime) VALUES (\'%s\',%d,\'%s\',\'%s\',\'%s\',\'%s\',%d,\'%s\',\'%d\');", \
					address, newNode->subDevType, newNode->name, realModel, "1.0.0", "20180101", regFlag, newNode->secret,newNode->onlineInfo->reportTime);


					res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
					if (res != SQLITE_OK)
					{
						ERROR("[ZIGBEE] inserter new device %s information into database failed: %s\n\n", address, zErrMsg);
						sqlite3_free(zErrMsg);
					}
					gettimeofday(&tm,NULL);
				}
				
			}
			else
				pthread_mutex_unlock(&mutexLinkTable);
			
		}
		break;
	}

	default:
		break;
	}


	pthread_mutex_lock(&mutexLinkTable);
	DevInfo *pCurrentNode = pHead;
	if (pCurrentNode == NULL)
	{
		pthread_mutex_unlock(&mutexLinkTable);
		WARN("[ZIBGEE] Matching deviceID %s, but device list is empty!\n\n",address);
		cJSON_Delete(root);
		return -1;
	}

	while (pCurrentNode != NULL)
	{
		if (strncmp(pCurrentNode->deviceId, address, 16) == 0)
		{
			pthread_mutex_unlock(&mutexLinkTable);
			gettimeofday(&pCurrentNode->onlineInfo->lastUpdateTime, NULL);
			if (pCurrentNode->onlineInfo->online == 0)
			{
				pCurrentNode->onlineInfo->online = 1;
				pCurrentNode->onlineInfo->offCount = 0;
				char outBuff[400];
				ReportAttrEventOnline online;
				sprintf(online.deviceId, "%s", pCurrentNode->deviceId);
				sprintf(online.modelId, "%s", pCurrentNode->modelId);
				sprintf(online.key, "%s", "Online");
				sprintf(online.value, "%s", "1");
				online.next = NULL;
				packageReportMessageToJson(REPORT_TYPE_ONOFF, (void*)&online, outBuff);

				/*此处插入上报函数*/
				messageReportFun(outBuff);

				if(pCurrentNode->subDevType==0x1204)
				{
					pthread_attr_t attr;
					pthread_attr_init (&attr);
					pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
					pthread_t pCallDevResponse;
					pthread_create(&pCallDevResponse,  &attr, (void *)thread_delayQueryDeviceState, (void*)pCurrentNode->deviceId);
					pthread_attr_destroy (&attr);
				}
			}
			pCurrentNode->onlineInfo->offCount = 0;

			pthread_mutex_lock(&mutexLinkTable);
			/*解析并上报*/
			parseDeviceReportMessage(pCurrentNode, endpoint_id, stateType, root);

			pthread_mutex_unlock(&mutexLinkTable);
			cJSON_Delete(root);
			return 0;
		}
		else
		{
			pCurrentNode = pCurrentNode->next;
		}
	}
	pthread_mutex_unlock(&mutexLinkTable);
	WARN("[ZIGBEE] Matching deviceID %s, but this device does not join the gateway \n\n", address);
	cJSON_Delete(root);
	return -1;
}

int thread_handle_message()
{
	ZigeeData *deleteNode=NULL;
	initDeviceStateReportLock();
	while(1)
	{

		pthread_mutex_lock(&mutex_device_state_lock);
		if (NULL!=zigbeeDataHead)
		{

			deleteNode=zigbeeDataHead;
			if (zigbeeDataTail==zigbeeDataHead)
				zigbeeDataTail=zigbeeDataHead->next;
			zigbeeDataHead=zigbeeDataHead->next;

			pthread_mutex_unlock(&mutex_device_state_lock);


			handle_report_state_data(deleteNode->address,deleteNode->endpoint_id,\
					deleteNode->stateType,deleteNode->state);
			free(deleteNode);
			deleteNode=NULL;
			continue;
		}


		pthread_mutex_unlock(&mutex_device_state_lock);
		usleep(200000);

	}
}

/*删除设备属性链表*/
void deleteDevAttrList(Attribute *attrHead)
{
	Attribute *currentNode;
	Attribute *deleteNode;

	currentNode = attrHead;
	attrHead = NULL;
	while (currentNode != NULL)
	{
		deleteNode = currentNode;
		currentNode = currentNode->next;
		free(deleteNode);
	}
}

/*删除设备信息链表*/
void deleteDeviceList()
{
	DevInfo *pCurrentNode;
	DevInfo *pDelteNode;

	pthread_mutex_lock(&mutexLinkTable);
	pCurrentNode = pHead;
	devcieNumber = 0;
	pHead = NULL;
	pTail = NULL;
	pthread_mutex_unlock(&mutexLinkTable);

	while (pCurrentNode != NULL)
	{
		pDelteNode = pCurrentNode;
		pCurrentNode = pCurrentNode->next;
		deleteDevAttrList(pDelteNode->attrHead);
		pDelteNode->onlineInfo->leaveGateway = 0;
		pDelteNode->onlineInfo = NULL;
		free(pDelteNode);
	}
}

int cleardevManageDatabase()
{
	int res=0;
	char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
	sprintf(sqlStr, "DELETE FROM Table_device_list;");
	res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIBGEE] clear all devices information in database failed  %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	else
	{
		deleteDeviceList();
	}

	groupClearList();
	bindClearList();

	return 0;
}


/*恢复出厂设置，上报成功后，清除数据库设备信息，重启*/
int sendCommandResetFactorySetting()
{
	int res;
	res = rex_send_command_data("0000000000000000", 1, COMMAND_TYPE_FACTORY_RESET, "{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] send factory reset command failed\n\n");
		return -1;
	}
	else
	{
#ifdef MULTI_SWITCH_SYNC
		deleteAllSwitchGroupFun();
#endif
		if(cleardevManageDatabase()==0)
		{
			init_gpio_refactory();
			INFO("[ZIGBEE] Factory reset success!\n\n");
			char outBuff[400];
			packageReportMessageToJson(REPORT_TYPE_REFACTORY, NULL, outBuff);
			int rt;
			rt = messageReportFun(outBuff);
			return rt;
		}
		else
			return -1;
	}

}

int allowDeviceJoinGateway(char* time)
{
	if(0!=get_led_ctrl_flag())
	{
		set_led_ctrl_flag(0);
		sleep(1);
	}
	int res = 0;
	res = sendCommandAllowDevJoin(time);
	if (res != 0)
	{
		WARN("[ZIGBEE] send command of allowing device to join the zigbee network failed\n\n");
	}
	else
	{
		INFO("[ZIGBEE] Send the command of allowing to join the zigbee network success!\n\n");

		int ledTime;
		ledTime = atoi(time);

		if (ledTime != 0)
		{
			reportFunction("Attribute",GW_GATEWAY_DEVICE_ID,GW_GATEWAY_MODEL_ID,"PermitJoining","1");

			pthread_attr_t attr;
			pthread_attr_init (&attr);
			pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

			pthread_t ledPthread;
			pthread_create(&ledPthread, &attr, (void*)init_gpio_app, (void*)&ledTime);
			pthread_attr_destroy (&attr);

			sleep(1);
		}
		else
		{
			if(0!=get_led_ctrl_flag())
				set_led_ctrl_flag(0);
			else
				reportFunction("Attribute",GW_GATEWAY_DEVICE_ID,GW_GATEWAY_MODEL_ID,"PermitJoining","0");
		}

	}
	return res;
}

static int currentTime = 0;
static int getEndDownTime = 0;
static int getStartDownTime = 0;
static int keyStatus;
static int keyDownCount = 0;
static int reset_key_status_bef = 0;
static int resetFlag = 1;/*是否复位标志符，1-还原出厂设置清空数据库，0不做任何处理*/
static int allowJoinZigbeeDevFlag = 1;

int resetButtenProcess(char *event)
{
	cJSON *root;
	root = cJSON_Parse(event);
	if (root != NULL)
	{
		cJSON *key;
		key = cJSON_GetObjectItem(root, "GatewayKey");

		if (strncmp(key->valuestring, "1", 1) == 0)
			keyStatus = 1;
		else
			keyStatus = 0;
		cJSON_Delete(root);

		currentTime = time(NULL);

		if (keyStatus != reset_key_status_bef)
		{
			if (keyStatus == 1)/*有按键按下*/
			{
				if ((0 != getEndDownTime) && (currentTime - getEndDownTime) >= 4)
				{
					keyDownCount = 0;
				}
				/*按键按下*/
				keyDownCount++;
				INFO("[ZIGBEE] Reset key down, %d times\n\n", keyDownCount);
				if (keyDownCount == 1)
					getStartDownTime = currentTime;
			}
			else
			{
				if (keyDownCount == 1)/*按键按下放开一次*/
				{
					INFO("[ZIGBEE] Reset key up, %d times\n\n", keyDownCount);
					if ((currentTime - getStartDownTime) >= 10  && (currentTime - getStartDownTime) < 20)
					{
						INFO("[Debug] press reset button time  10s<t<20s\n\n");
						if (1 == resetFlag)/*恢复出厂设置*/
						{
							if (sendCommandResetFactorySetting() == -1)
							{
								return -1;
							}

						}
					}
					else if ((currentTime - getStartDownTime) >= 20 )
					{
						INFO("[Debug] press reset button time>20s\n\n"); //恢复为默认web界面参数
						init_gpio_refactory();
						char outbuff[100]={0};
						packageNeworkReset(outbuff);
						messageReportFun(outbuff);
					}
					else if ((currentTime - getStartDownTime) >= 5 && (currentTime - getStartDownTime) < 10)
					{
						INFO("[ZIGBEE] press reset button 5s<time <10s\n\n");
						init_gpio_8s_butten();
						resetButen8sEventFun();
					}

				}
				else
				{
					if ((currentTime - getStartDownTime) <= 3)
					{
						if (keyDownCount >= 3)/*允许加入网络*/
						{
							if (1 == allowJoinZigbeeDevFlag)
							{
								allowDeviceJoinGateway("120");
							}
							keyDownCount = 0;
						}
					}
					else
					{
						if (keyDownCount > 1)
						{
							keyDownCount = 0;
						}
					}
				}
				getEndDownTime = currentTime;
			}
			reset_key_status_bef = keyStatus;
		}

		if ((0 != keyDownCount) && (0 == keyStatus))
		{
			currentTime = time(NULL);
			if ((currentTime - getStartDownTime) >= 8)
			{
				keyDownCount = 0;
			}
		}
	}
	return 0;
}

int report_event_data_cb(char *address, unsigned char endpoint_id, unsigned short event_type, char *event)
{
	INFO("[ZIGBEE]\033[40;33m[REX_REPORT]\033[0m Event report: address=%s, endpoint=%u\n", address, endpoint_id);
	INFO("[ZIGBEE]\033[40;33m[REX_REPORT]\033[0m Event report: %s\n", event);
	INFO("[ZIGBEE]\033[40;33m[REX_REPORT]\033[0m Event type: %04X\n\n", event_type);

	int res = 0;
	if (event_type == 0x03A0)
	{
		res = resetButtenProcess(event);
		return res;
	}
	else if (event_type == 0x0370)
	{
		deviceEventJoin(address, event);
		return 0;
	}
	DevInfo *currentNode;
	int find = 0;

	pthread_mutex_lock(&mutexLinkTable);
	currentNode = pHead;
	while (currentNode != NULL)
	{
		if (strncmp(address, currentNode->deviceId, 16) == 0)
		{
			find = 1;
			if (currentNode->onlineInfo->online == 0)
			{
				currentNode->onlineInfo->offCount = 0;
				currentNode->onlineInfo->leaveGateway = 1;
				currentNode->onlineInfo->online = 1;
				reportFunction(REPORT_TYPE_ONOFF, currentNode->deviceId, currentNode->modelId, "Online", "1");

			}
			gettimeofday(&currentNode->onlineInfo->lastUpdateTime, NULL);
			break;
		}
		else
		{
			currentNode = currentNode->next;
		}
	}
	pthread_mutex_unlock(&mutexLinkTable);

	if (find == 1)
	{
		struct timeval now;
		gettimeofday(&now,NULL);

		cJSON *root;
		root = cJSON_Parse(event);
		cJSON *subItem;

		Attribute *protectAttr;
		protectAttr = currentNode->attrHead;
		int isreport = 1;//report=1,no report=0;
		while (protectAttr != NULL)
		{
			if (strcmp(protectAttr->key, "ProtectionStatus") == 0)
			{
				if (strncmp(protectAttr->value, "0", 1) == 0)
				{
					isreport = 0;
				}
				break;
			}
			else
				protectAttr = protectAttr->next;
		}

		switch (event_type)
		{
		case 0x03A3:/*安防传感器状态1报警*/
		{
			Attribute *currentAttr;
			cJSON *alarmState1 = cJSON_GetObjectItem(root, "AlarmState");
			currentAttr = currentNode->attrHead;

			if (alarmState1 != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_SMOKE_ALARM) == 0 \
							||( endpoint_id==1 && strcmp(currentAttr->key, ATTRIBUTE_NAME_GAS_ALARM) == 0) \
							|| ( endpoint_id==2 && strcmp(currentAttr->key, ATTRIBUTE_NAME_CO_ALARM) == 0)\
							|| strcmp(currentAttr->key, ATTRIBUTE_NAME_CONTACT_ALARM) == 0 \
							|| strcmp(currentAttr->key, ATTRIBUTE_NAME_MOTION_ALARM) == 0 \
							|| strcmp(currentAttr->key, ATTRIBUTE_NAME_MOTION_CURTAIN_ALARM) == 0\
							|| strcmp(currentAttr->key, ATTRIBUTE_NAME_WATER_ALARM) == 0\
							|| strcmp(currentAttr->key,ATTRIBUTE_NAME_DIRECTION_ALARM)==0\
							|| strcmp(currentAttr->key,ATTRIBUTE_NAME_KEY_FOB_VALUE)==0)
					{
						if (strcmp(currentAttr->value,alarmState1->valuestring)!=0||(now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME)
						{


							if((strcmp(currentNode->modelId,"4a0e0d")==0 || strcmp(currentNode->modelId,"RH6010")==0))
							{
								if(strcmp(alarmState1->valuestring,"1")==0)
									sprintf(currentAttr->value, "%s", "4");
								else
									break;
							}
							else
								sprintf(currentAttr->value, "%s", alarmState1->valuestring);

							if (isreport == 1 && isTwoInfraredCurtainSensor(currentNode->modelId,currentAttr->value)>0)
							{
								gettimeofday(&currentAttr->tv,NULL);
								reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId,currentAttr->key, currentAttr->value);

							}
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
			break;
		}
		case 0x03A4:/*安防传感器状态2报警*/
		{
			Attribute *currentAttr;
			cJSON *alarmState2 = cJSON_GetObjectItem(root, "AlarmState");
			currentAttr = currentNode->attrHead;
			if (alarmState2 != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key,ATTRIBUTE_NAME_DIRECTION_ALARM)==0)
					{

						if (strcmp(currentAttr->value,alarmState2->valuestring)!=0||(now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME)
						{


							if (1==isTwoInfraredCurtainSensor(currentNode->modelId,alarmState2->valuestring))
								sprintf(currentAttr->value, "%s", "2");
							else
								sprintf(currentAttr->value, "%s", alarmState2->valuestring);
							if (isreport == 1 && isTwoInfraredCurtainSensor(currentNode->modelId,alarmState2->valuestring)>0)
							{
								gettimeofday(&currentAttr->tv,NULL);
								reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId,currentAttr->key, currentAttr->value);

							}
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
			break;
		}
		case 0x03A5:/*防撬开关报警*/
		{
			Attribute *currentAttr;
			cJSON *tamper = cJSON_GetObjectItem(root, "AlarmState");
			currentAttr = currentNode->attrHead;
			if (tamper != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key,ATTRIBUTE_NAME_TAMPER_ALARM)==0)
					{
						if (strcmp(currentAttr->value,tamper->valuestring)!=0||(now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME)
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", tamper->valuestring);
							if(isreport==1)
								reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId,currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}
			break;
		}
		case 0x0309: /*低电压报警事件*/
		{
			Attribute *currentAttr;
			cJSON *lowBatteryAlarm = cJSON_GetObjectItem(root, "AlarmState");
			currentAttr = currentNode->attrHead;
			if (lowBatteryAlarm != NULL)
			{
				while (currentAttr != NULL)
				{
					if (strcmp(currentAttr->key, ATTRIBUTE_NAME_LOW_BARRERY_ALARM) == 0)
					{
						if (strcmp(currentAttr->value,lowBatteryAlarm->valuestring)!=0||(now.tv_sec-currentAttr->tv.tv_sec)>DELAY_TIME)
						{
							gettimeofday(&currentAttr->tv,NULL);
							sprintf(currentAttr->value, "%s", lowBatteryAlarm->valuestring);
							reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId,currentAttr->key, currentAttr->value);
						}
						break;
					}
					else
					{
						currentAttr = currentAttr->next;
					}
				}
			}

			break;
		}
		case 0x0300: /*锁具解锁事件*/
		{

			subItem = cJSON_GetObjectItem(root, "Source");
			cJSON *userId = cJSON_GetObjectItem(root, "UserId");

			if (subItem != NULL && userId != NULL)
			{
				int type = 0;
				int userType = 0;
				type = atoi(subItem->valuestring);
				switch (type)
				{
				case 0:
					userType = 2;
					break;
				case 1:
					userType = 3;
					break;
				case 2:
					userType = 4;
					break;
				case 3:
					userType = 3;
					break;
				case 4:
					userType = 1;
					break;
				default:
					WARN("[ZIGBEE] unknown user type, type=%d\n", type);
				}
				char reportValue[100];
				sprintf(reportValue, "{\"UserId\":\"%s\",\"UserType\":\"%d\"}", userId->valuestring, userType);
				reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, "OpenDoor", reportValue);
				sprintf(reportValue, "%d", userType);
				reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, "lockOpenRecord_UserType", reportValue);
			}

			break;
		}
		case 0x0301: /*锁具试开事件*/
		{

			reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, "LockAlarm", "5");
			break;
		}
		case 0x0303: /*锁具用户管理*/
		{

			subItem = cJSON_GetObjectItem(root, "Action");
			cJSON *source = cJSON_GetObjectItem(root, "Source");
			cJSON *userId = cJSON_GetObjectItem(root, "UserId");

			if (subItem != NULL && source != NULL && userId != NULL)
			{
				int type = 7;
				int userType = 0;
				type = atoi(source->valuestring);
				switch (type)
				{
				case 0:
					userType = 2;
					break;
				case 1:
					userType = 3;
					break;
				case 2:
					userType = 4;
					break;
				case 3:
					userType = 3;
					break;
				case 4:
					userType = 1;
					break;
				default:
					WARN("[ZIGBEE] unknown user type, type=%d\n", type);
				}

				char command[12] = { 0 };

				if (strcmp(subItem->valuestring, "A") == 0)
				{
					sprintf(command, "%s", "UserAdd");
				}
				else if (strcmp(subItem->valuestring, "D") == 0)
				{
					sprintf(command, "%s", "UserDelete");
				}
				char reportValue[100];
				sprintf(reportValue, "{\"UserId\":\"%s\",\"UserType\":\"%d\"}", userId->valuestring, userType);
				reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, command, reportValue);
			}

			break;
		}
		case 0x0304:/*锁具报警事件*/
		{
			subItem = cJSON_GetObjectItem(root, "AlarmCode");
			if (subItem != NULL)
			{
				int num = -1;
				num = atoi(subItem->valuestring);
				char reportValue[3];

				if (num == 0)
				{
					sprintf(reportValue, "%d", 1);
				}
				else if (num == 1)
				{
					sprintf(reportValue, "%d", 1);
				}
				else if (num == 4)
				{
					sprintf(reportValue, "%d", 5);
				}
				else if (num == 10)
				{
					sprintf(reportValue, "%d", 6);
				}
				else if (num == 12 || num==38)
				{
					sprintf(reportValue, "%d", 10);
				}
				else if (num == 15 || num==39)
				{
					sprintf(reportValue, "%d", 2);
				}
				else
				{
					num = -1;
				}
				if (num != -1)
				{
					reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, "LockAlarm", reportValue);

				}

			}

			break;
		}
		case 0x0306:/*安防遥控器按键事件*/
		{
			switch (currentNode->subDevType)
			{
			case REX_DEV_TYPE_HUMMAN_BODY_INDUCTION:
			{
				cJSON *command = cJSON_GetObjectItem(root, "Command");
				if (command != NULL)
				{
					Attribute *currentAttr;
					currentAttr = currentNode->attrHead;
					while (currentAttr != NULL)
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_EMERGENCY_ALARM) == 0)
						{
							if (strcmp(command->valuestring, "02") == 0)
							{
								sprintf(currentAttr->value, "%d", 1);
								reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, currentAttr->key, currentAttr->value);
							}
							break;
						}
						else
						{
							currentAttr = currentAttr->next;
						}
					}
				}
				break;
			}
			case REX_DEV_TYPE_SECURITY_REMOTE_CONTROL:
			{
				cJSON *armMode = cJSON_GetObjectItem(root, "ArmMode");
				if (armMode != NULL)
				{
					Attribute *currentAttr;
					currentAttr = currentNode->attrHead;
					while (currentAttr != NULL)
					{
						if (strcmp(currentAttr->key, ATTRIBUTE_NAME_KEY_FOB_VALUE) == 0)
						{
							sprintf(currentAttr->value, "%s", armMode->valuestring + 1);
							reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, currentAttr->key, currentAttr->value);
							break;
						}
						else
						{
							currentAttr = currentAttr->next;
						}
					}
				}
				break;
			}
			case REX_DEV_TYPE_SMART_LOCK:
			{
				cJSON *command = cJSON_GetObjectItem(root, "Command");
				if (command != NULL)
				{
					if (strcmp(command->valuestring, "00") == 0)
					{
						cJSON *armMode = cJSON_GetObjectItem(root, "ArmMode");
						if (armMode != NULL)
						{
							int mode = atoi(armMode->valuestring);
							char tmpMode[4]={0};
							sprintf(tmpMode,"%d",mode);

							Attribute *currentAttr;
							currentAttr = currentNode->attrHead;
							while (currentAttr != NULL)
							{
								if (strcmp(currentAttr->key, ATTRIBUTE_NAME_ARM_MODE) == 0)
								{
									reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, currentAttr->key, tmpMode);
									break;
								}
								else
								{
									currentAttr = currentAttr->next;
								}
							}

						}

					}
				}
				break;
			}
			default:
				break;
			}
			break;
		}
		case 0x03A1:/*非标准辅控开关按键事件*/
		{
			subItem = cJSON_GetObjectItem(root, "KeyValue");
			if (subItem != NULL)
			{
				Attribute *currentAttr;
				currentAttr = currentNode->attrHead;
				if (currentAttr == NULL)
				{
					reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, "KeyFobValue", subItem->valuestring);
					break;
				}

				while (currentAttr != NULL)
				{
					if (1==endpoint_id && (strcmp(currentAttr->key, "KeyFobValue") == 0 || strcmp(currentAttr->key, "KeyFobValue_1") == 0))
					{
						reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, currentAttr->key, subItem->valuestring);
						break;
					}
					else if (1==endpoint_id)
					{
						reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, "KeyFobValue", subItem->valuestring);
						break;
					}
					else if(2==endpoint_id && strcmp(currentAttr->key, "KeyFobValue_2") == 0 )
					{
						reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, currentAttr->key, subItem->valuestring);
						break;
					}
					else if(3==endpoint_id && strcmp(currentAttr->key, "KeyFobValue_3") == 0 )
					{
						reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, currentNode->modelId, currentAttr->key, subItem->valuestring);
						break;
					}
					else
						currentAttr = currentAttr->next;
				}
			}
			break;
		}
#ifdef CTRL_LOCK
		case 0x0307:/*智能锁具远程开锁/上锁结果上报*/
		{
			reportFunction(REPORT_TYPE_EVENT, currentNode->deviceId, \
					currentNode->modelId,EVENT_REMOTE_UNLOCK_NOTIFICATION , event);
			break;
		}
#endif
		default:
			break;
		}
		res = 0;
		cJSON_Delete(root);
	}

	else
	{
		WARN("[ZIGBEE] Event report: No corresponding device in the device list\n\n");
		res = -1;
	}

	return res;
}

int report_self_defining_data_cb(char *address, unsigned char endpoint_id, char *data, unsigned char length)
{
	INFO("[ZIBGEE]\033[40;33m[REX_REPORT]\033[0m Custom data report: deviceId=%s, endpoint=%d\n",address,endpoint_id);
	INFO("[ZIBGEE]\033[40;33m[REX_REPORT]\033[0m Custom data report: data=%s\n",data);
	LOG_HEX(LOG_DEBUG,data,0,length);

	int find=0;
	cJSON *item;
	item=cJSON_CreateObject();
	pthread_mutex_lock(&mutexLinkTable);
	DevInfo *pCurrentNode = pHead;
	while(pCurrentNode)
	{
		if (strncmp(pCurrentNode->deviceId, address, 16) == 0)
		{
			gettimeofday(&pCurrentNode->onlineInfo->lastUpdateTime, NULL);
			if (pCurrentNode->onlineInfo->online == 0)
			{
				pCurrentNode->onlineInfo->online = 1;
				pCurrentNode->onlineInfo->offCount = 0;

				char outBuff[400];
				ReportAttrEventOnline online;
				sprintf(online.deviceId, "%s", pCurrentNode->deviceId);
				sprintf(online.modelId, "%s", pCurrentNode->modelId);
				sprintf(online.key, "%s", "Online");
				sprintf(online.value, "%s", "1");
				online.next = NULL;
				packageReportMessageToJson(REPORT_TYPE_ONOFF, (void*)&online, outBuff);

				/*此处插入上报函数*/
				messageReportFun(outBuff);
			}
			pCurrentNode->onlineInfo->offCount = 0;

			full_HandleSceneCustomReport(pCurrentNode,(unsigned char*)data, length);


			find=1;
			cJSON_AddStringToObject(item,"ModelId",pCurrentNode->modelId);
			break;
		}
		pCurrentNode=pCurrentNode->next;
	}



	pthread_mutex_unlock(&mutexLinkTable);
	if (find==0)
	{
		WARN("DeviceId= %s did not join the gateway\n\n",address);
		cJSON_Delete(item);
		return 0;
	}



	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Command","Report");
	cJSON_AddStringToObject(root,"FrameNumber","00");
	cJSON_AddStringToObject(root,"Type","CustomData");
	cJSON *datas;
	cJSON_AddItemToObject(root,"Data",datas=cJSON_CreateArray());

	cJSON_AddStringToObject(item,"DeviceId",address);
	cJSON_AddStringToObject(item,"Key","CustomData");
	char tmp[2]={0};
	sprintf(tmp,"%d",endpoint_id);
	cJSON_AddStringToObject(item,"Endpoint",tmp);

	char strBase64[500];
	base64_encode((unsigned char*)data, length, strBase64);
	cJSON_AddStringToObject(item,"Value",strBase64);
	cJSON_AddItemToArray(datas,item);
	char* strJson;
	strJson=cJSON_Print(root);
	messageReportFun(strJson);
	if(strJson)
		free(strJson);
	cJSON_Delete(root);
	return 0;
}


int replaceOldTable()
{
	int res;
	char sqlStr[500];
	sprintf(sqlStr,"ALTER TABLE \"Table_device_list\" RENAME TO \"_Table_device_list_old\";");
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] Replace Table_device_list NAME to _Table_device_list_old failed:%s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
		INFO("[ZIGBEE] Replace Table_device_list NAME to _Table_device_list_old : Success\n\n");

	sprintf(sqlStr, "%s", "CREATE TABLE Table_device_list( \
			deviceId CHAR(16), \
			subType INTEGER(4),\
			name CHAR(200),\
			modelId CHAR(50),\
			hardwareVersion CHAR(12),\
			softwareVersion CHAR(12),\
			registerStatus INTEGER(2),\
			secret CHAR(44),\
			reportTime CHAR(5));");

	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] CREATE TABLE Table_device_list %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
		INFO("[ZIGBEE] CREATE TABLE Table_device_list :Success\n\n");

	sprintf(sqlStr,"INSERT INTO Table_device_list (deviceId,subType,name,modelId,hardwareVersion,softwareVersion,registerStatus,secret) SELECT deviceId,subType,name,modelId,hardwareVersion,softwareVersion,registerStatus,secret FROM _Table_device_list_old;");
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);

	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] INSERT INTO Table_device_list from _Table_device_list_old failed:%s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	else
		INFO("[ZIGBEE]INSERT INTO Table_device_list from _Table_device_list_old: Success\n\n");

	return 0;
}


static int callback(void *data,int argc,char **argv,char **azColName)
{
	int i;
	char *tmp1=NULL;
	char *tmp2=NULL;
	char *tmp3=NULL;
	for (i=0;i<argc;i++)
	{
		tmp1=strstr(argv[i]?argv[i]:"NULL","modelId CHAR(20)");
		tmp2=strstr(argv[i]?argv[i]:"NULL","name CHAR(20)");
		tmp3=strstr(argv[i]?argv[i]:"NULL","name CHAR(44)");
		if(tmp1!=NULL || tmp2!=NULL || tmp3!=NULL)
		{
			replaceOldTable();
			break;
		}
	}
	return 0;
}



/*初始化设备链表*/
int initDevInfoTable()
{
	pDB = NULL;
	zErrMsg = NULL;
	char **azResult;
	int nRow = 0;
	int nColum = 0;
	char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
	int res;

	res = sqlite3_open(dataBasePath, &pDB);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] Open database %s failed: %s\n", dataBasePath,sqlite3_errmsg(pDB));
		return -1;
	}

	sprintf(sqlStr, "%s", "CREATE TABLE Table_device_list( \
			deviceId CHAR(16), \
			subType INTEGER(4),\
			name CHAR(200),\
			modelId CHAR(50),\
			hardwareVersion CHAR(10),\
			softwareVersion CHAR(10),\
			registerStatus INTEGER(2),\
			secret CHAR(44),\
			reportTime CHAR(5));");

	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] CREATE TABLE Table_device_list %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sprintf(sqlStr,"SELECT *FROM sqlite_master WHERE type=\"table\" and name=\'Table_device_list\';");
	res = sqlite3_exec(pDB, sqlStr, callback, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] !!!!!!!:%s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sleep(2);
	sprintf(sqlStr,"DROP TABLE _Table_device_list_old;");
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] delete _Table_device_list_old failed:%s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}


	sprintf(sqlStr,"%s","ALTER TABLE Table_device_list ADD COLUMN reportTime CHAR(5);");
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] Add reportTIme column to Table_device_list failed:%s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sprintf(sqlStr,"%s","UPDATE Table_device_list SET reportTime='300' WHERE reportTime IS NULL;");
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] Set column reportTIme=300 failed:%s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sprintf(sqlStr,"%s","delete from Table_device_list where rowid not in (select max(rowid) from Table_device_list group by deviceId);");
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		INFO("[ZIGBEE] delete same record failed:%s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sprintf(sqlStr, "%s", "SELECT * FROM Table_device_list");
	res = sqlite3_get_table(pDB, sqlStr, &azResult, &nRow, &nColum, &zErrMsg);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIBGEE] %s : %s\n\n", sqlStr, sqlite3_errmsg(pDB));
		sqlite3_free(zErrMsg);
		sqlite3_free_table(azResult);
		sqlite3_close(pDB);
		return -1;
	}

	if (nRow == 0)/*如果为空，自动扫描，添加数据库，建立链表节点*/
	{
		INFO("[ZIBGEE] initDevInfoTable: Table_device_list is empty,tatal 0\n");
	}
	else
	{
		INFO("[ZIBGEE] initDevInfoTable: %d devices have been added to the gateway\n\n", nRow);
		devcieNumber = nRow;
		int i;
		for (i = 0; i < nRow; i++)
		{

			DevInfo *newDevNode=NULL;

			if (azResult[(1 + i)*nColum + 3] != NULL)
			{
				INFO("[ZIBGEE] Read %d device data: %s\n", i+1,azResult[(1 + i)*nColum + 0]);
				newDevNode=ZHA_creatDevInfoStruct(azResult[(1 + i)*nColum + 3]);
			}

			if (newDevNode==NULL)
				continue;

			sprintf(newDevNode->realModelId, "%s", azResult[(1 + i)*nColum + 3]);

			sprintf(newDevNode->deviceId, "%s", azResult[(1 + i)*nColum + 0]);


			checkAirConditionerChannel(newDevNode->subDevType,newDevNode->attrHead);

			sprintf(newDevNode->name,"%s",azResult[(1 + i)*nColum + 2]);

			sprintf(newDevNode->hardwareVersion, "%s", azResult[(1 + i)*nColum + 4]);

			sprintf(newDevNode->softwareVersion, "%s", azResult[(1 + i)*nColum + 5]);

			newDevNode->registerStatus = atoi(azResult[(1 + i)*nColum + 6]);

			sprintf(newDevNode->onlineInfo->deviceId, "%s", newDevNode->deviceId);


			if (pOnlineHead == NULL)
			{
				pOnlineHead = newDevNode->onlineInfo;
				pOnlineTail = newDevNode->onlineInfo;
			}
			else
			{
				pOnlineTail->next = newDevNode->onlineInfo;
				pOnlineTail = newDevNode->onlineInfo;
			}

			if (pHead == NULL)
			{
				pHead = newDevNode;
				pTail = newDevNode;
			}
			else
			{
				pTail->next = newDevNode;
				pTail = newDevNode;
			}
		}
	}
	sqlite3_free_table(azResult);
	pthread_mutex_init(&mutexLinkTable, NULL);
	pthread_mutex_init(&mutexOnlineTable, NULL);
	return 0;
}

int initRexSDK()
{
	if (rex_init_process(REX_LL_INFO, 1048576, REX_TL_NONE) != 0)
	{
		ERROR("[ZIBGEE] Rex_SDK Log initialization settings failed !\n\n");
		return -1;
	}
	else
	{
		/*设置串口*/
		if (rex_set_serial_port(SERIAL_NAME_STRING, SERIAL_BAUD_RATE) != 0)
		{
			ERROR("[ZIBGEE] Rex_SDK Serial initialization failed!\n\n");
			return -1;
		}
		else
		{
			if (rex_set_ota_image_file_path(fileDownloadPath) != 0)
			{
				ERROR("[ZIBGEE] Rex_SDK Fail to set ota image file path!\n\n");
				return -1;
			}
			/*设置入网回调*/
			if (rex_set_callback(REX_DEVICE_JOIN, device_join_cb) != 0)
			{
				ERROR("[ZIGBEE] Rex_SDK Fail to set the callback function of device join network!\n\n");
				return -1;
			}
			else
			{
				/*设置离网回调*/
				if (rex_set_callback(REX_DEVICE_LEAVE, device_leave_cb) != 0)
				{
					ERROR("[ZIGBEE] Rex_SDK Fail to set the callback function of device leave network!\n\n");
					return -1;
				}
				else
				{
					/*设置状态上报回调*/
					if (rex_set_callback(REX_REPORT_STATE_DATA, report_state_data_cb) != 0)
					{
						ERROR("[ZIGBEE] Rex_SDK Fail to set the callback function of report state data!\n\n");
						return -1;
					}
					else
					{
						/*设置事件数据上报回调*/
						if (rex_set_callback(REX_REPORT_EVENT_DATA, report_event_data_cb) != 0)
						{
							ERROR("[ZIBGEE] Rex_SDK: Fail to set the callback function of report event data!\n\n");
							return -1;
						}
						else
						{
							if (rex_set_callback(REX_REPORT_SELF_DEFINING_DATA, report_self_defining_data_cb) != 0)
							{
								ERROR("[ZIBGEE] Rex_SDK: Fail to set the callback function of report self defining data!\n\n");
								return -1;
							}
							else
							{
								if (rex_start() != 0)
								{
									ERROR("[ZIBGEE] Rex_SDK: Fail to start rex_sdk!\n\n");
									return -1;
								}
								setCustomAttributes();

							}
						}
					}
				}
			}
		}
	}

	return 0;
}





void threadSubDevHeartBeat(void)
{
	DevOnlineInfo *pCurrentNode;
	DevOnlineInfo *pPreNode;
	DevOnlineInfo *pDelete;

	struct timeval check;
	DEBUG("[ZIBGEE] Open sub device heart beat thread\n\n");
	while (1)
	{

#ifdef TY_DEV
		select_sleep(280);
		/*280s检查一次*/
#else
		select_sleep(310);
		/*300s检查一次*/
#endif
		pCurrentNode = pOnlineHead;
		pPreNode = pOnlineHead;
		if (pOnlineHead == NULL)
		{
			continue;
		}
		else
		{
			while (pCurrentNode != NULL)
			{
#ifdef TY_DEV
				sendCommandCallDeviceResponse(pCurrentNode->deviceId);
				select_mssleep(200);
#endif
				if (pCurrentNode->leaveGateway == 0)/*如果该节点设备已经离网，则删除该设备*/
				{
					pthread_mutex_lock(&mutexOnlineTable);
					if (pCurrentNode == pOnlineHead && pCurrentNode == pOnlineTail)
					{

						pDelete = pCurrentNode;
						pOnlineHead = NULL;
						pOnlineTail = NULL;
						pCurrentNode = pOnlineHead;
					}
					else if (pCurrentNode == pOnlineHead && pCurrentNode != pOnlineTail)
					{
						pDelete = pCurrentNode;
						pOnlineHead = pCurrentNode->next;
						pPreNode = pCurrentNode->next;
						pCurrentNode = pCurrentNode->next;

					}
					else if (pCurrentNode != pOnlineHead && pCurrentNode == pOnlineTail)
					{

						pDelete = pCurrentNode;
						pPreNode->next = NULL;
						pOnlineTail = pPreNode;
						pCurrentNode = NULL;

					}
					else
					{
						pDelete = pCurrentNode;
						pPreNode->next = pCurrentNode->next;
						pCurrentNode = pCurrentNode->next;
					}
					pthread_mutex_unlock(&mutexOnlineTable);
					free(pDelete);
					continue;
				}

				gettimeofday(&check, NULL);

				if ((check.tv_sec - pCurrentNode->lastUpdateTime.tv_sec) >\
						((pCurrentNode->offCount+1)*pCurrentNode->reportTime+10))
				{

					pCurrentNode->offCount++;
					if (pCurrentNode->offCount == 3)
					{

						INFO("[ZIGEE] Device address %s no replay  %d times\n\n",pCurrentNode->deviceId, pCurrentNode->offCount);
						INFO("[ZIGBEE] Device address %s is offline!\n\n", pCurrentNode->deviceId);
						pCurrentNode->online = 0;
						char modelId[12];
						getDevModelId(pCurrentNode->deviceId, modelId);
						reportFunction(REPORT_TYPE_ONOFF, pCurrentNode->deviceId, modelId, "Online", "0");
					}
					else if (pCurrentNode->offCount > 3)
					{

						INFO("[ZIGEE] Device address %s is still offline!\n\n", pCurrentNode->deviceId);
						pCurrentNode->offCount = 3;
						pCurrentNode->online = 0;
					}
					else
					{

						INFO("[ZIGEE] Device address %s no replay %d times\n\n", pCurrentNode->deviceId,pCurrentNode->offCount);
					}
				}
				pPreNode = pCurrentNode;
				pCurrentNode = pCurrentNode->next;
			}
		}

	}
}


int isNonStandSceneDevice(int subType)
{
	int res=0;
	switch(subType)
	{
	case 0x9906:
	case 0x9907:
	case 0x9908:
	case 0x9909:
	case 0x9910:
	case 0x9001:
	case 0x1202:
	{
		res=1;
		break;
	}
	default:
		break;
	}
	return res;
}

int queryAllSpecyPatram(char *deviceID,char *modelID)
{
	int res=0;
	if(strcmp(modelID,"HY0095")==0 || strcmp(modelID,"HY0096")==0|| strcmp(modelID,"HY0097")==0)
	{
		res=sendCommandReadCustomParameters(deviceID,1,"LedEnable","PowerOffProtection",NULL,NULL);
	}
	else if (strcmp(modelID,"0a0c3c")==0 || strcmp(modelID,"HY0042")==0|| strcmp(modelID,"0a0c55")==0 \
			|| strcmp(modelID,"HY0043")==0 )
	{
		res=sendCommandReadCustomParameters(deviceID,1,"SwitchChildLock",NULL,NULL,NULL);
	}
	else if (strcmp(modelID,"HY0075")==0 )
	{
		res=sendCommandReadCustomParameters(deviceID,1,"WindRainSensor",NULL,NULL,NULL);
	}
	else if (strcmp(modelID,"HY0045")==0 )
	{
		res=sendCommandReadCustomParameters(deviceID,1,"COValue","ErrorCode",NULL,NULL);
	}
	else if (strcmp(modelID,"HY0106")==0 || strcmp(modelID,"HY0105")==0|| strcmp(modelID,"HY0104")==0)
	{
		res=sendCommandReadCustomParameters(deviceID,1,"LedEnable","PowerOffProtection","SwitchChildLock","VoltageOverloadProtection");
		res=sendCommandReadCustomParameters(deviceID,1,"PowerConsumptionClear","OverloadProtection","OverloadProtectEnable",NULL);
		res=sendCommandReadCustomParameters(deviceID,1,"VotageOverload","CurrentOverload",NULL,NULL);
	}
	else if (strcmp(modelID,"HY0102")==0)
	{
		res=sendCommandReadCustomParameters(deviceID,1,"LedEnable","PowerOffProtection","SwitchChildLock",NULL);
		res=sendCommandReadCustomParameters(deviceID,1,"OverloadProtection","VoltageOverloadProtection",NULL,NULL);
		res=sendCommandReadCustomParameters(deviceID,2,"OverloadProtection","VoltageOverloadProtection",NULL,NULL);
	}
	else if (strcmp(modelID,"HY0098")==0 )
	{
		res=sendCommandReadCustomParameters(deviceID,1,"LedEnable",NULL,NULL,NULL);
	}
	else if(strcmp(modelID,"HY0128")==0 )
	{
		res=sendCommandReadCustomParameters(deviceID,1,"Atomization",NULL,NULL,NULL);
		res=sendCommandReadCustomParameters(deviceID,2,"Atomization",NULL,NULL,NULL);
	}
	else if (strcmp(modelID,"HY0107")==0 || strcmp(modelID,"HY0121")==0 || strcmp(modelID,"HY0122")==0)
	{
		res=sendCommandReadCustomParameters(deviceID,1,"LedEnable","PowerOffProtection","KeyMode",NULL);
	}
	else if (strcmp(modelID,"HY0141")==0 || strcmp(modelID,"HY0142")==0)
	{
		res=sendCommandReadCustomParameters(deviceID,1,ATTRIBUTE_NAME_DCDELAY,ATTRIBUTE_NAME_LOCAL_CONFIG,NULL,NULL);
	}
	else if (strcmp(modelID,"HY0148")==0)
	{
		sendCommandReadCustomParameters(deviceID,1,"LightMode",NULL,NULL,NULL);
	}
	else if (strcmp(modelID,"HY0134")==0 || strcmp(modelID,"HY0112")==0)
	{
		sendCommandReadCustomParameters(deviceID,1,"WorkMode",NULL,NULL,NULL);
		usleep(200000);
		res=fullscene_GetSceneName(deviceID,NULL);
		usleep(200000);
		res=fullscene_GetScenePhoto(deviceID,NULL);
		usleep(200000);
		res=fullscene_GetEnable(deviceID,0);


	}
	else if ( strcmp(modelID,"HY0160")==0 || strcmp(modelID,"HY0161")==0)
	{
		sendCommandWriteCustomParameters(deviceID, "GetDeviceStatus",modelID);
		//sendCommandReadCustomParameters(deviceID,1,"WorkMode","GetDeviceStatus",NULL,NULL);
		usleep(200000);
		res=fullscene_GetSceneName(deviceID,NULL);
		usleep(200000);
		res=fullscene_GetScenePhoto(deviceID,NULL);
		usleep(200000);
		res=fullscene_GetEnable(deviceID,0);
		usleep(200000);
		res=fullscene_GetDltStatus(deviceID,0xff,0xff);
		usleep(200000);
		fullscene_GetSwitchEndpoints(deviceID,0x05);
		usleep(200000);
		fullscene_GetSwitchEndpoints(deviceID,0x06);
	}
	return res;
}


int threadCallDeviceResponse()
{
	DevInfo *pCurrentNode = pHead;
	if (pCurrentNode == NULL)
		return 0;

	while(pCurrentNode!=NULL)
	{
		if(isNonStandSceneDevice(pCurrentNode->subDevType))
			sendCommandCallDeviceResponse(pCurrentNode->deviceId);
		else
		{
			if (strcmp(pCurrentNode->modelId,"HY0134")==0 || strcmp(pCurrentNode->modelId,"HY0112")==0)
			{
				sendCommandGetDeviceState(pCurrentNode->deviceId, 1);
			}
			else
				sendCommandGetDeviceState(pCurrentNode->deviceId, 255);
			queryAllSpecyPatram(pCurrentNode->deviceId,pCurrentNode->realModelId);
		}

		pCurrentNode=pCurrentNode->next;
	}
	return 0;
}




int gatewayInit()
{

	if (initDevInfoTable() != 0)
		return -1;
	if (groupInit(pDB)!=0)
		return -1;
	if (bindListInit(pDB)!=0)
		return -1;

	initSwitchGroup();

	pthread_t phandleMsg;
	int res;
	res = pthread_create(&phandleMsg, NULL, (void *)thread_handle_message, NULL);
	if (res != 0)
	{
		ERROR("[ZIGBEE] Open thread_handle_message() thread error\n\n");
		return -1;
	 }


	gettimeofday(&tm,NULL);
	initRexSendLock();

	if (initRexSDK() != 0)
		return -1;


	resetLed();
	pthread_t pSubDevHeartBeart;
	sendCommandGetZigbeeNetworkInfo();
	sendCommandGetCooVer();
	sendCommandGetRexSDKVersion();
	upgradeCoofirmware();

	res = pthread_create(&pSubDevHeartBeart, NULL, (void *)threadSubDevHeartBeat, NULL);
	if (res != 0)
	{
		ERROR("[ZIGBEE] Open sub device heartbeat thread error\n\n");
		return -1;
	}

	pthread_t pUdp;
	res = pthread_create(&pUdp, NULL, (void *) udpServer, NULL);
	if (res != 0)
	{
		ERROR("[ZIGBEE] Open sub device udp server thread error\n\n");
		return -1;
	}
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);
	pthread_t pCallDevResponse;
	res = pthread_create(&pCallDevResponse,  &attr, (void *)threadCallDeviceResponse, NULL);
	pthread_attr_destroy (&attr);
	if (res != 0)
	{
		ERROR("[ZIBGEE] Open call device response thread error\n\n");
		return -1;
	}

	return 0;

}


/********************设备控制,查询,加入,删除*****************************/
int deviceCtrlCommand(KeyValue *currentKeyValue, DevInfo* currentNode,int rexSUbType)
{
	int res = -1;
	if (strcmp(currentKeyValue->key, "GetStatus") == 0)
	{
		res=sendCommandGetDeviceState(currentKeyValue->address, 255);
		return res;
	}
	else if (strcmp(currentKeyValue->key, "GetMCUVersion") == 0)
	{
		res=sendCommandGetMcuVersion(currentKeyValue->address);
		return res;
	}

	int rexType;
	if (currentNode!=NULL)
		rexType=currentNode->subDevType;
	else
		rexType=rexSUbType;

	switch (rexType)
	{
	case REX_DEV_TYPE_WATER_OVERFLOW_SENSOR:
	case REX_DEV_TYPE_THERMAL_RELEASE:
	case REX_DEV_TYPE_DOOR_MAGNETISM:
	case REX_DEV_TYPE_SMOKE_GAS_SENSOR:
	{
		res = devCtrlSecurityEquipment(currentKeyValue, currentNode);
		char tmpModelId[10];
		getDevModelId(currentKeyValue->address,tmpModelId);
		reportFunction(REPORT_TYPE_ATTRIBUTE, currentKeyValue->address, \
				tmpModelId, currentKeyValue->key, currentKeyValue->value);
		break;
	}
	case REX_DEV_TYPE_1_WAY_SWITCH:
	case REX_DEV_TYPE_2_WAY_SWITCH:
	case REX_DEV_TYPE_3_WAY_SWITCH:
	case REX_DEV_TYPE_4_WAY_SWITCH:
	{
		res = devCtrlSwitchPannel123(currentKeyValue);
		break;
	}
	case REX_DEV_TYPE_DLT:
	case REX_DEV_TYPE_1_WAY_AUX_DLT:
	case REX_DEV_TYPE_2_WAY_AUX_DLT:
	case REX_DEV_TYPE_3_WAY_AUX_DLT:
	case REX_DEV_TYPE_4_WAY_AUX_DLT:
	{
		res=devCtrlSwitchDLT(currentKeyValue);
		break;
	}
	case REX_DEV_TYPE_HSL_LIGHT:
	{

		res=devCtrlLightHSL(currentKeyValue);
		break;
	}
	case REX_DEV_TYPE_ELECTRIC_CURTAIN:
	{
		res= devCtrlElectricCurtain(currentKeyValue);
		break;
	}
	case REX_DEV_TYPE_HUMMAN_BODY_INDUCTION:
	{
		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH) == 0)
		{
			res = sendCommandCtrlSwitchState(currentKeyValue->address, 1, currentKeyValue->value);
		}
		else if (strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH) == 0)
		{
			res = sendCommandCtrlSwitchState(currentKeyValue->address, 1, "2");
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_EMERGENCY_ALARM) == 0)
		{
			if (strcmp(currentKeyValue->value, "0") == 0)
			{
				char outBuff[400];
				ReportAttrEventOnline reportAttr;
				sprintf(reportAttr.deviceId, "%s", currentKeyValue->address);
				sprintf(reportAttr.key, "%s", ATTRIBUTE_NAME_EMERGENCY_ALARM);
				sprintf(reportAttr.value, "%d", 0);
				reportAttr.next = NULL;
				packageReportMessageToJson(REPORT_TYPE_EVENT, (void*)&reportAttr, outBuff);
				/*此处插入上报接口*/
				messageReportFun(outBuff);
			}
		}
		break;
	}
	case REX_DEV_TYPE_SECURITY_ALARM:
	{
		res=devCtrlSecurityAlarm(currentKeyValue);
		break;
	}
	case REX_DEV_TYPE_1_WAY_SOCKET:
	case REX_DEV_TYPE_2_WAY_SOCKET:
	{
		res = devCtrlSocket12(currentKeyValue);
		break;
	}
	case REX_DEV_TYPE_FRESH_AIR_SYSTEM:
	case REX_DEV_TYPE_2_WAY_FRESH_AIR_SYSTEM:
	{
		res=devCtrlFreshAirSystem(currentKeyValue);
		break;
	}
	case REX_DEV_TYPE_HEART_COOL_DEVICE:
	{
		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH) == 0)
		{
			if (strcmp(currentKeyValue->value, "0") == 0)
			{
				res = sendCommandCtrlSwitchState(currentKeyValue->address, 1, "0");
			}
			else if (strcmp(currentKeyValue->value, "1") == 0)
			{
				res = sendCommandCtrlSwitchState(currentKeyValue->address, 1, "1");
			}
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE) == 0)
		{
			res = sendCommandSetTemperature(currentKeyValue->address, 1, currentKeyValue->value);
		}

		break;
	}
	case REX_DEV_TYPE_1_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_2_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_3_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_4_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_5_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_6_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_7_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_8_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_9_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_10_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_11_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_12_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_13_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_14_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_15_WAY_AIR_CONDITIONER:
	case REX_DEV_TYPE_16_WAY_AIR_CONDITIONER:
	{
		res = devCtrlAirConditioner(currentKeyValue);

		if (res==-1 && (strcmp(currentNode->realModelId,"HY0134")==0 \
				|| strcmp(currentNode->realModelId,"HY0112")==0 \
				|| strcmp(currentNode->realModelId,"HY0160")==0))
		{

			res=full_SceneCtrlCommand(currentKeyValue);
		}
		break;
	}
	default:
		break;
	}
	return res;
}


int gatewayCtrlCommand(KeyValue *keyValue)
{
	int res=0;
	if (strcmp(keyValue->key,"GetCooInfo")==0)
	{
		res=sendCommandGetCooInfo();
	}
	else if (strcmp(keyValue->key,"PermitJointing")==0)
		res=-1;
	else if (strcmp(keyValue->key,"RepeatReport")==0)
	{
		repeatReport=atoi(keyValue->value);
	}
	return res;

}

int handleCtrlDeviceMsg(KeyValue *keyValueHead)
{
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	DevInfo* currentNode;

	int find;

	int res = -1;
	find = 0;
	while (currentKeyValue != NULL)
	{
		if (strncmp(currentKeyValue->address, "0000000000000000", 16) == 0)
		{
			find=1;
			res=gatewayCtrlCommand(currentKeyValue);
			currentKeyValue = currentKeyValue->next;
			continue;

		}
		currentNode = pHead;

		while (currentNode != NULL)
		{
			if (strncmp(currentNode->deviceId, currentKeyValue->address, 16) == 0)
			{
				find = 1;
				res = deviceCtrlCommand(currentKeyValue, currentNode,0);
				break;
			}
			else
			{
				currentNode = currentNode->next;
			}
		}

		if (find == 0 || res == -1)
		{
			return -1;
		}
		currentKeyValue = currentKeyValue->next;
	}

	if (find == 1 && res == 0)
		return 0;
	else
		return -1;
}


int deleteDevInfo(char *address)
{
	int res = 0;
	char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
	DevInfo *pCurrentNode;
	DevInfo *pPreviousNode;
	DevInfo *pDelteNode;

	sprintf(sqlStr, "DELETE FROM Table_device_list WHERE deviceId=\'%s\';", address);
	res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
	if (res != SQLITE_OK)
	{
		ERROR("[\033[40;31mError\033[0m] Delete device %s infomation failed in database %s\r\n", address, zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	pthread_mutex_lock(&mutexLinkTable);
	pCurrentNode = pHead;
	pPreviousNode = pHead;

	if (pCurrentNode == NULL)
	{
		printf("[Debug] Delete device infomation: device %s does not exist in the gateway database and linkTable\n\n!", address);
		pthread_mutex_unlock(&mutexLinkTable);
		return 0;
	}


	while (pCurrentNode != NULL)
	{
		if (strncmp(pCurrentNode->deviceId, address, 16) != 0)
		{
			pPreviousNode = pCurrentNode;
			pCurrentNode = pCurrentNode->next;
		}
		else
		{
			devcieNumber--;
			pCurrentNode->onlineInfo->leaveGateway = 0;
			pCurrentNode->onlineInfo = NULL;
			if (pCurrentNode == pHead && pCurrentNode == pTail)
			{
				pDelteNode = pCurrentNode;
				pHead = NULL;
				pTail = NULL;
			}
			else if (pCurrentNode == pHead && pCurrentNode != pTail)
			{
				pDelteNode = pCurrentNode;
				pHead = pCurrentNode->next;
			}
			else if (pCurrentNode != pHead && pCurrentNode == pTail)
			{
				pDelteNode = pCurrentNode;
				pPreviousNode->next = NULL;
				pTail = pPreviousNode;
			}
			else
			{
				pDelteNode = pCurrentNode;
				pPreviousNode->next = pCurrentNode->next;

			}
			pthread_mutex_unlock(&mutexLinkTable);

			deleteDevAttrList(pDelteNode->attrHead);

			DEBUG("[ZIGBEE] Delete device %s type %04X information success!\n\n", pDelteNode->deviceId, pDelteNode->subDevType);
			free(pDelteNode);
			return 0;
		}
	}

	INFO("[ZIGBEE] Delete device infomation:The device %s does not exist in the gateway database and linkTable\n\n!", address);
	pthread_mutex_unlock(&mutexLinkTable);
	return -1;
}

int handleDeleteDeviceMsg(KeyValue *keyValueHead)
{
	int res = -1;
	int lastErrorCode = 0;
	KeyValue *currentNode;
	currentNode = keyValueHead;
	while (currentNode != NULL)
	{
		res = deleteDevInfo(currentNode->address);

		if (res==0)
		{
			char outBuff[400];
			packageReportMessageToJson(REPORT_TYPE_UNREGISTER, (void*)currentNode->address, outBuff);
			messageReportFun(outBuff);
		}
		groupDeleteItem(currentNode->address,NULL, NULL);
		bindDeleteAllItem(currentNode->address);
		sendCommandDeleteDevice(currentNode->address);
		currentNode = currentNode->next;
		if (res != 0)
			lastErrorCode = -1;
	}
	return lastErrorCode;
}

int handleAddDeviceMsg(KeyValue *keyValueHead)
{
	int res = 0;
	res = allowDeviceJoinGateway(keyValueHead->value);
	return res;
}

int checkAttriNameCanQuery(char *attributeName,char *modelId)
{
	if (strcmp(attributeName,ATTRIBUTE_NAME_KEY_FOB_VALUE)==0)
	{
		if(strcmp(modelId,"79159d")==0)
			return 0;
		return 1;
	}
	else if(strcmp(attributeName,ATTRIBUTE_NAME_CURTAIN_MODE)==0)
		return 1;
	else if(strcmp(attributeName,ATTRIBUTE_NAME_SIREN_WARNING_MODE)==0)
		return 1;
	else if(strcmp(attributeName,ATTRIBUTE_NAME_LOCK_ALARM)==0)
		return 1;
	else if(strcmp(attributeName,ATTRIBUTE_NAME_USR_ADD)==0)
		return 1;
	else if(strcmp(attributeName,ATTRIBUTE_NAME_USR_DELETE)==0)
		return 1;
	else if(strcmp(attributeName,ATTRIBUTE_NAME_OPEN_DOOR)==0)
		return 1;
	else if(strcmp(attributeName,ATTRIBUTE_NAME_ARMMODE)==0)
		return 1;
	else if(strcmp(attributeName,ATTRIBUTE_NAME_DIRECTION_ALARM)==0)
		return 1;

	return 0;
}

int queryDeviceAttrMsg(KeyValue *currentKeyValueNode, char *outbuff)
{
	if (strncmp(currentKeyValueNode->address, "0000000000000000", 16) == 0)
	{

		if (strcmp(currentKeyValueNode->key, "CooMac") == 0)
		{

			ReportAttrEventOnline *reportAttr;
			reportAttr = buildAttributeReportStruct(currentKeyValueNode->address, "000000", "CooMac", cooMac);
			packageReportMessageToJson(REPORT_TYPE_EXT_QUERY, reportAttr, outbuff);
			freeReportAttrList(reportAttr);
		}
		else if (strcmp(currentKeyValueNode->key, "CooVer") == 0)
		{
			ReportAttrEventOnline *reportAttr;
			reportAttr = buildAttributeReportStruct(currentKeyValueNode->address, "000000", "CooVer", cooVersion);
			reportAttr->next = buildAttributeReportStruct(currentKeyValueNode->address, "000000", "CooCode", cooCode);
			packageReportMessageToJson(REPORT_TYPE_EXT_QUERY, reportAttr, outbuff);
			freeReportAttrList(reportAttr);
		}
		else if (strcmp(currentKeyValueNode->key, "RexSDKVer") == 0)
		{
			ReportAttrEventOnline *reportAttr;
			reportAttr = buildAttributeReportStruct(currentKeyValueNode->address, "000000", "RexSDKVer", rexSdkVer);
			packageReportMessageToJson(REPORT_TYPE_EXT_QUERY, reportAttr, outbuff);
			freeReportAttrList(reportAttr);
		}
		else if (strcmp(currentKeyValueNode->key, "PermitJoining") == 0)
		{
			ReportAttrEventOnline *reportAttr;
			char value[2];
			sprintf(value, "%d", get_led_ctrl_flag());
			reportAttr = buildAttributeReportStruct(currentKeyValueNode->address, "000000", "PermitJoining", value);
			packageReportMessageToJson(REPORT_TYPE_EXT_QUERY, reportAttr, outbuff);
			freeReportAttrList(reportAttr);
		}
		else if (strcmp(currentKeyValueNode->key, "FirmwareVersion") == 0)
		{
			ReportAttrEventOnline *reportAttr;
			reportAttr = buildAttributeReportStruct(currentKeyValueNode->address, "000000", "FirmwareVersion", GW_FIRMWARE_VERSION);
			packageReportMessageToJson(REPORT_TYPE_EXT_QUERY, reportAttr, outbuff);
			freeReportAttrList(reportAttr);

		}
		else if (strcmp(currentKeyValueNode->key, "SofterVersion") == 0)
		{

			ReportAttrEventOnline *reportAttr;
			reportAttr = buildAttributeReportStruct(currentKeyValueNode->address, "000000", "SofterVersion", GW_SOFTWARE_VERSION);
			packageReportMessageToJson(REPORT_TYPE_EXT_QUERY, reportAttr, outbuff);
			freeReportAttrList(reportAttr);
		}
		else if (strcmp(currentKeyValueNode->key, "All") == 0)
		{
			ReportAttrEventOnline *repAttrHead = NULL;
			ReportAttrEventOnline *repAttrTail = NULL;
			ReportAttrEventOnline **prepAttrHead = &repAttrHead;
			ReportAttrEventOnline **prepAttrTail = &repAttrTail;
			char value[2];
			sprintf(value, "%d", get_led_ctrl_flag());
			addNewAttributeReportNodeToAttributeReportList(currentKeyValueNode->address, "000000", "PermitJoining", value, prepAttrHead, prepAttrTail);

			addNewAttributeReportNodeToAttributeReportList(currentKeyValueNode->address, "000000", "FirmwareVersion", GW_FIRMWARE_VERSION, prepAttrHead, prepAttrTail);

			addNewAttributeReportNodeToAttributeReportList(currentKeyValueNode->address, "000000", "SofterVersion", GW_SOFTWARE_VERSION, prepAttrHead, prepAttrTail);

			packageReportMessageToJson("DevAttri", repAttrHead, outbuff);

			freeReportAttrList(repAttrHead);

		}

		return 0;
	}
	DevInfo *currentNode;
	currentNode = pHead;

	ReportAttrEventOnline *reportAttrHead = NULL;
	ReportAttrEventOnline *reportAttrTail = NULL;
	ReportAttrEventOnline **preportAttrHead = &reportAttrHead;
	ReportAttrEventOnline **preportAttrTail = &reportAttrTail;
	int isAttriAll=0;
	int isFind;
	while (currentKeyValueNode != NULL)/*循环收到的属性链表*/
	{
		isFind=0;
		while (currentNode != NULL)/*循环设备链表，查询设备*/
		{

			if (strcmp(currentNode->deviceId, currentKeyValueNode->address) == 0)
			{
				isFind=1;
				Attribute *currentAttr = currentNode->attrHead;
				if (strncmp("All", currentKeyValueNode->key, 3) == 0)
				{
					isAttriAll=1;
					while (currentAttr != NULL)
					{
						if(0==checkAttriNameCanQuery(currentAttr->key,currentNode->modelId))
							addNewAttributeReportNodeToAttributeReportList(currentNode->deviceId, currentNode->modelId, currentAttr->key, currentAttr->value, preportAttrHead, preportAttrTail);
						currentAttr = currentAttr->next;
					}
					addNewAttributeReportNodeToAttributeReportList(currentNode->deviceId,currentNode->modelId,"SofterVersion", currentNode->softwareVersion, preportAttrHead, preportAttrTail);
					addNewAttributeReportNodeToAttributeReportList(currentNode->deviceId,currentNode->modelId,"FirmwareVersion",currentNode->hardwareVersion, preportAttrHead, preportAttrTail);

				}
				else if (strcmp("SofterVersion", currentKeyValueNode->key) == 0)
				{
					addNewAttributeReportNodeToAttributeReportList(currentNode->deviceId,currentNode->modelId,"SofterVersion", currentNode->softwareVersion, preportAttrHead, preportAttrTail);
				}
				else if (strcmp("FirmwareVersion", currentKeyValueNode->key) == 0)
				{
					addNewAttributeReportNodeToAttributeReportList(currentNode->deviceId,currentNode->modelId,"FirmwareVersion", currentNode->hardwareVersion, preportAttrHead, preportAttrTail);
				}
				else
				{
					while (currentAttr != NULL)
					{
						if (strcmp(currentAttr->key, currentKeyValueNode->key) == 0)
						{
							if(0==checkAttriNameCanQuery(currentAttr->key,currentNode->modelId))
									addNewAttributeReportNodeToAttributeReportList(currentNode->deviceId, \
											currentNode->modelId, currentAttr->key, currentAttr->value, preportAttrHead, preportAttrTail);
							break;
						}
						else if (strcmp(currentKeyValueNode->key,ATTRIBUTE_NAME_VOLTAGE_COFFICIENT)==0 ||\
									strcmp(currentKeyValueNode->key,ATTRIBUTE_NAME_CURRENT_COFFICIENT)==0 ||\
									strcmp(currentKeyValueNode->key,ATTRIBUTE_NAME_POWER_COFFICIENT)==0 ||\
									strcmp(currentKeyValueNode->key,ATTRIBUTE_NAME_POWER_WASTER_COFFICIENT)==0)
						{
							sendCommandReadCustomParameters(currentNode->deviceId,1,ATTRIBUTE_NAME_VOLTAGE_COFFICIENT,\
										ATTRIBUTE_NAME_CURRENT_COFFICIENT,ATTRIBUTE_NAME_POWER_COFFICIENT,\
										ATTRIBUTE_NAME_POWER_WASTER_COFFICIENT);
							break;
						}
						else
							currentAttr = currentAttr->next;
					}
				}
				break;
			}
			else
				currentNode = currentNode->next;
		}
		if(isFind==0)
		{
			ERROR("[ZIBGEE] queryDeviceAttrMsg: deviceid %s do not exist!\n\n",currentKeyValueNode->address);
		}

		currentKeyValueNode = currentKeyValueNode->next;
		currentNode = pHead;
	}

	if (reportAttrHead != NULL)
	{
		if (isAttriAll==1)
			packageReportMessageToJson("DevAttri", reportAttrHead, outbuff);
		else
			packageReportMessageToJson(REPORT_TYPE_EXT_QUERY, reportAttrHead, outbuff);
		freeReportAttrList(reportAttrHead);
	}
	return 0;
}

int threadReport(void *arg)
{
	char *tmp=(char *)arg;
	messageReportFun(tmp);
	return 0;
}

int handleQueryDeviceAttrMsg(KeyValue *currentKeyValueNode, char *outbuff)
{
	int res=-1;
	char tmpbuff[10000]={0};
	queryDeviceAttrMsg(currentKeyValueNode, tmpbuff);
	if (outbuff == NULL)
	{
		if (0!=strlen(tmpbuff))
		{
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_t pReport;
			res = pthread_create(&pReport, &attr, (void *)threadReport, (void *)tmpbuff);
			usleep(20000);
			pthread_attr_destroy(&attr);
			if (res != 0)
			{
				ERROR("[ZIGBEE] Open get device list thread error\n\n");
				return -1;
			}
			else
				res=0;
		}
	}
	else
	{
		if (0!=strlen(tmpbuff))
		{
			sprintf(outbuff,"%s",tmpbuff);
			res=0;
		}
		else
			res=-1;
	}
	return res;

}


int getDevicesList()
{
	int count = 0;
	char outBuff[REPORT_DEV_LIST_LENGTH]={0};

	ReportRegister *head = NULL;
	ReportRegister *tail = NULL;

	pthread_mutex_lock(&mutexLinkTable);
	DevInfo *currentNode;
	currentNode = pHead;
	if (currentNode == NULL)
	{
		pthread_mutex_unlock(&mutexLinkTable);
		packageReportMessageToJson(REPORT_TYPE_DEVLIST, NULL, outBuff);
		messageReportFun(outBuff);
		return 0;
	}

	while (currentNode != NULL)
	{
		ReportRegister *reportReg;
		reportReg = (ReportRegister*)malloc(sizeof(ReportRegister));
		memset(reportReg, 0, sizeof(ReportRegister));

		sprintf(reportReg->deviceId, "%s", currentNode->deviceId);
		sprintf(reportReg->modelId, "%s", currentNode->modelId);
		sprintf(reportReg->realModelId, "%s", currentNode->realModelId);
		sprintf(reportReg->hardwareVersion, "%s", currentNode->hardwareVersion);
		sprintf(reportReg->name, "%s", currentNode->name);
		sprintf(reportReg->version, "%s", currentNode->softwareVersion);

		reportReg->online = currentNode->onlineInfo->online;
		reportReg->registerStatus = currentNode->registerStatus;
		reportReg->next = NULL;
		if (head == NULL)
		{
			head = reportReg;
			tail = reportReg;
		}
		else
		{
			tail->next = reportReg;
			tail = reportReg;
		}
		count = count + 1;
		currentNode = currentNode->next;
		if (10 == count || NULL == currentNode)
		{
			count=0;
			packageReportMessageToJson(REPORT_TYPE_DEVLIST, (void*)head, outBuff);
			messageReportFun(outBuff);
			ReportRegister *deleteNode;
			ReportRegister *curNode;
			curNode = head;
			head = NULL;
			tail = NULL;
			while (curNode != NULL)
			{
				deleteNode = curNode;
				curNode = curNode->next;
				free(deleteNode);
				deleteNode = NULL;
			}

		}
	}
	pthread_mutex_unlock(&mutexLinkTable);
	return 0;
}

int handleGetDeviceListSync(char *outJsonBuff)
{
	int count = 0;

	ReportRegister *head = NULL;
	ReportRegister *tail = NULL;

	pthread_mutex_lock(&mutexLinkTable);
	DevInfo *currentNode;
	currentNode = pHead;
	if (currentNode == NULL)
	{
		pthread_mutex_unlock(&mutexLinkTable);
		packageReportMessageToJson(REPORT_TYPE_DEVLIST, NULL, outJsonBuff);
		
		return 0;
	}

	while (currentNode != NULL)
	{
		ReportRegister *reportReg;
		reportReg = (ReportRegister*)malloc(sizeof(ReportRegister));
		memset(reportReg, 0, sizeof(ReportRegister));

		sprintf(reportReg->deviceId, "%s", currentNode->deviceId);
		printf("deviceId = %s\n", reportReg->deviceId);
		sprintf(reportReg->modelId, "%s", currentNode->modelId);
		sprintf(reportReg->realModelId, "%s", currentNode->realModelId);
		sprintf(reportReg->hardwareVersion, "%s", currentNode->hardwareVersion);
		sprintf(reportReg->name, "%s", currentNode->name);
		sprintf(reportReg->version, "%s", currentNode->softwareVersion);

		reportReg->online = currentNode->onlineInfo->online;
		reportReg->registerStatus = currentNode->registerStatus;
		reportReg->next = NULL;
		if (head == NULL)
		{
			head = reportReg;
			tail = reportReg;
		}
		else
		{
			tail->next = reportReg;
			tail = reportReg;
		}
		count = count + 1;
		currentNode = currentNode->next;
		if (10 == count || NULL == currentNode)
		{
			count=0;
			packageReportMessageToJson(REPORT_TYPE_DEVLIST, (void*)head, outJsonBuff);
			
			ReportRegister *deleteNode;
			ReportRegister *curNode;
			curNode = head;
			head = NULL;
			tail = NULL;
			while (curNode != NULL)
			{
				deleteNode = curNode;
				curNode = curNode->next;
				free(deleteNode);
				deleteNode = NULL;
			}

		}
	}
	pthread_mutex_unlock(&mutexLinkTable);
	return 0;
}


int handleGetDeviceList()
{
	int res;
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

	pthread_t pGetDeviceList;
	res = pthread_create(&pGetDeviceList, &attr, (void *)getDevicesList, NULL);
	pthread_attr_destroy (&attr);
	if (res != 0)
	{
		ERROR("[ZIGBEE] Open get device list thread error\n\n");
		return -1;
	}
	return 0;
}

int handleRefactory()
{
	int res;
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
	pthread_t pRefactory;
	res = pthread_create(&pRefactory, &attr, (void *)sendCommandResetFactorySetting, NULL);
	pthread_attr_destroy(&attr);
	if (res != 0)
	{
		return -1;
	}
	return 0;
}

int handleNeighborInfo(KeyValue *keyValueHead)
{
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	DevInfo* currentNode;
	currentNode = pHead;
	int find=0;
	int res = -1;
	while (currentKeyValue != NULL)
	{
		if (strncmp("0000000000000000", currentKeyValue->address, 16) == 0)
		{
			res = sendCommandGetNeighborInfo(currentKeyValue->address);
			currentKeyValue = currentKeyValue->next;
			continue;
		}

		while (currentNode != NULL)
		{
			if (strncmp(currentNode->deviceId, currentKeyValue->address, 16) == 0)
			{
				find = 1;
				res = sendCommandGetNeighborInfo(currentKeyValue->address);
				break;
			}
			else
			{
				currentNode = currentNode->next;
			}
		}
		if (find==0 || res==-1)
			return -1;
		else
			find =0;
		currentKeyValue = currentKeyValue->next;
	}
	return res;
}

int handleChilerenInfo(KeyValue *keyValueHead)
{
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	DevInfo* currentNode;
	currentNode = pHead;
	int find=0;

	int res = -1;
	while (currentKeyValue != NULL)
	{
		if (strncmp("0000000000000000", currentKeyValue->address, 16) == 0)
		{
			res = sendCommandGetSubNodeInfo(currentKeyValue->address);
			currentKeyValue = currentKeyValue->next;
			continue;
		}

		while (currentNode != NULL)
		{
			if (strncmp(currentNode->deviceId, currentKeyValue->address, 16) == 0)
			{
				find = 1;
				res = sendCommandGetSubNodeInfo(currentKeyValue->address);
				break;
			}
			else
			{
				currentNode = currentNode->next;
			}
		}
		if (find==0 || res==-1)
			return -1;
		else
			find =0;
		currentKeyValue = currentKeyValue->next;
	}
	return res;
}

int deviceBindCommand(KeyValue *currentKeyValue)
{
	int res=-1;
	if (strcmp(currentKeyValue->key,"B")==0)
	{
		cJSON *root;
		root=cJSON_Parse(currentKeyValue->value);
		if (root!=NULL)
		{
			cJSON *function=cJSON_GetObjectItem(root,"Function");
			cJSON *srcEnd=cJSON_GetObjectItem(root,"SrcEndpoint");
			cJSON *dstAddt=cJSON_GetObjectItem(root,"DstAddr");
			cJSON *dstEnd=cJSON_GetObjectItem(root,"DstEndpoint");
			if (function!=NULL && srcEnd!=NULL && dstAddt!=NULL && dstEnd!=NULL)
			{
				res=sendBindOperationZHA(currentKeyValue->address,srcEnd->valuestring,"B",function->valuestring,\
						dstAddt->valuestring,dstEnd->valuestring);
			}
			cJSON_Delete(root);
		}
	}
	else if (strcmp(currentKeyValue->key,"U")==0)
	{
		cJSON *root;
		root=cJSON_Parse(currentKeyValue->value);
		if (root!=NULL)
		{
			cJSON *function=cJSON_GetObjectItem(root,"Function");
			cJSON *srcEnd=cJSON_GetObjectItem(root,"SrcEndpoint");
			cJSON *dstAddt=cJSON_GetObjectItem(root,"DstAddr");
			cJSON *dstEnd=cJSON_GetObjectItem(root,"DstEndpoint");
			if (function!=NULL && srcEnd!=NULL && dstAddt!=NULL && dstEnd!=NULL)
			{
				res=sendBindOperationZHA(currentKeyValue->address,srcEnd->valuestring,"U",function->valuestring,\
						dstAddt->valuestring,dstEnd->valuestring);
			}
			cJSON_Delete(root);
		}
	}
	else if(strcmp(currentKeyValue->key,"Q")==0)
	{
		res=sendBindQueryZHA(currentKeyValue->address);
	}
	return res;
}


int handleBindOperation(KeyValue *keyValueHead)
{
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	DevInfo* currentNode;
	currentNode = pHead;
	int find;

	int res = -1;
	find = 0;
	while (currentKeyValue != NULL)
	{
		while (currentNode != NULL)
		{
			if (strncmp(currentNode->deviceId, currentKeyValue->address, 16) == 0)
			{
				find = 1;
				res = deviceBindCommand(currentKeyValue);
				break;
			}
			else
			{
				currentNode = currentNode->next;
			}
		}

		if (find == 0 || res == -1)
		{
			return -1;
		}
		currentKeyValue = currentKeyValue->next;
	}

	if (find == 1 && res == 0)
		return 0;
	else
		return -1;

}

int deviceGroupCommand(KeyValue *currentKeyValue)
{
	int res=-1;
	if (strcmp(currentKeyValue->key,"A")==0)
	{
		int type;
		type=getDevRexType(currentKeyValue->address);
		if(type!=-1)
		{
			groupAdd(currentKeyValue->address,currentKeyValue->endpoint, currentKeyValue->value, type);

			res=sendGroupOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,currentKeyValue->value);
		}
	}
	else if (strcmp(currentKeyValue->key,"D")==0)
	{
		groupDeleteItem(currentKeyValue->address,currentKeyValue->endpoint, currentKeyValue->value);
		res=sendGroupOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key,"C")==0)
	{
		groupDeleteItem(currentKeyValue->address,currentKeyValue->endpoint,NULL);
		res=sendGroupOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,NULL);
	}
	else if (strcmp(currentKeyValue->key,"Q")==0)
		res=sendGroupOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,NULL);
	return res;

}


int handleGroupOperation(KeyValue *keyValueHead)
{
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	DevInfo* currentNode;
	currentNode = pHead;
	int find;

	int res = -1;
	find = 0;
	while (currentKeyValue != NULL)
	{
		while (currentNode != NULL)
		{
			if (strncmp(currentNode->deviceId, currentKeyValue->address, 16) == 0)
			{
				find = 1;
				res = deviceGroupCommand(currentKeyValue);
				break;
			}
			else
			{
				currentNode = currentNode->next;
			}
		}

		currentNode=pHead;
		if (find == 0 || res == -1)
		{
			return -1;
		}
		currentKeyValue = currentKeyValue->next;
	}

	if (find == 1 && res == 0)
		return 0;
	else
		return -1;

}


int handleMulticast(KeyValue *keyValueHead)
{
	int res=-1;
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	while (currentKeyValue != NULL)
	{
		res=groupCtrlCommand(currentKeyValue->groupId,currentKeyValue->type,currentKeyValue->key,currentKeyValue->value);
		currentKeyValue = currentKeyValue->next;
	}

	return res;
}

int deviceSceneCommand(KeyValue *currentKeyValue)
{
	int res=-1;
	cJSON *root=cJSON_Parse(currentKeyValue->value);
	cJSON *groupId=cJSON_GetObjectItem(root,"GroupId");
	cJSON *sceneId=cJSON_GetObjectItem(root,"SceneId");
	cJSON *sceneName=cJSON_GetObjectItem(root,"SceneName");
	cJSON *buttonCode=cJSON_GetObjectItem(root,"ButtonCode");

	if (strcmp(currentKeyValue->key,"A")==0)
		sendSceneOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,groupId->valuestring,\
				sceneId->valuestring,NULL,NULL);
	else if (strcmp(currentKeyValue->key,"S")==0)
	{
		if (buttonCode!=NULL)
			sendSceneOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,groupId->valuestring,\
					sceneId->valuestring,NULL,buttonCode->valuestring);
		else
			sendSceneOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,groupId->valuestring,\
					sceneId->valuestring,sceneName->valuestring,NULL);
	}

	else if (strcmp(currentKeyValue->key,"E")==0)
		sendSceneOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,groupId->valuestring,\
				sceneId->valuestring,NULL,NULL);
	else if (strcmp(currentKeyValue->key,"D")==0)
		sendSceneOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,groupId->valuestring,\
				sceneId->valuestring,NULL,NULL);
	else if (strcmp(currentKeyValue->key,"C")==0)
		sendSceneOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,currentKeyValue->value,\
				NULL,NULL,NULL);
	else if (strcmp(currentKeyValue->key,"Q")==0)
		sendSceneOperation(currentKeyValue->address,currentKeyValue->endpoint,currentKeyValue->key,currentKeyValue->value,\
				NULL,NULL,NULL);
	return res;
}

int handleScene(KeyValue *keyValueHead)
{
	int res=-1;
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	while (currentKeyValue != NULL)
	{
		res=deviceSceneCommand(currentKeyValue);
		currentKeyValue = currentKeyValue->next;
	}

	return res;
}


int airConditionerBindCommand(KeyValue *currentKeyValue)
{
	int res=-1;

	char outBuff[1260];
	cJSON *root=NULL;
	cJSON *pannelAddr;
	cJSON *endpoint;

	if (strcmp(currentKeyValue->key,"A")==0 || strcmp(currentKeyValue->key,"D")==0)
	{
		root=cJSON_Parse(currentKeyValue->value);
		pannelAddr=cJSON_GetObjectItem(root,"PannelAddr");
		endpoint=cJSON_GetObjectItem(root,"Endpoint");
	}

	if (strcmp(currentKeyValue->key,"A")==0)
	{
		res=bindAddItem(currentKeyValue->address,atoi(endpoint->valuestring),pannelAddr->valuestring);
		packageAirBindResult(currentKeyValue->address, "A", pannelAddr->valuestring,endpoint->valuestring,"00",outBuff);
	}
	else if(strcmp(currentKeyValue->key,"D")==0)
	{
		res=bindDeleteItem(currentKeyValue->address,atoi(endpoint->valuestring),pannelAddr->valuestring);
		packageAirBindResult(currentKeyValue->address, "D", pannelAddr->valuestring,endpoint->valuestring,"00",outBuff);
	}
	else if(strcmp(currentKeyValue->key,"C")==0)
	{
		res=bindClearList();
		packageAirBindResult(currentKeyValue->address, "C", NULL,NULL,"00",outBuff);
	}
	else if (strcmp(currentKeyValue->key,"Q")==0)
	{
		res=bindQueryList(currentKeyValue->address,outBuff);
	}

	messageReportFun(outBuff);
	return res;
}


int handleAirBind(KeyValue *keyValueHead)
{
	int res=-1;
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	while (currentKeyValue != NULL)
	{
		res=airConditionerBindCommand(currentKeyValue);
		currentKeyValue = currentKeyValue->next;
	}
	return res;
}

int handleSendCustomDataCommand(KeyValue *keyValueHead)
{
	int res=-1;
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;
	unsigned char bindata[1024]={0};
	unsigned char len=0;

	while (currentKeyValue != NULL)
	{
		len=base64_decode( currentKeyValue->value, bindata);
		res=rex_send_self_defining_data(currentKeyValue->address,0,atoi(currentKeyValue->endpoint),(char*)bindata,len);
		if (res==0)
			DEBUG("Custom data send success\n\n");
		else
			ERROR("Custom data send failed\n\n");
		currentKeyValue = currentKeyValue->next;
	}
	return res;
}

int subNameCommand(KeyValue *keyValueHead,char *outbuff)
{
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	ReportAttrEventOnline *repAttrHead = NULL;
	ReportAttrEventOnline *repAttrTail = NULL;
	ReportAttrEventOnline **prepAttrHead = &repAttrHead;
	ReportAttrEventOnline **prepAttrTail = &repAttrTail;

	while (currentKeyValue != NULL)
	{
		pthread_mutex_lock(&mutexLinkTable);
		DevInfo *pCurrentNode = pHead;
		if (pCurrentNode == NULL)
		{
			pthread_mutex_unlock(&mutexLinkTable);
			WARN("[ZIBGEE] Matching deviceID %s, but device list is empty!\n\n",currentKeyValue->address);
			return 0;
		}

		while (pCurrentNode != NULL)
		{
			if(strcmp(pCurrentNode->deviceId,currentKeyValue->address)==0)
			{
				if (strcmp(currentKeyValue->key,"SetName")==0)
				{

					sprintf(pCurrentNode->name,"%s",currentKeyValue->value);

					/*插入数据库*/
					int res = 0;
					char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
					sprintf(sqlStr, "UPDATE Table_device_list SET name=\"%s\" WHERE deviceId=\"%s\";", \
							currentKeyValue->value, currentKeyValue->address);

					res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
					if (res != SQLITE_OK)
					{
						ERROR("[ZIGBEE] Update  device %s name into database failed: %s\n\n", currentKeyValue->address, zErrMsg);
						sqlite3_free(zErrMsg);
					}
				}

				addNewAttributeReportNodeToAttributeReportList(currentKeyValue->address, pCurrentNode->modelId, "Name", pCurrentNode->name, prepAttrHead, prepAttrTail);
				break;
			}

			pCurrentNode=pCurrentNode->next;
		}

		pthread_mutex_unlock(&mutexLinkTable);

		currentKeyValue=currentKeyValue->next;

	}

	if (repAttrHead!=NULL)
	{

		packageReportMessageToJson(REPORT_TYPE_SUBNAME, repAttrHead, outbuff);
		freeReportAttrList(repAttrHead);
	}

	return 0;
}

int handleSubNameCommand(KeyValue *keyValueHead,char *outbuff)
{
	int res=-1;
	char tmpbuff[10000]={0};
	subNameCommand(keyValueHead, tmpbuff);
	if (outbuff == NULL)
	{
		if (0!=strlen(tmpbuff))
		{
			pthread_attr_t attr;
			pthread_attr_init(&attr);
			pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
			pthread_t pReport;
			res = pthread_create(&pReport, &attr, (void *)threadReport, (void *)tmpbuff);
			usleep(20000);
			pthread_attr_destroy(&attr);
			if (res != 0)
			{
				ERROR("[ZIGBEE] Open get device name thread error\n\n");
				return -1;
			}
			else
				res=0;
		}
	}
	else
	{
		if (0!=strlen(tmpbuff))
		{
			sprintf(outbuff,"%s",tmpbuff);
			res=0;
		}
		else
			res=-1;
	}
	return res;
}

int deviceOtaCommand(KeyValue *node)
{
	sprintf(tmp_devFilePath,"%s",node->value);
	return sendCommandDevOta(node->address,node->value);
}

int handleDevOtaCommand(KeyValue *keyValueHead,char *outbuff)
{
	int res=-1;
	KeyValue *currentKeyValue;
	currentKeyValue = keyValueHead;

	while (currentKeyValue != NULL)
	{
		res=deviceOtaCommand(currentKeyValue);
		currentKeyValue = currentKeyValue->next;
	}
	return res;
}



int getDevicesInfoList()
{
	int count = 0;
	int totleCount=0;;

	pthread_mutex_lock(&mutexLinkTable);
	DevInfo *currentNode;
	currentNode = pHead;


	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Command","Report");
	cJSON_AddStringToObject(root,"FrameNumber","00");
	cJSON_AddStringToObject(root,"GatewayId",cooMac);
	cJSON_AddStringToObject(root,"Type","DevsInfo");
	char tmp[4]={0};
	sprintf(tmp, "%d", devcieNumber);
	cJSON_AddStringToObject(root, "TotalNumber", tmp);

	cJSON *data;
	cJSON *item;
	cJSON *params;
	cJSON *paramItem;
	cJSON_AddItemToObject(root,"Data",data=cJSON_CreateArray());

	if (currentNode == NULL)
	{
		pthread_mutex_unlock(&mutexLinkTable);
		cJSON_AddStringToObject(root, "AlreadyReportNumber", "0");
		char *jsonStr;
		jsonStr=cJSON_PrintUnformatted(root);

		messageReportFun(jsonStr);
		cJSON_Delete(root);
		free(jsonStr);
		return 0;
	}

	while (currentNode != NULL)
	{
		item=cJSON_CreateObject();
		cJSON_AddStringToObject(item,"DeviceId",currentNode->deviceId);
		cJSON_AddStringToObject(item,"ModelId",currentNode->modelId);
		cJSON_AddStringToObject(item,"RealModelId",currentNode->realModelId);
		cJSON_AddStringToObject(item,"Name",currentNode->name);
		cJSON_AddStringToObject(item,"Version",currentNode->softwareVersion);
		cJSON_AddStringToObject(item,"HardwareVersion",currentNode->hardwareVersion);
		char status[2]={0};
		sprintf(status,"%d",currentNode->onlineInfo->online);
		cJSON_AddStringToObject(item,"Online",status);
		sprintf(status,"%d",currentNode->registerStatus);
		cJSON_AddStringToObject(item,"RegisterStatus",status);

		cJSON_AddItemToObject(item,"Params",params=cJSON_CreateArray());
		Attribute *paramNode=currentNode->attrHead;
		while(paramNode!=NULL)
		{
			paramItem=cJSON_CreateObject();
			cJSON_AddStringToObject(paramItem,"Key",paramNode->key);
			cJSON_AddStringToObject(paramItem,"Value",paramNode->value);
			cJSON_AddItemToArray(params,paramItem);
			paramNode=paramNode->next;
		}
		cJSON_AddItemToArray(data,item);

		count = count + 1;
		currentNode = currentNode->next;
		if (10 == count || NULL == currentNode)
		{
			totleCount=totleCount+count;
			char reportNUm[4]={0};
			sprintf(reportNUm,"%d",totleCount);
			cJSON_DeleteItemFromObject(root,"AlreadyReportNumber");
			cJSON_AddStringToObject(root, "AlreadyReportNumber", reportNUm);
			count=0;
			char *jsonStr;
			jsonStr=cJSON_PrintUnformatted(root);

			messageReportFun(jsonStr);
			cJSON_DeleteItemFromObject(root,"Data");
			free(jsonStr);
			cJSON_AddItemToObject(root,"Data",data=cJSON_CreateArray());
		}

	}
	pthread_mutex_unlock(&mutexLinkTable);

	cJSON_Delete(root);
	return 0;
}

int handleGetDeviceInfoListSync(char *outBuffJson)
{
	int count = 0;
	int totleCount=0;;

	pthread_mutex_lock(&mutexLinkTable);
	DevInfo *currentNode;
	currentNode = pHead;


	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Command","Report");
	cJSON_AddStringToObject(root,"FrameNumber","00");
	cJSON_AddStringToObject(root,"GatewayId",cooMac);
	cJSON_AddStringToObject(root,"Type","DevsInfo");
	char tmp[4]={0};
	sprintf(tmp, "%d", devcieNumber);
	cJSON_AddStringToObject(root, "TotalNumber", tmp);

	cJSON *data;
	cJSON *item;
	cJSON *params;
	cJSON *paramItem;
	cJSON_AddItemToObject(root,"Data",data=cJSON_CreateArray());

	if (currentNode == NULL)
	{
		pthread_mutex_unlock(&mutexLinkTable);
		cJSON_AddStringToObject(root, "AlreadyReportNumber", "0");
		char *jsonStr;
		jsonStr=cJSON_PrintUnformatted(root);

		
		strcpy(outBuffJson, jsonStr);
		cJSON_Delete(root);
		free(jsonStr);
		return 0;
	}

	while (currentNode != NULL)
	{
		item=cJSON_CreateObject();
		cJSON_AddStringToObject(item,"DeviceId",currentNode->deviceId);
		cJSON_AddStringToObject(item,"ModelId",currentNode->modelId);
		cJSON_AddStringToObject(item,"RealModelId",currentNode->realModelId);
		cJSON_AddStringToObject(item,"Name",currentNode->name);
		cJSON_AddStringToObject(item,"Version",currentNode->softwareVersion);
		cJSON_AddStringToObject(item,"HardwareVersion",currentNode->hardwareVersion);
		char status[2]={0};
		sprintf(status,"%d",currentNode->onlineInfo->online);
		cJSON_AddStringToObject(item,"Online",status);
		sprintf(status,"%d",currentNode->registerStatus);
		cJSON_AddStringToObject(item,"RegisterStatus",status);

		cJSON_AddItemToObject(item,"Params",params=cJSON_CreateArray());
		Attribute *paramNode=currentNode->attrHead;
		while(paramNode!=NULL)
		{
			paramItem=cJSON_CreateObject();
			cJSON_AddStringToObject(paramItem,"Key",paramNode->key);
			cJSON_AddStringToObject(paramItem,"Value",paramNode->value);
			cJSON_AddItemToArray(params,paramItem);
			paramNode=paramNode->next;
		}
		cJSON_AddItemToArray(data,item);

		count = count + 1;
		currentNode = currentNode->next;
		if (10 == count || NULL == currentNode)
		{
			totleCount=totleCount+count;
			char reportNUm[4]={0};
			sprintf(reportNUm,"%d",totleCount);
			cJSON_DeleteItemFromObject(root,"AlreadyReportNumber");
			cJSON_AddStringToObject(root, "AlreadyReportNumber", reportNUm);
			count=0;
			char *jsonStr;
			jsonStr=cJSON_PrintUnformatted(root);

			strcpy(outBuffJson, jsonStr);
			cJSON_DeleteItemFromObject(root,"Data");
			free(jsonStr);
			cJSON_AddItemToObject(root,"Data",data=cJSON_CreateArray());
		}

	}
	pthread_mutex_unlock(&mutexLinkTable);

	cJSON_Delete(root);
	return 0;
}

int handleGetDeviceInfoList()
{
	int res;
	pthread_attr_t attr;
	pthread_attr_init (&attr);
	pthread_attr_setdetachstate (&attr, PTHREAD_CREATE_DETACHED);

	pthread_t pGetDeviceList;
	res = pthread_create(&pGetDeviceList, &attr, (void *)getDevicesInfoList, NULL);
	pthread_attr_destroy (&attr);
	if (res != 0)
	{
		ERROR("[ZIGBEE] Open get devices informatoin list thread error\n\n");
		return -1;
	}
	return 0;
}

void handleReStartCmd()
{
	char outbuff[200]={0};
	packageRestartAck(outbuff);
	messageReportFun(outbuff);
	sleep(3);
	sync();
	reboot(RB_AUTOBOOT);
}

int messageProcess(char *inputJsonString,char *outJsonBuff)
{

	RecvCommand command;
	memset(&command,'\0',sizeof(RecvCommand));
	int res = -1;

	if (unpackReceiveCommandString(inputJsonString, &command) == 0)
	{
		if (strcmp(command.commandType, "Ctrl") == 0)
		{
			res = handleCtrlDeviceMsg(command.keyValueHead);
		}
		else if (strcmp(command.commandType, "Delete") == 0)
		{
			res = handleDeleteDeviceMsg(command.keyValueHead);
		}
		else if (strcmp(command.commandType, "Add") == 0)
		{

			res = handleAddDeviceMsg(command.keyValueHead);
		}
		else if (strcmp(command.commandType, "DevAttri") == 0 ||
				strcmp(command.commandType, "Attribute")==0 ||
				strcmp(command.commandType, "Query")==0)
		{

			res = handleQueryDeviceAttrMsg(command.keyValueHead, outJsonBuff);
		}
		else if (strcmp(command.commandType, "DevList") == 0)
		{
			if(NULL == outJsonBuff)
			{
				res = handleGetDeviceList();
			}
			else
			{
				res = handleGetDeviceListSync(outJsonBuff);
			}
		}
		else if (strcmp(command.commandType, "ReFactory") == 0)
		{
			res=handleRefactory();
		}
		else if (strcmp(command.commandType, "NeighborInfo") == 0)
		{
			res=handleNeighborInfo(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"ChildrenInfo") == 0)
		{
			res=handleChilerenInfo(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"Bind") == 0)
		{
			res=handleBindOperation(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"Group") == 0)
		{
			res=handleGroupOperation(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"Multicast") == 0)
		{
			res=handleMulticast(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"Scene") == 0)
		{
			res=handleScene(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"AirBind") == 0)
		{
			res=handleAirBind(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"FastJoin") == 0)
		{
			res=handleFastJoinCommand(inputJsonString);
		}
		else if  (strcmp(command.commandType,"SetSig") == 0)
		{
			res=handleSetGatewaySignaturCommand(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"GetSig")==0)
		{
			res=handleGetGatewaySignaturCommand();
		}
		else if (strcmp(command.commandType,"CustomData")==0)
		{
			res=handleSendCustomDataCommand(command.keyValueHead);
		}
		else if (strcmp(command.commandType,"SubName")==0)
		{
			res=handleSubNameCommand(command.keyValueHead,outJsonBuff);
		}
		else if (strcmp(command.commandType,"DevOta")==0)
		{
			res=handleDevOtaCommand(command.keyValueHead,outJsonBuff);
		}
		else if(strcmp(command.commandType,"DevsInfo")==0)
		{
			if(NULL == outJsonBuff)
			{
				res=handleGetDeviceInfoList();
			}
			else
			{
				res=handleGetDeviceInfoListSync(outJsonBuff);
			}
			
		}
		else if(strcmp(command.commandType,"Channel")==0)
		{
			res=sendCommandSetNetworkChannle(command.keyValueHead->value);
		}
//#ifdef MULTI_SWITCH_SYNC
		else if(strcmp(command.commandType,"SwitchGroup")==0)
		{
			res=handleSyncSwitchGroupCmd(command.keyValueHead);
		}
//#endif
		else if (strcmp(command.commandType, "Restart") == 0)
		{
			handleReStartCmd();
		}
#ifdef CTRL_LOCK
		else if (strcmp(command.commandType, "CtrlLock") == 0)
		{
			handleCtrlLockCmd(command.keyValueHead);
		}
#endif
		else
		{
			ERROR("[ZIGBEE] command Type error: %s\n\n",command.commandType);
		}
	}
	if (command.keyValueHead != NULL)
	{
		freeCommandKeyValueList(command.keyValueHead);
	}

	return res;
}


void getCooInfo(char *buff)
{
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Channel", cooChannel);
	cJSON_AddStringToObject(root, "PanId", cooPanId);
	cJSON_AddStringToObject(root, "ExtPanId", cooExtPanId);
	cJSON_AddStringToObject(root, "NetWorkKey", cooNetWorkKey);
	cJSON_AddStringToObject(root, "CooAddress", cooMac);
	char *strJson;
	strJson = cJSON_Print(root);

	sprintf(buff, "%s", strJson);
	cJSON_Delete(root);
	if (strJson!=NULL)
	{
		free(strJson);
	}
}

void getCooMac(char *macBuff)
{
	sprintf(macBuff,"%s",cooMac);
}

void getCooVersionAndCode(char *versionCode,char *version)
{
	sprintf(versionCode,"%s",cooCode);
	sprintf(version,"%s",cooVersion);
}

int loadDeviceInfo(char *devId, char *modelId)
{
	int find = 0;
	DevInfo *currentNode;
	pthread_mutex_lock(&mutexLinkTable);
	currentNode = pHead;
	while (currentNode != NULL)
	{
		if (strncmp(devId, currentNode->deviceId, 16) == 0)
		{
			find = 1;
			return 0;
		}
		else
		{
			currentNode = currentNode->next;
		}
	}
	pthread_mutex_unlock(&mutexLinkTable);

	if (find == 0)
	{
		DevInfo *newNode;
		newNode = ZHA_creatDevInfoStruct(modelId);
		if (newNode == NULL)
			return -1;
		sprintf(newNode->deviceId, "%s", devId);
		sprintf(newNode->onlineInfo->deviceId, "%s", devId);
		newNode->onlineInfo->online = 0;
		newNode->softwareVersion[0] = '\0';

		int regFlag = 1;
		char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
		sprintf(sqlStr, "REPLACE INTO Table_device_list (deviceId,devType,subType,name,modelId,reporting,hardwareVersion,\
		softwareVersion,registerStatus,secret) VALUES (\'%s\',%d,\'%s\',\'%s\',\'%s\',\'%s\',%d,\'%s\');", \
		devId, newNode->subDevType, newNode->name, modelId, "1.0.0", "20180101", regFlag, newNode->secret);
		int res=0;
		res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
		if (res != SQLITE_OK)
		{
			ERROR("[ZIGBEE] inserter new device %s information into database failed: %s\n\n", devId, zErrMsg);
			sqlite3_free(zErrMsg);
			freeDevInfoNode(newNode);
			return -1;
		}
		pthread_mutex_lock(&mutexLinkTable);
		if (pHead == NULL)
		{
			pHead = newNode;
			pTail = newNode;
		}
		else
		{
			pTail->next = newNode;
			pTail = newNode;
		}
		pthread_mutex_unlock(&mutexLinkTable);

		pthread_mutex_lock(&mutexOnlineTable);
		if (pOnlineHead == NULL && pOnlineTail == NULL)
		{
			pOnlineHead = newNode->onlineInfo;
			pOnlineTail = newNode->onlineInfo;
		}
		else
		{
			pOnlineTail->next = newNode->onlineInfo;
			pOnlineTail = newNode->onlineInfo;
		}
		pthread_mutex_unlock(&mutexOnlineTable);
	}
	sendCommandGetDeviceState(devId, 255);
	return 0;

}

void getCooVer(char *outbuff)
{
	sprintf(outbuff,"%s",cooVersion);
}

int checkDeiceOnlineStatus(char *deviceId)
{
	DevInfo *pCurrentNode;
	pCurrentNode = pHead;
	while(pCurrentNode!=NULL)
	{
		if (strcmp(deviceId,pCurrentNode->deviceId)==0)
			return pCurrentNode->onlineInfo->online;
	}
	return -1;
}


int updateDevVersionDB(char *devId,char *key,char *value)
{
	char sqlStr[SQLITE_COMMAND_MAX_LENGTH];
	sprintf(sqlStr, "UPDATE Table_device_list SET %s=\'%s\' WHERE deviceId=\'%s\';", key,value, devId);
	int res = 0;
	res = sqlite3_exec(pDB, sqlStr, NULL, NULL, &zErrMsg);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] update new device information into database error: %s\n", zErrMsg);
		sqlite3_free(zErrMsg);
		return -1;
	}
	return 0;
}
