/*
 * specialDeviceJudge.h
 *
 *  Created on: Nov 28, 2018
 *      Author: jerry
 */

#ifndef SRC_SPECIALDEVICEJUDGE_H_
#define SRC_SPECIALDEVICEJUDGE_H_

/*********************
 * 函数名：isTwoInfraredCurtainSensor
 * 描  述：判断该报警设备是否为双向红外幕帘，是否允许上报;
 *        当不是双向红外幕帘时，不允许上报;
 *        当是双向红外幕帘,但是alarmValue！=1时，不允许上报;
 * 参  数： @char *modelId：产品modelid;
 *         @char *alarmValue: 0 或 1
 * 返  回：0不允许上报，1允许上报
 */
int isTwoInfraredCurtainSensor(char *modelId,char *alarmValue);


#endif /* SRC_SPECIALDEVICEJUDGE_H_ */
