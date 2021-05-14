/*
 * syncSwitch.h
 *
 *  Created on: Jul 21, 2020
 *      Author: jerry
 */

#ifndef SRC_SYNCSWITCH_H_
#define SRC_SYNCSWITCH_H_

#include "hyprotocol.h"

/*初始化哈系表*/
int initSwitchGroup();

/*处理客户端下发的开关同步指令*/
int handleSyncSwitchGroupCmd(KeyValue *head);

/*处理设备端上报的开关状态指令*/
int checkSyncSwitchReport(char *deviceId,char *key,unsigned char endpoint,char *value);

/*清除开关同步组*/
void deleteAllSwitchGroupFun();

#endif /* SRC_SYNCSWITCH_H_ */
