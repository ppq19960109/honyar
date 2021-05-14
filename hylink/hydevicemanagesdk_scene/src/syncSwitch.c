/*
 * syncSwitch.c
 *
 *  Created on: Jul 21, 2020
 *      Author: jerry
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#include "syncSwitch.h"
#include "common.h"
#include "hashmap.h"
#include "sqlite3.h"
#include "callback.h"
#include "sqlite3.h"
#include "log.h"
#include "rexCommand.h"
#include "deviceList.h"
#include "getpath.h"
#include "usr_sleep.h"


#define TIME_OUT	(5)
#define GROUPID_NAME_MAX (62)
#define ALL_GROUP_INFO_MEX_LEN (8000)

typedef struct SWITCH_GROUP
{
	char groupId[DEVICE_GROUP_ID_LEN];
	char deviceId[DEVICE_DEVICE_ID_LENGTH];
	char key[ATTRIBUTE_KEY_MAX_LENGTH];
	unsigned char endpointId;
	struct SWITCH_GROUP *next;
}SwitchGroup;

typedef struct SYNC_SWITCH_GROUP
{

	char groupId[DEVICE_GROUP_ID_LEN];
	char name[GROUPID_NAME_MAX];
	int enable;
	int devNum;
	SwitchGroup* head;
	SwitchGroup* tail;
	struct SYNC_SWITCH_GROUP *next;
}SyncSwitchGroup;


typedef struct SYNC_QUEUE
{

	char groupId[DEVICE_GROUP_ID_LEN];
	char deviceId[DEVICE_DEVICE_ID_LENGTH];
	unsigned char endpointId;
	char value[4];
	struct timeval now;
	struct SYNC_QUEUE *next;
}SyncQueue;


static SyncQueue *queueHead=NULL;
static SyncQueue *queueTail=NULL;

static SyncSwitchGroup *listHead=NULL;
static SyncSwitchGroup *listTail=NULL;

static hash_map_t *mapfd=NULL;
static sqlite3 *pDB = NULL;
static char *zErrMsg = NULL;
static pthread_mutex_t mutexSwitchGroup;/*白名单线程锁*/

static int groupNum = 0;

#define MAX_GROUP_NUM	(10)
#define MAZ_DEV_KEY_NUM	(5)

/*****************数据库相关************************/
/*初始化同步开关组数据库*/
static int openSwitchGroupDatabase()
{
	int res = 0;
	char *path;
	char absolutPath[200];

	path=getFileDirByLink("/proc/self/exe");
	sprintf(absolutPath,"%s%s",path,"switchgroup.db");
	res = sqlite3_open(absolutPath, &pDB);
	if (res)
	{
		ERROR("\033[0;32;31mopen switchgroup.db error: %s\033[0m \n", sqlite3_errmsg(pDB));
		free(path);
		return -1;
	}
	free(path);
	return 0;
}

/*创建表*/
static int creatSwitchGroupTableToDB()
{
	int res=0;
	char sqlStr[400];
	sprintf(sqlStr, "%s", "CREATE TABLE Table_switch_group( \
			groupId CHAR(40) PRIMARY KEY NOT NULL, \
			groupName CHAR(62),\
			enable CHAR(4));");

	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Create Table_switch_group: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sprintf(sqlStr, "%s", "CREATE TABLE Table_switch_group_item( \
			groupId CHAR(40) NOT NULL, \
			deviceId CHAR(40) NOT NULL,\
			key CHAR(48),\
			PRIMARY KEY(groupId,deviceId));");

	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Create Table_switch_group: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	return res;
}

/*从数据库删除整个组*/
static int deleteSwitchGroupIdFromDB(char *groupId)
{
	char sqlStr[200];
	sprintf(sqlStr,"DELETE FROM Table_switch_group WHERE groupId=\'%s\';",groupId);
	int res;
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Table_switch_group: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sprintf(sqlStr,"DELETE FROM Table_switch_group_item WHERE groupId=\'%s\';",groupId);
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Table_switch_group_item: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	return res;
}

