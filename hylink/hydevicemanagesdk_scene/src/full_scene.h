/*
 * full_scene.h
 *
 *  Created on: Nov 5, 2019
 *      Author: jerry
 */

#ifndef SRC_FULL_SCENE_H_
#define SRC_FULL_SCENE_H_

#include "common.h"
#include "hyprotocol.h"

/**************************************************************
函数名：fullscene_GetSceneName
描述：查询场景按键名称
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetSceneName(char *devicId,char *keyNumber);

/**************************************************************
函数名：fullscene_GetScenePhoto
描述：查询场景按键图片
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetScenePhoto(char *devicId,char *keyNumber);


/**************************************************************
函数名：full_SceneCtrlCommand
描述：设置全面屏场景名称\图片\空调,新风,地暖
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int full_SceneCtrlCommand(KeyValue *currentKeyValue);

/**************************************************************
函数名：full_HandleSceneCustomReport
描述：处理全面屏自定义数据上报解析
参数：@char *data：输入数据首地址;
@ unsigned char length:数据长度.
返回：0成功，-1失败；
**************************************************************/
int full_HandleSceneCustomReport(DevInfo *pCurrentNode,unsigned char *data, unsigned char length);


/**************************************************************
函数名：fullscene_GetEnable
描述：获取全部功能矢量开关
参数：@char *data：输入数据首地址;
@int devTyoe:1-空调,2-地暖,3-新风
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetEnable(char *devicId,int devTyoe);


/**************************************************************
函数名：fullscene_McuGetVersion
描述：查询mcu版本号
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int fullscene_McuGetVersion(char *devicId,int selectVer);

/**************************************************************
函数名：fullscene_GetSwitchEndpoints
描述：获取设备开关/窗帘路数
参数：@char *devicId
@int devType:0x05-开关，0x06-窗帘
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetSwitchEndpoints(char *devicId,int devType);


/**************************************************************
函数名：fullscene_GetDltStatus
描述：获取DLT状态指令
参数：@char *devicId
@endponit:1-5
@int attriType:0xFF-所有属性，0x01-开关属性，0x02-亮度属性，0x03-色温属性
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetDltStatus(char *devicId,int endponit,int attriType);
#endif /* SRC_FULL_SCENE_H_ */
