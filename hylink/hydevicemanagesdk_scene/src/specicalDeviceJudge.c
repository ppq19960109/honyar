/*
 * specicalDeviceJudge.c
 *
 *  Created on: Nov 28, 2018
 *      Author: jerry
 */

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
/*********************
 * 函数名：isTwoInfraredCurtainSensor
 * 描  述：判断该报警设备是否为双向红外幕帘，是否允许上报;
 *        当不是双向红外幕帘时，不允许上报;
 *        当是双向红外幕帘,但是alarmValue！=1时，不允许上报;
 * 参  数： @char *modelId：产品modelid;
 *         @char *alarmValue: 0 或 1
 * 返  回：2非双向红外幕帘设备，0不允许上报，1允许上报
 */
int isTwoInfraredCurtainSensor(char *modelId,char *alarmValue)
{
	int res=0;
	if (strcmp(modelId,"HY0023")==0 || strcmp(modelId,"a1WKnxuEJ9N")==0)//5c26d3
	{
		if(1==atoi(alarmValue))
			res=1;
	}
	else
		res=2;
	return res;
}
