/*
 * ty_device.h
 *
 *  Created on: Nov 2, 2020
 *      Author: jerry
 */

#ifndef TY_DEVICE_H_
#define TY_DEVICE_H_

#include "hyprotocol.h"

/**************************************************************
函数名：isTYDeviceFirmware
描述：	判断是否为涂鸦固件，主要判断开头两个字符是否为“_T”
参数：	@char *realModelId：设备ModelId.对于涂鸦设备实际为Manufacturer值，
返回：	1涂鸦固件，0不是；
**************************************************************/
int isTYDeviceFirmware(char *realModelId);

/**************************************************************
函数名：ty_devCtrlSwitchPannel123
描述：	涂鸦开关面板控制
参数：	@KeyValue *currentKeyValue，
返回：	-1失败，0成功；
**************************************************************/
int ty_devCtrlSwitchPannel123(KeyValue *currentKeyValue);

#endif /* TY_DEVICE_H_ */
