/*
 * bindAirConditioning.h
 * Created on: Sep 25, 2018
 * Author: jerry
 * Description: 用于将1路空调温控器面板绑定到16路空调协议转换器（或叫485协议转换器）的某一路。
 *              这样1路空调面板的状态改变将通过网关装发给对应16路空调协议转换器的某一路，
 *              由转换器控制实际空调。
 */

#ifndef SRC_BINDAIRCONDITIONING_H_
#define SRC_BINDAIRCONDITIONING_H_

/***************************************************
 * 函数名称：bindListInit
 * 描   述：在数据库中创建绑定表，如果已经建立则根据绑定表中的数据创建绑定链表;
 * 参数说明：@sqlite3 *pd：数据库文字描述符;
 * 返回结果：0成功，-1失败
 **************************************************/
int bindListInit(sqlite3 *pd);

/***************************************************
 * 函数名称：bindAddItem
 * 描   述：在数据库和绑定链表中添加绑定关系;
 * 参数说明：@char *targetAddr：16路空调协议转换器地址或mac;
 *         @int targetEndpoint：16路空调协议转换器的第几路；
 *         @char *pannelAddr：路空调面板的地址
 * 返回结果：0成功，-1失败
 **************************************************/
int bindAddItem(char *targetAddr,int targetEndpoint,char *pannelAddr);

/***************************************************
 * 函数名称：bindDeleteAllItem
 * 描   述：从数据库和绑定链表中删除所有和地址address有关的所有绑定关系;
 * 参数说明：@char *address：16路空调协议转换器或1路空调温控器面板地址或mac;
 * 返回结果：0成功，-1失败
 **************************************************/
int bindDeleteAllItem(char *address);

/***************************************************
 * 函数名称：bindDeleteItem
 * 描   述：从数据库和绑定链表中删除某一个绑定关系;
 * 参数说明：@char *targetAddr：16路空调协议转换器地址或mac;
 *         @int targetEndpoint：16路空调协议转换器的第几路；
 *         @char *pannelAddr：1路空调面板的地址
 * 返回结果：0成功，-1失败
 **************************************************/
int bindDeleteItem(char *targetAddr,int targetEndpoint,char *pannelAddr);

/***************************************************
 * 函数名称：bindClearList
 * 描   述：从数据库和绑定链表中删除所有绑定关系;
 * 参数说明：
 * 返回结果：0成功，-1失败
 **************************************************/
int bindClearList();

/***************************************************
 * 函数名称：bindCheck
 * 描   述：查询1路空调面板是否存在绑定关系;
 * 参数说明：@char *pannelAddr：1路空调面板的地址
 * 		   @char *targetAddr：若有绑定关系，此处返回16路空调协议转换器地址或mac;
 *         @int targetEndpoint：若有绑定关系，此处返回16路空调协议转换器的第几路；
 * 返回结果：0成功，-1不存在绑定关系
 **************************************************/
int bindCheck(char *pannelAddr,char *targetAddr,int *targetEndpoint);

/***************************************************
 * 函数名称：bindQueryList
 * 描   述：查询空调协议转换器所有的绑定关系数据;
 * 参数说明：@char *targetAddr：16路空调协议转换器地址或mac;
 * 		   char *outBuff：输出绑定关系，jsong格式
 * 返回结果：恒等于0成功
 **************************************************/
int bindQueryList(char *targetAddr,char *outBuff);

/***************************************************
 * 函数名称：packageAirBindResult
 * 描   述：打包绑定操作执行结果上报消息;
 * 参数说明：@char *deviceId：16路空调协议转换器地址或mac
 * 		   @char *key：操作类型，A添加绑定，D删除绑定，C清除所有绑定
 * 		   @char *pannelAddr：1路空调面板的地址;
 *         @int endpont：绑定到16路空调协议转换器的第几路；
 *         @char *result：操作结果
 *         @char*outBuff：输出打包好的JOSN字符串
 * 返回结果：空
 **************************************************/
void packageAirBindResult(char *deviceId, char *key, char *pannelAddr,char *endpont,char *result,char*outBuff);

#endif /* SRC_BINDAIRCONDITIONING_H_ */
