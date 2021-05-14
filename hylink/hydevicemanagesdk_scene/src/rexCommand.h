#ifndef _REXCOMMAND_H_
#define _REXCOMMAND_H_
#include "hyprotocol.h"

/**************************************************************
函数名：initRexSendLock
描述：初始化rex发送函数线程锁;
参数：
返回：空；
**************************************************************/
void initRexSendLock();

/**************************************************************
函数名：setSendRexCommandInterval
描述：设置发送Rex指令的时间间隔;
参数：@unsigned long msInterval：时间间隔单位毫秒
返回：0；
**************************************************************/
int setSendRexCommandInterval(unsigned long msInterval);


/*REX控制命令函数*/
/**************************************************************
函数名：sendCommandSetupZigbeeNetwork
描述：建立ZigBee网络;
参数：
返回：0成功，-1失败；
**************************************************************/
int sendCommandSetupZigbeeNetwork();

/**************************************************************
函数名：sendCommandGetGatewayMAC
描述：获取子设备 MAC;
参数：
返回：0成功，-1失败；
**************************************************************/
int sendCommandGetGatewayMAC();

/**************************************************************
函数名：sendCommandGetCooVer
描述：获取COO 版本号;
参数：
返回：0成功，-1失败；
**************************************************************/
int sendCommandGetCooVer();

/**************************************************************
函数名：sendCommandAllowDevJoin
描述：允许子设备加入网关;
参数：@char* time：允许入网时间，单位秒，取值0-255；其中0关闭入网，255永久开启入网，
返回：0成功，-1失败；
**************************************************************/
int sendCommandAllowDevJoin(char* time);

/**************************************************************
函数名：sendCommandDeleteDevice
描述：注销子设备或控制子设备退出网关;
参数：@char *address：子设备地址
返回：0成功，-1失败；
**************************************************************/
int sendCommandDeleteDevice(char *address);

/**************************************************************
函数名：sendCommandGetDeviceState
描述：获取设备状态;
参数：@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
	  @int endpoint: 第几路，从1开始，255整个设备
返回：0成功，-1失败；
**************************************************************/
int sendCommandGetDeviceState(char *deviceId, int endpoint);

/**************************************************************
函数名：sendCommandCtrlSwitchState
描述：控制开关面板开/关;
参数：@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
	  @unsigned char endpoint_id: 第几路，从1开始，255整个设备
	  @char *state：开"1",关"0"
返回：0成功，-1失败；
**************************************************************/
int sendCommandCtrlSwitchState(char *address, unsigned char endpoint_id, char *state);

/**************************************************************
函数名：sendCommandCtrlLuminance
描述：	控制灯具亮度;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@unsigned char endpoint_id:第几路，从1开始，255整个设备
		@char *state：亮度百分比"0"到"100"
返回：	0成功，-1失败；
**************************************************************/
int sendCommandCtrlLuminance(char *address, unsigned char endpoint_id, char *state);

/**************************************************************
函数名：sendCommandCtrlColorTemperature
描述：	控制灯具色温;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@char *state：色温"0"到"9000"，根据具体设备而定
返回：	0成功，-1失败；
**************************************************************/
int sendCommandCtrlColorTemperature(char *address, char *state);

/**************************************************************
函数名：sendCommandCtrlHUE
描述：	控制灯具色调;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@char *state：色调"0"到"254"，
返回：	0成功，-1失败；
**************************************************************/
int sendCommandCtrlHUE(char *address, char *state);

/**************************************************************
函数名：sendCommandCtrlSaturation
描述：	控制灯具饱和度;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@char *state：饱和度"0"到"254"，
返回：	0成功，-1失败；
**************************************************************/
int sendCommandCtrlSaturation(char *address, char *state);

/**************************************************************
函数名：sendCommandCtrlHSL
描述：	控制灯具HSL;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
	@char *level：亮度百分比"0"到"100"
	@char *saturation：饱和度"0"到"254"，
	@char *hue:色调"0"到"254"，
返回：	0成功，-1失败；
**************************************************************/
int sendCommandCtrlHSL(char *address, char *level, char *saturation, char *hue);

/**************************************************************
函数名：sendCommandCurtainOperation
描述：	控制窗帘开、关、停;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@char *state：窗帘状态，"0"开启，"1"关闭，"2"停止
返回：	0成功，-1失败；
**************************************************************/
int sendCommandCurtainOperation(char *address,unsigned char endpointId, char *state);

