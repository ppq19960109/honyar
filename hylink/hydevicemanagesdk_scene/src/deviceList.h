/*
 * deviceList.h
 *
 *  Created on: May 2, 2018
 *      Author: jerry
 */

#ifndef _DEVICELIST_H_
#define _DEVICELIST_H_

 /**************************************************************
 函数名：setDataBasePath
 描述：设置设备管理层数据库路径;
 参数：@const char *path：设备管理层数据库路径;
 返回：0成功，-1失败；
 **************************************************************/
int setDataBasePath(const char *path);


/**************************************************************
函数名：setConfigurFilePath
描述：设置子设备配置文件路径;
参数：@const char *path：子设备配置文件路径;
返回：0成功，-1失败；
**************************************************************/
int setConfigurFilePath(const char *path);


/**************************************************************
函数名：setFileDownloadPath
描述：设置COO固件下载、升级文件路径;
参数：@const char *path：COO固件下载、升级文件路径;
返回：0成功，-1失败；
**************************************************************/
int setFileDownloadPath(const char *path);

/**************************************************************
函数名：setOtaConfigureFilePath
描述：设置ota配置文件路径;
参数：@const char *path：ota配置文件路径;
返回：0成功，-1失败；
**************************************************************/
int setOtaConfigureFilePath(const char *path);


/**************************************************************
函数名：gatewayInit
描述：网关初始化，包括初始化设备信息链表，启动rexSDK
参数：空
返回：0成功，-1失败；
**************************************************************/
int gatewayInit();

/**************************************************************
函数名：messageProcess
描述：命令处理函数
参数：@char *inputJsonString：控制命令Json字符串，参考鸿雁对外协议说明文档；
@char *outJsonBuff：查询命令时的同步返回buff指针，outJsonBuff指针由调用方创建；
返回：0命令接收或发送成功，-1命令发送失败；
**************************************************************/
int messageProcess(char *inputJsonString,char *outJsonBuff);


/**************************************************************
函数名：getCooInfo
描述：获取COO信息
参数：@char *buff：COO信息Json字符串输出buff指针，由调用方创建；
{
"Channel":"15",
"PanId":"080F", //4位
"ExtPanId":"1234567887654321",//16位
"NetWorkKey":"123456788765432112345678",//32位
"CooAddress":"123456787654321" //16位
}
返回：空；
**************************************************************/
void getCooInfo(char *buff);


/**************************************************************
函数名：getDevModelId
描述：获取子设备ModelId
参数：	@char *devId：子设备的地址，16位字符串
		@char *outbuff：输出子设备ModelI，outbuff大小10个字符，outbuff指针由调用方创建；
返回：空；
**************************************************************/
void getDevModelId(char *devId,char *outbuff);


/**************************************************************
函数名：setDevRegisterFlag
描述：	设置Zigbee子设备注册标志位，不包括网关;
参数：	@char *devId：设备Id；
		@int registerStatus：是否已平台注册标志位，1-已经注册，0-未注册；
返回：	0成功，标志位更新成功
-1失败，查无此设备；
**************************************************************/
int setDevRegisterFlag(char *devId, int registerStatus);


/**************************************************************
函数名：cleardevManageDatabase
描述：	清空设备管理层数据库;
参数：
返回：	0成功，-1失败
 **************************************************************/
int cleardevManageDatabase();

/**************************************************************
函数名：loadDeviceInfo
描述：	加载平台注册设备信息到网关，用于从平台将已注册子设备信息，加载到一个新的网关中;
参数：	@char *devId：设备Id；
		@char *modelId：设备ModelId；
返回：	0成功，-1失败
**************************************************************/
int  loadDeviceInfo(char *devId, char *modelId);

/**************************************************************
函数名：getCooMac
描述：	获取COO的MAC;
参数：	@char *macBuff：mac的输出buff；
返回：	空
**************************************************************/
void getCooMac(char *macBuff);

/**************************************************************
函数名：getCooVer
描述：	获取COO的版本;
参数：	@char *outbuff：版本输出buff；
返回：	空
**************************************************************/
void getCooVer(char *outbuff);

/**************************************************************
函数名：getCooVersionAndCode
描述：	获取COO的版本代码和版本号;
参数：	@char *versionCode：版本代号；
        @char *version：版本日期
返回：	空
**************************************************************/
void getCooVersionAndCode(char *versionCode,char *version);

/**************************************************************
函数名：checkDeiceOnlineStatus
描述：查询设备在线状态。
参数：@char *deviceId:设备Id
返回：-1设备ID错误或无该设备，0 离线，1 在线；
**************************************************************/
int checkDeiceOnlineStatus(char *deviceId);

/**************************************************************
函数名：updateDevVersionDB
描述：更新设备数据库版本信息。
参数：@char *deviceId:设备Id
@char *key：softwareVersion 或 hardwareVersion
@char *value:
返回：-0成功，-1 失败；
**************************************************************/
int updateDevVersionDB(char *devId,char *key,char *value);

#endif /* SRC_DEVICELIST_H_ */
