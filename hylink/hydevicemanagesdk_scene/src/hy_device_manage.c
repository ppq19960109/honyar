
#include <stdio.h>
#include <string.h>
#include "sqlite3.h"
#include "cJSON.h"
#include "rex_export_gateway.h"
#include "rex_export_type.h"
#include "common.h"
#include "deviceConfigure.h"
#include "led.h"
#include "callback.h"
#include "deviceList.h"
#include "rexCommand.h"
#include "hy_device_manage.h"
#include "scene.h"
#include "scene_list.h"
#include "log.h"
#include "error_no.h"
#include "json_key.h"


#define LOCAL_SCENE_COMMAND			0xF0
#define LOCAL_SYNERGY_COMMAND		0xF1

#define BCAST_SYN_SEND_DATA			0xE0
#define DEST_SYN_SEND_DATA			0xE1


/*******************************日志静态变量*********************************/
static char logPatch[50]={0};
static log_level_t enLog_Level=LOG_DEBUG;

/*场景类指针声明*/
extern scene_list_class_t* g_pstSceneList;

/*注册上报回调函数*/
void hySetReportFunHandler(void* funName)
{

	setMessageReportFunHandler(funName);
	/*设置本地场景回调接口*/
	scene_report_reg(funName);
}

/*注册：复位按键长按8s的事件回调函数*/
void hySetResetButen8sFunHandler(void* funName)
{
	serResetButen8sFunHandler(funName);
}

static  void initlogOutput()
{
	InitLog(enLog_Level, logPatch);
}

/*设置设备管理层数据路径*/
int hySetDeviceManageDataBasePath(const char* dataBasePath)
{
	initlogOutput();
	return setDataBasePath(dataBasePath);
}

/*设置子设备配置文件路径路径*/
int hySetConfigurFilePath(const char* configurFilePath)
{
	return setConfigurFilePath(configurFilePath);
}

/*设置COO文件下载和升级文件路径*/
int hySetFileDownLoadPath(const char* fileDownloadPath)
{
	return setFileDownloadPath(fileDownloadPath);
}

/*设置ota配置文件路径
int hySetOtaConfigureFilePath(const char* otaConfigureFilePath)
{
	return setOtaConfigureFilePath(otaConfigureFilePath);
}
*/

int hySetLogInit(int log_level,const char* logFilePath)
{
	int res=-1;
	enLog_Level=log_level;

	if (logFilePath==NULL || logFilePath[0]=='\0')
		memset(logPatch,0,50);
	else
	{
		if (strlen(logFilePath)>=50)
		{
			printf("[ERROR] log path length >50 charactors\n\n");
			return res;
		}
		else
			sprintf(logPatch,"%s",logFilePath);
	}
	return 0;
}

/**************************************************************
函数名：hySceneInit
描述：本地化场景初始化
参数：@char *dataBasePath：数据库路径
*	  @int  eventDrivenType：0表示事件来源由内部zigbee上报产生
*							1表示事件来源由外部传入
返回：0初始化成功，-1失败；
**************************************************************/
int hyLocalSceneInit(const char* dataBasePath, int  eventDrivenType)
{
	return  scene_init(dataBasePath, eventDrivenType);
}

/**************************************************************
函数名：hySceneDestroy
描述：销毁本地场景的初始化
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int hyLocalSceneDestroy(void)
{
	return scene_destroy();
}

/**************************************************************
函数名：hySceneEventHandler
描述：告知场景，事件发生
参数：@char* devId：设备IEEE地址
	  @char* key：设备键名
	  @char* value：设备键值
返回：空；
**************************************************************/
void hyLocalSceneEventHandler(char* devId, char* key, char* value)
{
	scene_event2(devId, key, value);
}


/**************************************************************
函数名：hyLocalSceneActionCbReg
描述：注册场景动作回调
参数：@int iCbid：回调ID
	  @void* pFun：回调函数指针
返回：空；
**************************************************************/
int hyLocalSceneActionCbReg(int iCbid, void *pFun, void *pUserData)
{
	return scene_action_cb_reg(iCbid, pFun, pUserData);
}