/**************************************************************
函数名：sendCommandSetCurtainPosition
描述：	控制窗帘位置;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@char *state：窗帘位置百分比，"1-99"位置百分比，"0"全开，"100"全关
返回：	0成功，-1失败；
**************************************************************/
int sendCommandSetCurtainPosition(char *address,unsigned char endpointId,  char *state);

/**************************************************************
函数名：sendCommandSoundLightAlarm
描述：	控制声光报警器报警;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@const char *alarmType：警报类型；
		0-停止报警
		1-产生盗窃报警
		2=产生火灾报警
		3-产生紧急事件报警
		8-产生布防报警
		9-产生撤防报警
		@const char *twinkele：警灯是否闪烁，0-无警灯，1-警灯闪烁
		@const char *duration：时长 "0"-"254"
返回：	0成功，-1失败；
**************************************************************/
int sendCommandSoundLightAlarm(char *address, const char *alarmType, const char *twinkele, const char *duration);

/**************************************************************
函数名：sendCommandFreshFanMode
描述：	控制送风模式，用于新风系统、加热制冷设备、空调;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@unsigned char endpoint_id：第几路，从1开始，255整个设备
		@char *state：送风模式，视具体设备而定
		0-off;
		1-低；
		2=中；
		3-高；
		4-On;
		5-自动；//不支持
		6-智能；//不支持
返回：	0成功，-1失败；
**************************************************************/
int sendCommandFreshFanMode(char *address, unsigned char endpoint_id, char *state);

/**************************************************************
函数名：sendCommandSetSocketState
描述：	控制插座开/关;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@unsigned char endpoint_id：第几路，从1开始，255整个设备
		@char *state：状态
		0-off;
		1-On;
		2-切换；
返回：	0成功，-1失败；
**************************************************************/
int sendCommandSetSocketState(char *address, unsigned char endpoint_id, char *state);

/**************************************************************
函数名：sendCommandWorkMode
描述：	控制工作模式，用于加热制冷设备、空调;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@unsigned char endpoint_id：第几路，从1开始，255整个设备
		@char *state：状态
		0-off;
		1-自动；
		3-制冷；
		4-加热；
		5-紧急加热；//不支持
		6-预冷；//不支持
		7-送风；
		8-除湿；
		9-睡眠；//不支持
		10-用户自定义
返回：	0成功，-1失败；
**************************************************************/
int sendCommandWorkMode(char *address, unsigned char endpoint_id, char *state);

/**************************************************************
函数名：sendCommandSetTemperature
描述：	设置温度，用于加热制冷设备、空调;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@unsigned char endpoint_id：第几路，从1开始，255整个设备
		@char *state：温度，长度3个字符，"0"-"100"
返回：	0成功，-1失败；
**************************************************************/
int sendCommandSetTemperature(char *address, unsigned char endpoint_id, char *state);

/**************************************************************
函数名：sendCommandSetSwitchChildLock
描述：	设置插座童锁开关;
参数：	@char *address：子设备地址，16个"FFFFFFFFFFFFFFFF"为广播地址；
		@char *paramName: 參數名稱;
		@char *paramValue：參數值
返回：	0成功，-1失败；
**************************************************************/
int sendCommandWriteCustomParameters(char *address, char *paramName,char *paramValue);



/**************************************************************
函数名：sendCommandSetZigbeeNetworkParams
描述：	设置Zigbee网络参数;
参数：	@char *channel：网络信道，11-26；
		@char *panId：网络表示，4位16进制字符串；
		@char *extPanId：扩展网络标识，16位16进制字符串；
		@char *NetworkKey：网络密匙，32位16进制字符串；
返回：	0成功，-1失败；
**************************************************************/
int sendCommandSetZigbeeNetworkParams(char *channel, char *panId, char* extPanId, char *NetworkKey);

/**************************************************************
函数名：sendCommandGetDevVersion
描述：	获取设备版本号;
参数：	@char *devId：子设备ID；
返回：	0成功，-1失败；
**************************************************************/
int sendCommandGetDevVersion(char *devId);

/**************************************************************
函数名：sendCommandCallDeviceResponse
描述：	呼叫设备响应;
参数：	@char *devId：子设备ID；
返回：	0成功，-1失败；
**************************************************************/
int sendCommandCallDeviceResponse(char *dev);

/**************************************************************
函数名：sendCommandGetCooInfo
描述：	获取COO通道信息
参数：	空；
返回：	0成功，-1失败；
**************************************************************/
int sendCommandGetCooInfo();

