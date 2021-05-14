#include "sqlite3.h"


/*********************
函数名：groupInit
描述：分组链表初始化
参数：@sqlite3 *pd：数据库
返回：0-成功，-1失败
**********************/
int groupInit(sqlite3 *pd);


/*********************
函数名：groupAdd
描述：分组链表ADD
参数：@char *address:设备的Id
@char *endpoint;
@char *groupId:组号;
@int subDevType:ex组号
返回：0-成功，-1失败
**********************/
int groupAdd(char *address, char *endpoint,char *groupId, int subDevType);

/*********************
函数名：groupDeleteItem
描述：删除链表中某一个设备的组号记录
参数：
@char *deviceId：设备的Id;
@char *endpoint:
@char *groupI：要删除的组号
返回：0-成功，-1失败
**********************/
int groupDeleteItem(char *deviceId,char *endpoint,char *groupId);


/*********************
函数名：groupGetGroupRexType
描述：获取该组同类设备的rex设备类型；
参数：@char *groupI：组号
返回：rex组号，0失败
**********************/
int groupGetGroupRexType(char *groupId);


/*********************
函数名：groupGetGroupList  //暂不支持，以待扩充
描述：获取同组下的设备Id；
参数：@char *groupId：组号
返回：rex组号，0失败
**********************/
int groupGetGroupList();

/*********************
函数名：groupClearList
描述：删除分组链表；
参数：
返回：空
**********************/
void groupClearList();

/*********************
函数名：groupCtrlCommand
描述：组播控制指令；
参数：@char *geoupId:组号;
	@char *type:类型,A=插座类设备,B=窗帘类设备,C=照明类设备;
	@char *key:控制参数,参看组播支持产品类表;
	@char *value:值
返回：0成功,-1失败
**********************/
int groupCtrlCommand(char *geoupId,char *type,char *key,char *value);