/*清空数据库所有组*/
int clearAllSwitchGroupFromDB()
{
	int res;
	char sqlStr[200];
	sprintf(sqlStr,"DELETE FROM Table_switch_group;");

	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Table_switch_group: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}

	sprintf(sqlStr,"DELETE FROM Table_switch_group_item;");
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Table_switch_group_item: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	return res;
}

/*向表Table_switch_group插入或更新组号信息*/
static int insertSwitchGroupIdToDB(char *groupId,char *groupName,char *enable)
{
	char sqlStr[200];
	sprintf(sqlStr,"REPLACE INTO Table_switch_group (groupId,groupName,enable) VALUES (\'%s\',\'%s\',\'%s\');",\
			groupId,groupName,enable);
	int res;
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Table_switch_group: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	return res;
}




/*向数据库插入组规则信息*/
static int insertSwitchGroupItemToDB(char *groupId,char *deviceId,char *key)
{
	char sqlStr[200];
	sprintf(sqlStr,"REPLACE INTO Table_switch_group_item (groupId,deviceId,key) VALUES (\'%s\',\'%s\',\'%s\');",\
			groupId,deviceId,key);
	int res;
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Table_switch_group_item: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	return res;
}

int updateSwitchGroupEnableToDB(char *groupId,char *enable)
{
	char sqlStr[200];
	sprintf(sqlStr,"UPDATE Table_switch_group SET enable=\'%s\' WHERE groupId=\'%s\';",\
			enable,groupId);
	int res;
	res = sqlite3_exec(pDB, sqlStr, 0, 0, &zErrMsg);
	if (res != SQLITE_OK)
	{
		DEBUG("Table_switch_group: %s\n\n", zErrMsg);
		sqlite3_free(zErrMsg);
	}
	return res;
}

/*****************开关组链表操作****************************/
/*释放组信息节点存储空间*/
static void freeSyncSwitchGroup(SyncSwitchGroup *node)
{
	if (!node)
		return ;
	SwitchGroup *current=node->head;
	node->head=NULL;
	SwitchGroup *delNode=NULL;
	while(current)
	{
		delNode=current;
		current=current->next;
		free(delNode);
		delNode=NULL;
	}
	free(node);
	node=NULL;
}

/*添加至链表*/
static void addSwitchGroupToList(SyncSwitchGroup *node)
{
	if(NULL==listHead && NULL==listTail)
	{
		listHead=node;
		listTail=node;
	}
	else
	{
		listTail->next=node;
		listTail=node;
	}
	groupNum++;
}

/*从链表删除组*/
static void deleteSwitchGroupFromList(char *groupId)
{
	SyncSwitchGroup *curNode=listHead;
	SyncSwitchGroup *preNode=listHead;
	SyncSwitchGroup *delNode=NULL;
	while(curNode)
	{

		if(strcmp(groupId,curNode->groupId)==0)
		{
			groupNum--;
			if(listHead==curNode && listTail==curNode)
			{
				listHead=NULL;
				listTail=NULL;
				preNode=NULL;
				delNode=curNode;
			}
			else if (listHead==curNode && listTail!=curNode)
			{
				delNode=curNode;
				listHead=curNode->next;
				preNode=listHead;

			}
			else if(listHead!=curNode && listTail==curNode)
			{
				delNode=curNode;
				preNode->next=NULL;
				listTail=preNode;

			}
			else
			{
				delNode=curNode;
				preNode->next=curNode->next;
			}
			freeSyncSwitchGroup(delNode);
			break;
		}
		else
		{
			preNode=curNode;
			curNode=curNode->next;
		}

	}
}

/*清空链表所有组*/
static void deleteAllSwitchGroupFromList()
{
	SyncSwitchGroup *curNode=listHead;
	listHead=NULL;
	listTail=NULL;
	groupNum=0;
	SyncSwitchGroup *delNode=NULL;
	while(curNode)
	{
		delNode=curNode;
		curNode=curNode->next;
		freeSyncSwitchGroup(delNode);
	}
}

