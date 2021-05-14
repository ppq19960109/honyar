#ifndef _HYPROTOCOL_H_
#define _HYPROTOCOL_H_

#include "common.h"
#include "cJSON.h"

/************************************************************
结构体名：注册信息上报结构体
*************************************************************/
typedef struct ReportRegister
{
	char deviceId[DEVICE_DEVICE_ID_LENGTH];	/*address*/
	char secret[DEVICE_SECRET_LENGTH];		/*TRD中定义的secret*/
	char modelId[DEVICE_MODEL_ID_LENGTH];						/*子设备modelId*/
	char name[DEVICE_NAME_LENGTH];
	char subDevType[8];						/*Rex子设备类型*/
	char version[12];						/*版本号如20170807*/
	char hardwareVersion[20];
	int totalNum;							
	int currentNum;
	int online;								/*是否在线，0不在线，1在线*/
	int registerStatus;						/*是否已经注册，0平台未注册，1已注册*/
	char realModelId[DEVICE_MODEL_ID_LENGTH];
	struct ReportRegister *next;
}ReportRegister;


/************************************/
//#define KEY_MAX_LENGTH (24)
#define VALUE_MAX_LENGTH (32)


/************************************************************
结构体名：属性、事件、上下线上报信息结构体
************************************************************/
typedef struct ReportAttrEventOnline
{
	char deviceId[DEVICE_DEVICE_ID_LENGTH];/*address*/
	char modelId[DEVICE_NAME_LENGTH];
	char key[ATTRIBUTE_KEY_MAX_LENGTH];				/*属性名*/
	char value[ATTRIBUTE_VALUE_MAX_LENGTH];			/*属性值*/
	struct ReportAttrEventOnline *next;
}ReportAttrEventOnline;


/**************************************************************/
#define REPORT_TYPE_REGISTER		"Register"
#define REPORT_TYPE_UNREGISTER		"UnRegister"
#define REPORT_TYPE_ATTRIBUTE		"Attribute"
#define REPORT_TYPE_EVENT			"Event"
#define REPORT_TYPE_ONOFF			"OnOff"
#define REPORT_TYPE_REFACTORY		"ReFactory"
#define REPORT_TYPE_DEVLIST			"DevList"
#define REPORT_TYPE_DEVSINFO		"DevsInfo"
#define REPORT_TYPE_DEVATTRI		"DevAttri"
#define REPORT_TYPE_COO_INFO		"CooInfo"
#define REPORT_TYPE_NEIGHBOR_INFO	"NeighborInfo"
#define REPORT_TYPE_CHILDREN_INFO	"ChildrenInfo"
#define REPORT_TYPE_BIND	"Bind"
#define REPORT_TYPE_CUSTOM	"CUSTOM"
#define REPORT_TYPE_SUBNAME	"SubName"
#define REPORT_TYPE_EXT_QUERY	"Query"


/**************************************************************
函数名：packageReportMessageToJson
描述：打包上报信息，被deviceList.c调用;
参数：@const char *reportType：上报信息类型;
		#define REPORT_TYPE_REGISTER		"Register"
		#define REPORT_TYPE_UNREGISTER		"UnRegister"
		#define REPORT_TYPE_ATTRIBUTE		"Attribute"
		#define REPORT_TYPE_EVENT			"Event"
		#define REPORT_TYPE_ONOFF			"OnOff"
		#define REPORT_TYPE_REFACTORY		"ReFactory"
		#define REPORT_TYPE_DEVLIST			"DevList"

	  @void* reportMsgStruct：上报信息结构体指针，将ReportAttrEventOnline * 或ReportRegister *的结构体指针变为空指针；
	  @char *outBuff：输出打包信息结构体，由调用方创建；
返回：0成功，-1失败；
**************************************************************/
int packageReportMessageToJson(const char *reportType, void* reportMsgStruct, char *outBuff);



/**************************************************************
函数名：freeReportAttrList
描述：释放上报的属性结构体链表。
参数：@ReportAttrEventOnline *reportAttrHead：属性链表头指针;
返回：空；
**************************************************************/
void freeReportAttrList(ReportAttrEventOnline *reportAttrHead);



/************************************************************
结构体名：下发命令的键值对结构体
************************************************************/

typedef struct KeyValue
{
	char address[DEVICE_DEVICE_ID_LENGTH];
	char groupId[DEVICE_GROUP_ID_LEN];
	char key[ATTRIBUTE_KEY_MAX_LENGTH];
	char type[10];
	char name[48];
	char enable[8];
	char value[1024];
	char endpoint[4];
	struct KeyValue *next;
}KeyValue;


