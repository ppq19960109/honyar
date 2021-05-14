#ifndef _DEVICECONFIGURE_H_
#define _DEVICECONFIGURE_H_

#include "common.h"


/**************************************************************
函数名：ZHA_creatDevInfoStruct
描述：根据子设备ModelId创建设备信息链表节点结构体。
	  被deviceList.c调用，用于初始化子设备信息链表和子设备入网时创建节点;
参数：@char *modelId：子设备ModelId;
返回：成功返回DevInfo *结构体指针，失败NULL；
**************************************************************/
DevInfo *ZHA_creatDevInfoStruct(char *modelId);


/**************************************************************
函数名：ZHA_creatDevAttrStruct
描述：根据子设备ModelId创建该子设备的属性链表和属性节点个数;
参数：@char *modelId：子设备ModelId;
	  @int *attrNum：属性个数
返回：成功返回属性链表头指针Attribute *，失败NULL；
**************************************************************/
Attribute *ZHA_creatDevAttrStruct(const char *modelId, int *attrNum);

#endif