/*****************哈系表相关****************************/
/*向哈系表插入组信息节点*/
static int insertSyncGroupItems(SyncSwitchGroup *node)
{
	if (!node)
		return -1;
	SwitchGroup *currentNode=node->head;

	mapfd->inst(mapfd,node->groupId,(void*)&node,sizeof(SyncSwitchGroup *));

	char devKey[64]={0};
	while(currentNode)
	{
		sprintf(devKey,"%s%s",currentNode->deviceId,currentNode->key);
		mapfd->inst(mapfd,devKey,(void*)&node,sizeof(SyncSwitchGroup *));
		currentNode=currentNode->next;
	}
	return 0;
}

/*哈系表查找组号信息*/
SyncSwitchGroup* findSwitchGroupId(char *groupId)
{
	void *tmp = NULL;
	tmp = mapfd->find(mapfd, groupId);
	if (tmp == NULL)
		return NULL;
	SyncSwitchGroup *node = NULL;
	memcpy(&node, tmp, 4);
	if (node == NULL)
		return NULL;
	return node;
}

/*通过设备ID和 key查找哈系表信息*/
SyncSwitchGroup* findDevKeyInSwitchGroup(char *deviceId, char *key)
{
	char index[64] = {0};
	sprintf(index, "%s%s", deviceId, key);

	void *tmp = NULL;
	tmp = mapfd->find(mapfd, index);
	if (tmp == NULL)
		return NULL;
	SyncSwitchGroup *node = NULL;
	memcpy(&node, tmp, 4);
	if (node == NULL)
		return NULL;
	return node;
}


/*****************匹配链表相关****************************/
/*删除匹配链表中的节点信息*/
static void deleteQueueNode(SyncQueue **nowNode,SyncQueue **preNode)
{
	SyncQueue *delNode=*nowNode;
	if(queueHead==*nowNode && queueTail==*nowNode)
	{
		queueHead=NULL;
		queueTail=NULL;
		*preNode=NULL;
		*nowNode=NULL;
	}
	else if (queueHead==*nowNode && queueTail!=*nowNode)
	{
		queueHead=(*nowNode)->next;
		*preNode=queueHead;
		*nowNode=(*nowNode)->next;
	}
	else if(queueHead!=*nowNode && queueTail==*nowNode)
	{
		(*preNode)->next=NULL;
		queueTail=*preNode;
		*nowNode=NULL;
	}
	else
	{
		(*preNode)->next=(*nowNode)->next;
		*nowNode=(*nowNode)->next;
	}
	free(delNode);
	delNode=NULL;
}

/*删除匹配链表中的组号信息*/
static void deleteSwitchGroupIdFromQueue(char *groupId)
{
	SyncQueue *currNode=queueHead;
	SyncQueue *preNode=queueHead;
	SyncQueue *delNode=NULL;
	while(currNode)
	{
		if(0==strcmp(currNode->groupId,groupId))
		{
			if(queueHead==currNode && queueTail==currNode)
			{
				delNode=currNode;
				queueHead=NULL;
				queueTail=NULL;
				preNode=NULL;
				currNode=NULL;
			}
			else if (queueHead==currNode && queueTail!=currNode)
			{
				delNode=currNode;
				queueHead=currNode->next;
				preNode=queueHead;
				currNode=currNode->next;
			}
			else if(queueHead!=currNode && queueTail==currNode)
			{
				delNode=currNode;
				preNode->next=NULL;
				queueTail=preNode;
				currNode=NULL;
			}
			else
			{
				delNode=currNode;
				preNode->next=currNode->next;
				currNode=currNode->next;
			}
			free(delNode);
			delNode=NULL;
		}
		else
		{
			preNode=currNode;
			currNode=currNode->next;
		}
	}
}

/*删除匹配链表所有信息*/
static void clearAllSwitchGroupFromQueue()
{
	SyncQueue *currNode=queueHead;
	SyncQueue *delNode=NULL;
	queueTail=NULL;
	queueHead=NULL;
	while(currNode)
	{
		delNode=currNode;
		currNode=currNode->next;
		free(delNode);
		delNode=NULL;
	}
}

/*插入匹配列表*/
static void addQueneNode(SyncQueue *nowNode)
{
	if(NULL==queueHead && NULL==queueTail)
	{
		queueHead=nowNode;
		queueTail=nowNode;
	}
	else
	{
		queueTail->next=nowNode;
		queueTail=nowNode;
	}
}