/*y运行网关*/
int hyGatewayRun()
{
	int res=0;
	initlogOutput();
	res=gatewayInit();
	return res;
}

int checkEndFlag(char *inputBuff,int inputSize)
{
	int i=0;
	for (i=inputSize-1;i>=0;i--)
	{
		if (inputBuff[i]=='\0')
			return 0;
	}
	ERROR("[ZIGBEE] The input string has no ending '\0\n\n");
	return -1;
}
/*解析下发指令的类型*/
static int hyMsgProcess_QueryFlag(char *inputBuff, int *piLoaclFlag)
{
	int iValueType = 0;
	char acType[TYPE_MAX_LEN] = {0};
	
	cJSON *pstJson = NULL;
	cJSON *pstDataArry = NULL;
	cJSON *pstData = NULL;

	*piLoaclFlag = 0xFF;
	
	pstJson = cJSON_Parse(inputBuff);
	if(NULL == pstJson)
	{
		ERROR("The json format error.\n");
		return -1;
	}

	if(ERROR_SUCCESS == JSON_value_get(
		JSON_KEY_TYPE, 
		acType, 
		TYPE_MAX_LEN, 
		NULL,
		&iValueType,
		pstJson)
	)
	{
		if(!strncmp(acType,
			JSON_VALUE_TYPE_SCENE, 
			TYPE_MAX_LEN)
		)
		{
			/*场景相关处理指令*/
			*piLoaclFlag = LOCAL_SCENE_COMMAND;
		}
		else if(!strncmp(acType,
			JSON_VALUE_TYPE_DELETE, 
			TYPE_MAX_LEN)
		)
		{
			if(NULL == g_pstSceneList)
			{
				cJSON_Delete(pstJson);
				return 0;
			}
			/*收到删除子设备的指令，自动删除场景中与之相关的配置*/
			if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_DATA, 
					NULL, 
					0, 
					&pstDataArry, 
					&iValueType, 
					pstJson
				)
			)
			{
				ERROR("The json format error.\n");
				cJSON_Delete(pstJson);
				return 0;
			}
			else
			{
				if(cJSON_Array != iValueType)
				{
					ERROR("The json format error.\n");
					cJSON_Delete(pstJson);
					return 0;
				}
			}
			
			int i = 0;
			int iCount = cJSON_GetArraySize(pstDataArry);
			for(i = 0; i < iCount; i++)
			{
				if(NULL != (pstData = cJSON_GetArrayItem(pstDataArry,i)))
				{
					char acDevId[DEV_ID_MAX_LEN] = {0};
					
					if(ERROR_SUCCESS != JSON_value_get(
						JSON_KEY_DATA_DAVICE_ID, 
						acDevId, 
						DEV_ID_MAX_LEN, 
						NULL,
						&iValueType,
						pstData)
					)
					{
						ERROR("The json format error.\n");
						continue;
					}

					g_pstSceneList->scene_dev_unregister(g_pstSceneList, acDevId);
				}
			}
		}
	}

	if(NULL != pstJson)
	{
		cJSON_Delete(pstJson);
	}
	
	return 0;
}


/*接收处理命令，返回状态码
@int fd：可用于socket fd，做为同步信息回传字描述符，若不用置为-1；
@char *inputJsonString：Json字符串；
返回：0命令接收或发送成功，-1命令发送失败；
*/
int hyMsgProcess_SyncQuery(char *inputBuff,int inputSize, char *outBuff,int outSize,int*outLen)
{
	if(0!=checkEndFlag(inputBuff,inputSize))
	{
		return -1;
	}

	if (NULL==inputBuff || NULL==outBuff)
	{
		ERROR("[ZIGBEE] inputBuff or outBuff is NULL\n\n");
		return -1;
	}

	int res=-1;
	char tmp[10000]={0};
	int iLoaclFlag = 0xFF;
	
	/*获取请求标志位*/
	hyMsgProcess_QueryFlag(inputBuff, &iLoaclFlag);
	/*向本地网关发送数据*/
	if (LOCAL_SCENE_COMMAND == iLoaclFlag)
	{
		res = scene_query(inputBuff, inputSize, outBuff, outSize, outLen);
	}
	else
	{

		if(messageProcess(inputBuff,tmp)==0)
		{

			*outLen=strlen(tmp);
			if (*outLen>=outSize)
			{
				ERROR("[ZIGBEE] Actual output length(%d) is longer than outSize (%d)\n\n",*outLen,outSize);
				res=-1;
			}
			else if (0!=*outLen)
			{
				sprintf(outBuff,"%s",tmp);
				res=0;
			}
			else
				res=0;
		}
	}
	return res;
}


