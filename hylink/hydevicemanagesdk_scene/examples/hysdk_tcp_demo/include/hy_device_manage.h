#ifndef _DEVICEMANAGE_H_
#define _DEVICEMANAGE_H_

/*******************************************************
 函数名：hySetLogInit
 描述： 设置日志输出等级，输出文件路径
 参数：@int log_level: LOG_NONE=0(关闭日志)，
 	 	 	 	 	 LOG_FATAL=1(重大错误)，
 	 	 	 	 	 LOG_ERROR=2(错误信息)，
 	 	 	 	 	 LOG_WARN=3(警告信息)，
 	 	 	 	 	 LOG_INFO=4(情报信息)，
 	 	 	 	 	 LOG_DEBUG=5(调试信息)，
 	 	 	 	 	 LOG_TRACE=6(跟踪信息)，
 	 @const char* logFilePath:日志输出路径，请指定/tmp下的路径，如传入NULL或空，则日志输出到标准输出;
 返回： 0成功，-1失败
 */
int hySetLogInit(int log_level,const char* logFilePath);



/**************************************************************
函数名：hySetReportFunHandler
描述：设置上报回调函数
参数：@void* funName：上报回调函数名，具体由第三方用户实现：int funName(char *)，返回0成功，-1失败;
返回：空；
**************************************************************/
void hySetReportFunHandler(void* funName);


/**************************************************************
函数名：hySetResetButen8sFunHandler
描述：设置复位按键长按8s的事件回调函数，若不用则 回调函数里填  return 0;
参数：@void* funName：上报回调函数名，具体由第三方用户实现：int funName()，返回0成功，-1失败;
返回：空；
**************************************************************/
void hySetResetButen8sFunHandler(void* funName);

/**************************************************************
函数名：hySettDeviceManageDataBasePath
描述：设置设备管理层数据库路径;
参数：@const char* dataBasePath：设备管理层数据库路径,example: /usr/testlib/;
返回：0成功，-1失败；
**************************************************************/
int hySetDeviceManageDataBasePath(const char* dataBasePath);


/**************************************************************
函数名：hySetConfigurFilePath
描述：设置子设备配置文件路径;
参数：@const char* configurFilePath：子设备配置文件路径,example: /usr/testlib/configure;
返回：0成功，-1失败；
**************************************************************/
int hySetConfigurFilePath(const char* configurFilePath);


/**************************************************************
函数名：hySetFileDownLoadPath
描述：设置COO固件下载、升级文件路径;
参数：@const char* fileDownloadPath：COO固件下载、升级文件路径example: /usr/testlib/;
返回：0成功，-1失败；
**************************************************************/
int hySetFileDownLoadPath(const char* fileDownloadPath);



/**************************************************************
函数名：hyGatewayRun
描述：网关初始化，包括初始化设备信息链表，启动rexSDK
参数：空
返回：0成功，-1失败；
**************************************************************/
int hyGatewayRun();


/**************************************************************
函数名：hyMsgProcess_SyncQuery
描述：命令处理函数，查询设备全部和部分属性时同步返回（此处查询为查询网关中的内存）
参数：@char *inputJsonString：控制命令Json字符串，参考鸿雁对外协议说明文档；
	  @int inputSize：输入缓存的大小,至少为实际控制命令Json字符串长度+1；
      @char *outBuff：查询结果输出缓存，不能为空；
	  @int outSize：输出缓存大小；
	  @int*outLen，实际输出字符串大小,由调用者创建；
返回：0命令接收或发送成功，-1失败；
**************************************************************/
int hyMsgProcess_SyncQuery(char *inputBuff,int inputSize, char *outBuff,int outSize,int*outLen);

/**************************************************************
函数名：hyMsgProcess_AsynQuery
描述：命令处理函数，查询设备全部和部分属性时异步返回
参数：@char *inputJsonString：控制命令Json字符串，参考鸿雁对外协议说明文档；
	  @int inputSize：输入缓存的大小,至少为实际控制命令Json字符串长度+1；
返回：0命令接收或发送成功，-1失败；
**************************************************************/
int hyMsgProcess_AsynQuery(char *inputJsonString,int inputSize);

/**************************************************************
函数名：hyGetDeviceModelId
描述：获取子设备ModelId
参数：@char *devId：子设备的地址，16位字符串
      @char *outbuff：输出子设备ModelI，outbuff大小10个字符，outbuff指针由调用方创建；
返回：空；
**************************************************************/
void hyGetDeviceModelId(char *devId,char *outbuff);