/*********************************************************************/
/*初始化多控开关同步控制*/
int initSwitchGroup()
{
	pthread_mutex_init(&mutexSwitchGroup, NULL);
	openSwitchGroupDatabase();
	creatSwitchGroupTableToDB();

	return 0;
}

static void packAck(char *type,char *groupId, char *devId, char *key, char *res,char *mesg, char *outBuff)
{
	cJSON *root;
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "SwichGroup");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);

	cJSON *data;
	cJSON *dataList;
	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());

	cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
	cJSON_AddStringToObject(dataList, "Op", type);
	if(groupId)
		cJSON_AddStringToObject(dataList, "GroupId", groupId);
	if(devId)
		cJSON_AddStringToObject(dataList, "DeivceId", devId);
	if(key)
		cJSON_AddStringToObject(dataList, "Key", key);

	cJSON_AddStringToObject(dataList, "Value", res);

	if (mesg)
		cJSON_AddStringToObject(dataList, "Message", mesg);
	char *strJson=cJSON_PrintUnformatted(root);
	sprintf(outBuff,"%s",strJson);
	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
}

/*删除所有开关组*/
void deleteAllSwitchGroupFun()
{
	clearAllSwitchGroupFromQueue();
	deleteAllSwitchGroupFromList();
	clearAllSwitchGroupFromDB();
	mapfd->clear(mapfd);
}

static int deleteSwitchGroupId(char *groupid)
{
	/*删除数据库相应信息*/
	deleteSwitchGroupIdFromDB(groupid);

	return 0;
}

/*删除某个开关组*/
static void handleDeleteSwitchGroupFun(KeyValue *head)
{
	KeyValue *currentNode=head;
	char outBuff[1024];
	char inBuff[1024];
	while(currentNode)
	{
		deleteSwitchGroupId(currentNode->groupId);
		/*删除场景*/
		memset(inBuff, 0x0, 1024);
		snprintf(inBuff, 1024, "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"LocalScene\",\"Data\":[{\"Op\":\"DelScene\",\"Id\":\"%s\"}]}",
			currentNode->groupId);
		scene_query(
			inBuff, 
			strlen(inBuff) + 1, 
			NULL, 
			0, 
			NULL
		);
		packAck("Delete",currentNode->groupId,NULL,NULL,"0","成功",outBuff);
		messageReportFun(outBuff);
		currentNode = currentNode->next;
	}
}


static int querySwitchGroupInfo(char *groupId,char *Op,char *outBuff,int *len)
{
	int nRow = 0;
	int nColum = 0;
	int res;
	char **resultDev;
	char **resContent;
	char sqlStr[200];


	sprintf(sqlStr,"SELECT * FROM Table_switch_group_item WHERE groupId=\'%s\';",groupId);
	res = sqlite3_get_table(pDB, sqlStr, &resultDev, &nRow, &nColum, &zErrMsg);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIBGEE] %s : %s\n\n", sqlStr, sqlite3_errmsg(pDB));
		sqlite3_free(zErrMsg);
		sqlite3_free_table(resultDev);
		return -1;
	}
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "SwichGroup");

	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);

	cJSON *data;
	cJSON *dataList;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());

	cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
	cJSON_AddStringToObject(dataList, "Op", Op);
	cJSON_AddStringToObject(dataList, "GroupId", resultDev[(1 + 0)*nColum + 0]);
	cJSON_AddStringToObject(dataList, "Name", resultDev[(1 + 0)*nColum + 1]);
	cJSON_AddStringToObject(dataList, "Enable", resultDev[(1 + 0)*nColum + 2]);

	cJSON *itemArray;
	cJSON_AddItemToObject(dataList, "Items", itemArray = cJSON_CreateArray());

	sprintf(sqlStr,"SELECT * FROM Table_switch_group_item WHERE groupId=\'%s\';",groupId);
	res = sqlite3_get_table(pDB, sqlStr, &resContent, &nRow, &nColum, &zErrMsg);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIBGEE] %s : %s\n\n", sqlStr, sqlite3_errmsg(pDB));
		sqlite3_free(zErrMsg);
		sqlite3_free_table(resultDev);
		sqlite3_free_table(resContent);
		cJSON_Delete(root);
		return -1;
	}
	cJSON *devKey;
	int i;
	char modelId[DEVICE_MODEL_ID_LENGTH];
	for(i=0;i<nRow;i++)
	{
		cJSON_AddItemToArray(itemArray, devKey = cJSON_CreateObject());
		cJSON_AddStringToObject(devKey, "DeviceId", resContent[(1 + i)*nColum + 1]);
		memset(modelId,0,DEVICE_MODEL_ID_LENGTH);
		getDevModelId(resContent[(1 + i)*nColum + 1], modelId);
		if(modelId[0]=='\0')
			sprintf(modelId,"None");
		cJSON_AddStringToObject(devKey, "ModelId", modelId);
		cJSON_AddStringToObject(devKey, "Key", resContent[(1 + i)*nColum + 2]);
	}

	char *strJson=cJSON_PrintUnformatted(root);
	if(*len<strlen(strJson))
	{
		cJSON_Delete(root);
		if(strJson!=NULL)
			free(strJson);
		sqlite3_free_table(resultDev);
		sqlite3_free_table(resContent);
		return -1;
	}

	sprintf(outBuff,"%s",strJson);
	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
	sqlite3_free_table(resultDev);
	sqlite3_free_table(resContent);
	*len=strlen(strJson);
	return 0;
}