/**************************************************************
函数名：sendCommandGetNeighborInfo
描述：	获取邻居信息，获取指定设备所有邻居信息及通信质量，可用于网络诊断
参数：	空；
返回：	0成功，-1失败；
**************************************************************/
int sendCommandGetNeighborInfo(char *devId);

/**************************************************************
函数名：sendCommandGetSubNodeInfo
描述：	获取子节点信息，获取指定设备所有子节点信息，即与父节点通信质量，ZED节点的通信质量，用于网络诊断
参数：	空；
返回：	0成功，-1失败；
**************************************************************/
int sendCommandGetSubNodeInfo(char *devId);

/**************************************************************
函数名：devCtrlSecurityEquipment
描述：	安防类设备，撤防/布防控制
参数：	@@KeyValue *currentKeyValue：命令键值对结构体;
		其中currentKeyValue->key="ProtectionStatus";
			currentKeyValue->value=1-布防，0-撤防；
		@DevInfo* currentNode：该设备信息节点指针；
返回：	0成功，-1失败；
**************************************************************/
int devCtrlSecurityEquipment(KeyValue *currentKeyValue, DevInfo* currentNode);


/**************************************************************
函数名：devCtrlSwitchPannel123
描述：	1、2、3位开关控制
参数：	@KeyValue *currentKeyValue：命令键值对结构体;
返回：	0成功，-1失败；
**************************************************************/
int devCtrlSwitchPannel123(KeyValue *currentKeyValue);


/**************************************************************
函数名：devCtrlSwitchDLT
描述：	DLT开/关,色温、亮度控制
参数：	@KeyValue *currentKeyValue：命令键值对结构体;
返回：	0成功，-1失败；
**************************************************************/
int devCtrlSwitchDLT(KeyValue *currentKeyValue);

/**************************************************************
函数名：devCtrlLightHSL
描述：	HSL灯具控制
参数：	@KeyValue *currentKeyValue：命令键值对结构体;
返回：	0成功，-1失败；
**************************************************************/
int devCtrlLightHSL(KeyValue *currentKeyValue);

/**************************************************************
函数名：devCtrlElectricCurtain
描述：	窗帘开关、开合帘电机控制
参数：	@KeyValue *currentKeyValue：命令键值对结构体;
返回：	0成功，-1失败；
**************************************************************/
int devCtrlElectricCurtain(KeyValue *currentKeyValue);

/**************************************************************
函数名：devCtrlSecurityAlarm
描述：	声光报警器控制
参数：	@KeyValue *currentKeyValue：命令键值对结构体;
返回：	0成功，-1失败；
**************************************************************/
int devCtrlSecurityAlarm(KeyValue *currentKeyValue);

/**************************************************************
函数名：devCtrlSocket12
描述：	插座开/关，童锁开关控制
参数：	@KeyValue *currentKeyValue：命令键值对结构体;
返回：	0成功，-1失败；
**************************************************************/
int devCtrlSocket12(KeyValue *currentKeyValue);


/**************************************************************
函数名：devCtrlFreshAirSystem
描述：	新风控制器控制
参数：	@KeyValue *currentKeyValue：命令键值对结构体;
返回：	0成功，-1失败；
**************************************************************/
int devCtrlFreshAirSystem(KeyValue *currentKeyValue);

/**************************************************************
函数名：devCtrlAirConditioner
描述：	一路或多路空调的控制
参数：	@KeyValue *currentKeyValue：命令键值对结构体;
返回：	0成功，-1失败；
**************************************************************/
int devCtrlAirConditioner(KeyValue *currentKeyValue);

int sendBindOperationZHA(char *address,char *srcEndpoint,char *key,char *function,char *dstAddr,char *dstEndpoint);

int sendBindQueryZHA(char *address);

int sendGroupOperation(char *address,char *endpoint,char *action,char *group);

int sendSceneOperation(char *address,char *endpoint,char *action,char *groupId,char *sceneId,\
		char * sceneName,char *buttonCode);

int sendCommandGetZigbeeNetworkInfo();

/**************************************************************
函数名：sendCommandCloseFastJoin
描述：	关闭快速入网指令
参数：
返回：	0成功，-1失败；
**************************************************************/
int sendCommandCloseFastJoin();

/**************************************************************
函数名：sendCommandReBuildNetwark
描述：	重新建立网络指令
参数：
返回：	0成功，-1失败；
**************************************************************/
int sendCommandReBuildNetwark();