int hyMsgProcess_AsynQuery(char *inputBuff,int inputSize)
{

	if(0!=checkEndFlag(inputBuff,inputSize))
	{
		return -1;
	}

	if (NULL==inputBuff)
	{
		ERROR("[ZIGBEE] inputBuff is NULL\n\n");
		return -1;
	}
	inputBuff[inputSize-1]='\0';

	int iLoaclFlag = 0xFF;
	
	/*获取请求标志位*/
	hyMsgProcess_QueryFlag(inputBuff, &iLoaclFlag);
	/*向本地网关发送数据*/
	if (LOCAL_SCENE_COMMAND == iLoaclFlag)
	{
		return scene_query(inputBuff, inputSize, NULL, 0, NULL);
	}
	else
	{
		return messageProcess(inputBuff,NULL);
	}
	
}

void hyGetDeviceModelId(char *devId,char *outbuff)
{
	getDevModelId(devId,outbuff);
}

int hySetDeviceRegisterStatus(char *devId, int registerStatus)
{
	return setDevRegisterFlag(devId,registerStatus);
}


void hyGetCooInfo(CooInfo *cooInformation)
{
	char outbuff[300];
	getCooInfo(outbuff);
	cJSON *root;
	root=cJSON_Parse(outbuff);
	if (root!=NULL)
	{
		cJSON *channel;
		cJSON *panId;
		cJSON *extPanId;
		cJSON *netWorkKey;
		cJSON *cooAddress;
		channel=cJSON_GetObjectItem(root,"Channel");
		sprintf(cooInformation->channel,"%s",channel->valuestring);
		panId=cJSON_GetObjectItem(root,"PanId");
		sprintf(cooInformation->panId,"%s",panId->valuestring);
		extPanId=cJSON_GetObjectItem(root,"ExtPanId");
		sprintf(cooInformation->extPanId,"%s",extPanId->valuestring);
		netWorkKey=cJSON_GetObjectItem(root,"NetWorkKey");
		sprintf(cooInformation->netWorkKey,"%s",netWorkKey->valuestring);
		cooAddress=cJSON_GetObjectItem(root,"CooAddress");
		sprintf(cooInformation->cooAddress,"%s",cooAddress->valuestring);
		cJSON_Delete(root);
	}
}

int hySetCooInfo(CooInfo *cooInformation)
{
	 return sendCommandSetZigbeeNetworkParams(cooInformation->channel, cooInformation->panId,\
			 cooInformation->extPanId, cooInformation->netWorkKey);
}


int hyClearDeviceManageDataBase()
{
	return cleardevManageDatabase();
}

int hyLoadDeviceInfo(char *devId, char *modelId)
{
	return loadDeviceInfo(devId, modelId);
}

int hySendCustomData(char *deviceId,char *customData,int dataSize)
{
	if (dataSize>1024 || 1024<strlen(customData))
	{
		ERROR("[ZIGBEE] Custom data length is large than 1024\n\n");
		return -1;
	}

	int res=0;
	res=rex_send_self_defining_data(deviceId,0,1,customData,dataSize);
	if(res==0)
		INFO("[ZIGBEE] Send custom data success\n\n");
	return res;

}


void hyGetCooVersionAndCode(char*versionCode,char *version)
{
	getCooVersionAndCode(versionCode,version);
}

int hyQueryMinIntervalSet(unsigned long ulMinInterval)
{
	return setSendRexCommandInterval(ulMinInterval);
}

int hyQueryDeviceOnlineStatus(char *deviceId)
{
  return checkDeiceOnlineStatus(deviceId);
}





