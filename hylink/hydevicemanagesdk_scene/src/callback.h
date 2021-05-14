#ifndef _CALLBACK_H_
#define _CALLBACK_H_

/**************************************************************
函数名：setMessageReportFunHandler
描述：设置设备信息上报回调函数
参数：@void* funName：上报回调函数名，具体由第三方用户实现：int funName(char *)，返回0成功，-1失败;
返回：空；
**************************************************************/
int setMessageReportFunHandler(void* funName);

/**************************************************************
函数名：serResetButen8sFunHandler
描述：设置复位按键长按8s的事件回调函数，若不用则 回调函数里填  return 0;
参数：@void* funName：上报回调函数名，具体由第三方用户实现：int funName()，返回0成功，-1失败;
返回：空；
**************************************************************/
int serResetButen8sFunHandler(void* funName);

/**************************************************************
函数名：messageReportFun
描述：设备信息上报回调函数，被deviceList.c 和led.c调用
参数：@void* funName：上报回调函数名，具体由第三方用户实现：int funName()，返回0成功，-1失败;
返回：空；
**************************************************************/
int messageReportFun(char *inputBuff);

/**************************************************************
函数名：resetButen8sEventFun
描述：复位按键长按8s的事件回调函数，被deviceList.c 调用;
参数：@void* funName：上报回调函数名，具体由第三方用户实现：int funName()，返回0成功，-1失败;
返回：空；
**************************************************************/
int resetButen8sEventFun();



#endif 