static int handleAddSwitchGroupFun(KeyValue *head)
{

	char outBuff[1024] = { 0 };
	char inBuff[1024] = { 0 };
	int buffLen=1024;
	KeyValue *current=head;
	while(current)
	{
		deleteSwitchGroupId(current->groupId);
		/*删除场景*/
		memset(inBuff, 0x0, 1024);
		snprintf(inBuff, 1024, "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"LocalScene\",\"Data\":[{\"Op\":\"DelScene\",\"Id\":\"%s\"}]}",
			current->groupId);
		scene_query(
			inBuff, 
			strlen(inBuff) + 1, 
			NULL, 
			0, 
			NULL
		);
			
		if(groupNum>=MAX_GROUP_NUM)
		{
			packAck("Add",current->groupId,NULL,NULL, "101", "达到组别总个数设置上限", outBuff);
			messageReportFun(outBuff);
			return -1;
		}
		cJSON *root=cJSON_Parse(current->value);
		int count=cJSON_GetArraySize(root);

		if(count>MAZ_DEV_KEY_NUM)
		{
			packAck("Add", current->groupId, NULL,NULL,"102", "达到设备按键总个数设置上限", outBuff);
			messageReportFun(outBuff);
			cJSON_Delete(root);
			return -1;;
		}

		if(count==0)
		{
			packAck("Add", current->groupId, NULL,NULL,"108", "无效设置", outBuff);
			messageReportFun(outBuff);
			cJSON_Delete(root);
			return -1;;
		}

		insertSwitchGroupIdToDB(current->groupId,current->name,current->enable);
		/*添加场景*/
		memset(inBuff, 0x0, 1024);
		snprintf(inBuff, 1024, "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"LocalScene\",\"Data\":[{\"Op\":\"AddScene\",\"Id\":\"%s\",\"Name\":\"%s\",\"Enable\":\"%s\",\"ExecDelayed\":\"0\",\"UpdateTime\":\"\",\"Note\":\"\"}]}",
			current->groupId, current->name, current->enable);
		scene_query(
			inBuff, 
			strlen(inBuff) + 1, 
			NULL, 
			0, 
			NULL
		);

		int i;
		cJSON *item;
		cJSON *deviceId;
		cJSON *key;
		for(i=0;i<count;i++)
		{
			item=cJSON_GetArrayItem(root,i);
			deviceId=cJSON_GetObjectItem(item,"DeviceId");
			key=cJSON_GetObjectItem(item,"Key");

			if(!deviceId || !key)
			{
				packAck("Add", current->groupId, NULL, NULL, "106","指令解析失败", outBuff);
				messageReportFun(outBuff);
				cJSON_Delete(root);
				continue;
			}
			

			insertSwitchGroupItemToDB(current->groupId, deviceId->valuestring, key->valuestring);

			/*添加状态同步触发*/
			memset(inBuff, 0x0, 1024);
			snprintf(inBuff, 1024, "{\"Command\":\"Dispatch\",\"FrameNumber\":\"00\",\"Type\":\"LocalScene\",\"Data\":[{\"Op\":\"AddCond\",\"Id\":\"%s\",\"Logic\":\"Or\",\"CondType\":\"Event\",\"TriggerType\":\"StateSync\",\"ContinueTime\":\"0\",\"DevId\":\"%s\",\"Key\":\"%s\",\"Value\":\"1\",\"Active\":\"Equ\"}]}",
				current->groupId, deviceId->valuestring, key->valuestring);
			scene_query(
				inBuff, 
				strlen(inBuff) + 1, 
				NULL, 
				0, 
				NULL
			);
		}
		//select_mssleep(100);
		querySwitchGroupInfo(current->groupId,"Add",outBuff,&buffLen);
		messageReportFun(outBuff);
		cJSON_Delete(root);
		current=current->next;
	}
	return 0;
}