/********************************************************
函数名： sendCommandOpenFastJoin
描  述：下发快速入网功能；
参  数：@char *command：下发地址数组JSON
返  回：0成功，-1失败
*********************************************************/
int sendCommandOpenFastJoin(char *command);

/********************************************************
函数名： sendCommandClearGatewayNetInfo
描  述：发送清除网关网络参数信息；
参  数：
返  回：0成功，-1失败
*********************************************************/
int sendCommandClearGatewayNetInfo();

/********************************************************
函数名： sendCommandBuildNetwork
描  述：建立网络；
参  数：
返  回：0成功，-1失败
*********************************************************/
int sendCommandBuildNetwork();


/********************************************************
函数名： sendCommandSetSignature
描  述：设置网关签名信息；
参  数：@const char *type:签名类型
			  0：擦出所有签名;
              1：写入ProductModel;
              2:写入ProductKey;
              3:写入ProductSecret;
              4:写入ProductName;
              5:写入DeviceSecret;
              6:写入DeviceKey;
      @char *data: 数据
返  回：0成功，-1失败
*********************************************************/
int sendCommandSetSignature(const char *type,char *data);

/********************************************************
函数名： sendCommandGetSignature
描  述：获取网关签名信息；
参  数：
返  回：0成功，-1失败
*********************************************************/
int sendCommandGetSignature();

/********************************************************
函数名： sendCommandGetMcuVersion
描  述：获取设备mcu版本；
参  数：@char *address:设备地址或mac
返  回：0成功，-1失败
*********************************************************/
int sendCommandGetMcuVersion(char *address);

/********************************************************
函数名： sendCommandGetCurtainRainSensorParam
描  述：获取窗帘电机风雨传感器属性；
参  数：@char *address:设备地址或mac;

返  回：0成功，-1失败
*********************************************************/
int sendCommandGetCurtainRainSensorParam(char *address);


/********************************************************
函数名： sendCommandSetCurtainRainSensorParam
描  述：获取窗帘电机风雨传感器属性；
参  数：@char *address:设备地址或mac;
       @char *value: 写入值 1使能风雨传感器，0关闭风雨传感器
返  回：0成功，-1失败
*********************************************************/
int sendCommandSetCurtainRainSensorParam(char *address,char *value);

/********************************************************
函数名： sendCommandGetRexSDKVersion
描  述：获取rexSDK版本号信息指令；
参  数：
返  回：0成功，-1失败
*********************************************************/
int sendCommandGetRexSDKVersion();

/********************************************************
函数名： sendCommandReadCustomParameters
描  述：查詢自定義參數值；
参  数：@char *deviceID：設備id;
       @unsigned char endpoint;
       @char *paramName1\2\3\4: 參數名稱；
返  回：0成功，-1失败
*********************************************************/
int sendCommandReadCustomParameters(char *deviceID,unsigned char endpoint,char *paramName1,char *paramName2,char *paramName3,char *paramName4);


/********************************************************
函数名： sendCommandSetJoinNetWorkMode
描  述：设置入网模式；
参  数：@int mode:1正常入网,2 复合入网,即快速入网；
返  回：0成功，-1失败
*********************************************************/
int sendCommandSetJoinNetWorkMode(int mode);

/********************************************************
函数名： sendCommandSetAtHoldRPTOD
描  述：发送AT+HoldRPTOD指令;
参  数：@
返  回：0成功，-1失败
*********************************************************/
int sendCommandSetAtHoldRPTOD();

/********************************************************
函数名： sendCommandDevOta
描  述：发送子设备OTA升级文件;
参  数：@char *deviceID:将要升级的子设备Id;
@char *filePath:子设备固件的绝对路径,形如:"/usr/app/4451_1_20200103.ebl"
返  回：0成功，-1失败
*********************************************************/
int sendCommandDevOta(char *deviceID,char *filePath);

/********************************************************
函数名： sendCommandSetNetworkChannle
描  述：设置通信信道;
参  数：@char *channel：“11”——“26”
返  回：0成功，-1失败
*********************************************************/
int sendCommandSetNetworkChannle(char *channel);

/********************************************************
函数名： sendCommandSetReportAttribute
描  述：设置属性上报;
参  数：@char *devID,\
		char *clusterId,\
		char *attributeId,\
		char *attributeType,\
		char *interval,\
		char *changeValue
返  回：0成功，-1失败
*********************************************************/
int sendCommandSetReportAttribute(char *devID,\
		char *clusterId,\
		char *attributeId,\
		char *attributeType,\
		char *interval,\
		char *changeValue);

#endif 