/**************************************************************
函数名：hySetDeviceRegisterStatus
描述：	设置Zigbee子设备注册标志位，不包括网关;
参数：	@char *devId：设备Id；
		@int registerStatus：是否已平台注册标志位，1-已经注册，0-未注册；
返回：	0成功，标志位更新成功
		-1失败，查无此设备；
**************************************************************/
int hySetDeviceRegisterStatus(char *devId, int registerStatus);


/*************************************************************/
/*Coo信息结构体*/
typedef struct CooInformation
{
	char channel[4];//2位
	char panId[6];//4位
	char extPanId[18];//16位
	char netWorkKey[34];//32位
	char cooAddress[18];//16位
}CooInfo;

/**************************************************************
函数名：hyGetCooInfo
描述：获取COO信息
参数：@CooInfo *cooInformation：COO信息信息结构体，由调用方创建；
返回：空；
**************************************************************/
void hyGetCooInfo(CooInfo *cooInformation);


/**************************************************************
函数名：hySetCooInfo
描述：	设置Zigbee网络参数;
参数：	@CooInfo *cooInformation：COO信息信息结构体，由调用方创建；
返回：	0成功，-1失败；
**************************************************************/
int hySetCooInfo(CooInfo *cooInformation);



/**************************************************************
函数名：hyClearDeviceManageDataBase
描述：	清空数据库；此操作不能让已加入网关的设备离网，单纯清空数据库;
参数：	
返回：	0成功，-1失败；
**************************************************************/
int hyClearDeviceManageDataBase();


/**************************************************************
函数名：hyLoadDeviceInfo
描述：	加载平台注册设备信息到网关，用于从平台将已注册子设备信息，加载到一个新的网关中;
参数：	@char *devId：设备Id；
		@char *modelId：设备ModelId；
返回：	0成功，-1失败
**************************************************************/
int hyLoadDeviceInfo(char *devId, char *modelId);


/**************************************************************
函数名：hySendCustomData
描述：向设备发送自定义透传数据（设备必须支持透传）
参数：	@char *devId：设备Id；
@char* customData：自定义透传数据地址指针，不能超过1024字节；
@int dataSize：实际自定义透传数据大小字节大小；
返回：	0成功，-1失败；
**************************************************************/
int hySendCustomData(char *deviceId, char* customData, int dataSize);


/**************************************************************
函数名：hyGetCooVersionAndCode
描述：获取COO固件版本代码和版本日期）
参数：	@char*versionCode：版本代码输出缓存，至少8个字符；
@char *version：版本日期输出缓存；
返回：	空；
**************************************************************/
void hyGetCooVersionAndCode(char*versionCode,char *version);

/**************************************************************
函数名：hyQueryMinIntervalSet
描述：设置发送指令最小间隔，默认0ms。
参数：	@unsigned long ulMinInterval:时间间隔，单位毫秒，初始默认值为200ms
返回：	0；
**************************************************************/
int hyQueryMinIntervalSet(unsigned long ulMinInterval);


/**************************************************************
函数名：hyQueryDeviceOnlineStatus
描述：查詢設備在線狀態。
参数：	@char *deviceId:設備ID
返回：	0；
**************************************************************/
int hyQueryDeviceOnlineStatus(char *deviceId);

/*本地场景接口*/
/**************************************************************
函数名：hyLocalSceneInit
描述：本地化场景初始化
参数：@char *dataBasePath：数据库路径，数据库必须为全路径，即包括数据库文件名
	  @int  eventDrivenType: 事件驱动类型，
	  1表示外部事件驱动，即由SDK调用者提供事件传入（使用接口hyLocalSceneEventHandler）
	  	0表示内部事件驱动，即由SDK内部提供事件传入（此时接口hyLocalSceneEventHandler调用后无任何效果）
返回：0初始化成功，-1失败；
**************************************************************/
int hyLocalSceneInit(const char* dataBasePath, int  eventDrivenType);

/**************************************************************
函数名：hyLocalSceneDestroy
描述：销毁本地场景的初始化
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int hyLocalSceneDestroy(void);


/**************************************************************
函数名：hyLocalSceneEventHandler
描述：告知场景，事件发生
参数：@char* devId：设备IEEE地址
	  @char* key：设备键名
	  @char* value：设备键值
返回：空；
**************************************************************/
void hyLocalSceneEventHandler(char* devId, char* key, char* value);


#endif 