/*查询所有开关组组号*/
static int queryAllGroupId(char *outBuff)
{
	int res;
	int nRow = 0;
	int nColum = 0;
	char **azResult;

	int mRow = 0;
	int mColum = 0;
	char **resultDev;

	char modelId[DEVICE_MODEL_ID_LENGTH];

	char sqlStr[200];
	sprintf(sqlStr,"SELECT * FROM Table_switch_group");
	res = sqlite3_get_table(pDB, sqlStr, &azResult, &nRow, &nColum, &zErrMsg);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIBGEE] %s : %s\n\n", sqlStr, sqlite3_errmsg(pDB));
		sqlite3_free(zErrMsg);
		sqlite3_free_table(azResult);
		return -1;
	}
	cJSON *root;
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "SwichGroup");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);

	cJSON *data;
	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	int i=0;
	for(i=0;i<nRow;i++)
	{
		cJSON *dataList;
		cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
		cJSON_AddStringToObject(dataList, "Op", "Query");
		cJSON_AddStringToObject(dataList, "GroupId", azResult[(1 + i)*nColum + 0]);
		cJSON_AddStringToObject(dataList, "Name", azResult[(1 + i)*nColum + 1]);
		cJSON_AddStringToObject(dataList, "Enable", azResult[(1 + i)*nColum + 2]);

		sprintf(sqlStr,"SELECT * FROM Table_switch_group_item WHERE groupId=\'%s\';",azResult[(1 + i)*nColum + 0]);
		res = sqlite3_get_table(pDB, sqlStr, &resultDev, &mRow, &mColum, &zErrMsg);
		if (res != SQLITE_OK)
		{
			ERROR("[ZIBGEE] %s : %s\n\n", sqlStr, sqlite3_errmsg(pDB));
			sqlite3_free(zErrMsg);
			sqlite3_free_table(resultDev);
			sqlite3_free_table(azResult);
			cJSON_Delete(root);
			return -1;
		}
		cJSON *items;
		cJSON_AddItemToObject(dataList, "Items", items = cJSON_CreateArray());
		cJSON *devKey;
		int j=0;
		for(j=0;j<mRow;j++)
		{
			cJSON_AddItemToArray(items, devKey = cJSON_CreateObject());
			cJSON_AddStringToObject(devKey, "DeviceId", resultDev[(1 + j)*nColum + 1]);
			memset(modelId,0,DEVICE_MODEL_ID_LENGTH);
			if(modelId[0]=='\0')
				sprintf(modelId,"None");
			getDevModelId(resultDev[(1 + j)*nColum + 1], modelId);
			cJSON_AddStringToObject(devKey, "ModelId", modelId);
			cJSON_AddStringToObject(devKey, "Key", resultDev[(1 + j)*nColum + 2]);
		}
		sqlite3_free_table(resultDev);
	}
	char *str=cJSON_PrintUnformatted(root);
	sprintf(outBuff,"%s",str);
	if(str)
		free(str);
	sqlite3_free(zErrMsg);
	sqlite3_free_table(azResult);
	cJSON_Delete(root);
	return 0;
}