/************************************************************
结构体名：接收到的命令结构体，其中控制查询参数采用链表形式结构体
************************************************************/
typedef struct RecvCommand
{
	char commandType[20];/*Ctrl,Delete,Add,Query*/
	KeyValue *keyValueHead;
	int number;/*keyValue的个数*/
}RecvCommand;


/**************************************************************
函数名：unpackReceiveCommandString
描述：解析平台下发命令。
参数：@char *recvJsonString：平台下发命令字符串指针;
      @RecvCommand *command：返回解析后的命令结构体指针，
返回：0成功，-1失败
**************************************************************/
int unpackReceiveCommandString(char *recvJsonString, RecvCommand *command);


/**************************************************************
函数名：freeCommandKeyValueList
描述：释放unpackReceiveCommandString产生的属性键值对结构体链表。
参数：@KeyValue *keyValueHead：键值对结构体链表头指针;
返回：空
**************************************************************/
void freeCommandKeyValueList(KeyValue *keyValueHead);

/**************************************************************
函数名：packageBindTalbeToJson
描述：打包标准ZHA辅控开关绑定关系查询结果上报消息。
参数：@char *deviceId：辅控开关地址;
     @char *modelId：辅控开关modelid;
     @cJSON *rexBindTable: 异步上报的绑定关系表josn;
     @char *outBuff：打包结果json字符串输出缓存，
返回：空
**************************************************************/
void packageBindTalbeToJson(char *deviceId, char* modelId, cJSON *rexBindTable,char *outBuff);

/**************************************************************
函数名：packageGroupResult
描述：打包标准分组管理指令的结果上报消息。
参数：@char *deviceId：设备关地址;
     @char *modelId：modelid;
     @char *key：指令类型，A添加分组，D删除分组，C删除所有分组，Q查询分组;
     @char *groupid:组号
     @char *result：操作结果
     @char *outBuff：打包结果json字符串输出缓存，
返回：空
**************************************************************/
void packageGroupResult(char *deviceId, char* modelId, char *key, char *groupid,char *result,unsigned char endpoint_id,char*outBuff);

/**************************************************************
函数名：packageGroupNumber
描述：
参数：@char *deviceId：设备关地址;
     @char *modelId：modelid;
     char *key：指令类型，A添加分组，D删除分组，C删除所有分组，Q查询分组;
     @cJSON *number
     @char *outBuff：打包结果json字符串输出缓存，
返回：空
**************************************************************/
void packageGroupNumber(char *deviceId, char* modelId, char *key, cJSON *number,unsigned char endpoint_id,char*outBuff);


/**************************************************************
函数名：packageSceneResult
描述：打包标准场景管理指令的结果上报消息。
参数：@char *deviceId：设备关地址;
     @char *modelId：modelid;
     char *key：指令类型，A添加场景号，S修改场景名称或图片，D删除场景号，C删除所有场景号，Q查询场景;
     @char *result：操作结果
     @char *outBuff：打包结果json字符串输出缓存，
返回：空
**************************************************************/
void packageSceneResult(char *deviceId, char* modelId, char *key, char *result,char*outBuff);

/**************************************************************
函数名：packageUnknowDeviceInfo
描述：未知设备入网信息上报。
参数：@char *deviceId：设备mac;
     @char *modelId:未知设备ModelId;
     @char *type: 未知设备瑞瀛设备类型;
     @char *outBuff:输出json buff地址
返回：空
**************************************************************/
void packageUnknowDeviceInfo(char *deviceId, char* modelId,char *type,char *outBuff);

/**************************************************************
函数名：packageNeworkReset
描述：恢复web界面参数上报。
参数： @char *outBuff:输出json buff地址
返回：空
**************************************************************/
void  packageNeworkReset(char *outBuff);

/**************************************************************
函数名：packageNeworkReset
描述：恢复web界面参数上报。
参数： @char *deviceId:设备ID;
@char *modelId:modelId;
@char *errorCOde:错误码
char *outbuff:输出json buff地址
返回：空
**************************************************************/
void packageDeviceOtaResult(char *deviceId,char *modelId,char *errorCOde,char *outbuff);

/**************************************************************
函数名：packageRestartAck
描述：软重启上报。
参数：
char *outbuff:输出json buff地址
返回：空
**************************************************************/
void packageRestartAck(char *outbuff);
#endif