/*查询开关组*/
static int handleQuerySwitchGroupFun(KeyValue *head)
{
	KeyValue *current=head;

	char outBuff[ALL_GROUP_INFO_MEX_LEN] = { 0 };
	int buffLen=ALL_GROUP_INFO_MEX_LEN;
	while(current)
	{
		if(current->groupId[0]=='\0')
		{
			if(0==queryAllGroupId(outBuff))
				messageReportFun(outBuff);
			else
			{
				packAck("Query",NULL, NULL, NULL, "107","异常错误", outBuff);
				messageReportFun(outBuff);
			}
			current=current->next;
			continue;

		}

		querySwitchGroupInfo(current->groupId,"Query",outBuff,&buffLen);
		messageReportFun(outBuff);
		current=current->next;

	}
	return 0;
}


/*多控开关组指令处理函数*/
int handleSyncSwitchGroupCmd(KeyValue *head)
{
	if(!head)
		return -1;

	if(head->type==NULL)
		return -1;

	pthread_mutex_lock(&mutexSwitchGroup);
	if(strcmp(head->type,"Add")==0)
	{
		handleAddSwitchGroupFun(head);
	}
	else if(strcmp(head->type,"Delete")==0)
	{
		handleDeleteSwitchGroupFun(head);
	}
	else if(strcmp(head->type,"Query")==0)
	{
		handleQuerySwitchGroupFun(head);
	}

	pthread_mutex_unlock(&mutexSwitchGroup);
	return 0;
}


static int isDevStateInQuene(char *deviceId,unsigned char endpoint,char *value)
{

	SyncQueue *nowNode=queueHead;
	SyncQueue *preNode=queueHead;
	struct timeval check;
	gettimeofday(&check,NULL);
	int matchOk=0;

	while(nowNode)
	{

		if(0==strcmp(nowNode->deviceId,deviceId) && \
				nowNode->endpointId==endpoint && \
				0==strcmp(nowNode->value,value))
		{
			matchOk=1;
			deleteQueueNode(&nowNode,&preNode);
		}
		else if((check.tv_sec-nowNode->now.tv_sec)>TIME_OUT)
		{
			deleteQueueNode(&nowNode,&preNode);
		}
		else
		{
			preNode=nowNode;
			nowNode=nowNode->next;
		}

	}

	return matchOk;
}

static void syncOperation(char *deviceId,SyncSwitchGroup *node,char *value)
{
	SwitchGroup *now=node->head;
	while(now)
	{
		if(strcmp(now->deviceId,deviceId)!=0)
		{
			SyncQueue *newNode=(SyncQueue *)malloc(sizeof(SyncQueue));
			memset(newNode,0,sizeof(SyncQueue));

			sprintf(newNode->groupId,"%s",now->groupId);
			sprintf(newNode->deviceId,"%s",now->deviceId);
			newNode->endpointId=now->endpointId;
			sprintf(newNode->value,"%s",value);
			gettimeofday(&(newNode->now),NULL);
			addQueneNode(newNode);
		}
		now=now->next;
	}

	SwitchGroup *current=node->head;
	while(current)
	{
		if(strcmp(current->deviceId,deviceId)!=0)
		{
			sendCommandCtrlSwitchState(current->deviceId, current->endpointId, value);
		}
		current=current->next;
	}
}

/*判断多控开关同步*/
int checkSyncSwitchReport(char *deviceId,char *key,unsigned char endpoint,char *value)
{
	pthread_mutex_lock(&mutexSwitchGroup);
	char devKey[64]={0};
	sprintf(devKey,"%s%s",deviceId,key);
	void *tmp=mapfd->find(mapfd,devKey);
	if(tmp==NULL)
	{
		pthread_mutex_unlock(&mutexSwitchGroup);
		return 0;
	}
	SyncSwitchGroup *node = NULL;
	memcpy(&node,tmp,4);
	if (node==NULL)
	{
		pthread_mutex_unlock(&mutexSwitchGroup);
		return -1;
	}

	if(0==isDevStateInQuene(deviceId,endpoint,value) && node->enable==1)
	{
		syncOperation(deviceId,node,value);
	}
	pthread_mutex_unlock(&mutexSwitchGroup);

	return 0;

}
