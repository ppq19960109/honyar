
#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sqlite3.h"
#include "log.h"
#include "cJSON.h"
#include "rex_export_gateway.h"
#include "rex_export_type.h"




typedef struct groupManage
{
	char address[17];
	char endpoint[4];
	char groupId[8];
	int subDevType;
	struct groupManage *next;
}GroupManage;


static GroupManage *groupHead = NULL;//分组链表头指针
static GroupManage *groupTail = NULL;//分组链表尾指针
static pthread_mutex_t mutexGroup;  //分组链表锁
static sqlite3 *gData;

int groupInit(sqlite3 *pd)
{
	pthread_mutex_init(&mutexGroup, NULL);

	gData = pd;
	char *errorStr = NULL;
	char **azResult;
	int nRow = 0;
	int nColum = 0;

	int res=0;
	char sqlStr[300];
	sprintf(sqlStr, "%s", "CREATE TABLE Table_group_list(\
			groupId CHAR(5),\
			deviceId CHAR(16),\
			endpoint CHAR(4),\
			subType INTEGER(4));");
	res = sqlite3_exec(pd, sqlStr, 0, 0, &errorStr);
	

	sprintf(sqlStr, "%s", "SELECT *FROM Table_group_list");
	res = sqlite3_get_table(pd, sqlStr, &azResult, &nRow, &nColum, &errorStr);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] operation %s failed: %s\n\n",sqlStr,errorStr);
		sqlite3_free_table(azResult);
		return -1;
	}
	if (nRow == 0)
	{
		INFO("[ZIGBEE] Table_group_list is empty\n\n");
	}
	else
	{
		int i = 0;
		for (i = 0; i < nRow; i++)
		{
			GroupManage *newNode;
			newNode = (GroupManage*)malloc(sizeof(GroupManage));

			sprintf(newNode->groupId, azResult[(1 + i)*nColum + 0]);
			sprintf(newNode->address, azResult[(1+i)*nColum+1]);
			sprintf(newNode->endpoint, azResult[(1+i)*nColum+2]);
			newNode->subDevType = atoi(azResult[(1 + i)*nColum + 3]);
			newNode->next = NULL;

			if (groupHead == NULL)
			{
				groupHead = newNode;
				groupTail = newNode;
			}
			else
			{
				groupTail->next = newNode;
				groupTail = newNode;
			}
		}
		sqlite3_free_table(azResult);
	}
	return 0;
}


int groupAdd(char *address, char *endpoint,char *groupId, int subDevType)
{
	GroupManage *newNode;
	newNode = (GroupManage*)malloc(sizeof(GroupManage));

	sprintf(newNode->address, address);
	sprintf(newNode->groupId, groupId);
	sprintf(newNode->endpoint, endpoint);
	newNode->subDevType=subDevType;
	newNode->next = NULL;
	
	pthread_mutex_lock(&mutexGroup);

	GroupManage *currentNode;
	currentNode = groupHead;

	while (currentNode != NULL)
	{
		if (strcmp(address, currentNode->address) == 0 && strcmp(groupId, currentNode->groupId) == 0\
				&&strcmp(endpoint, currentNode->endpoint) == 0)
		{
			free(newNode);
			pthread_mutex_unlock(&mutexGroup);
			return 0;
		}

		else
			currentNode = currentNode->next;
	}


	if (groupHead == NULL)
	{
		groupHead = newNode;
		groupTail = newNode;
	}
	else
	{
		groupTail->next = newNode;
		groupTail = newNode;
	}
		
	pthread_mutex_unlock(&mutexGroup);

	char *errorStr = NULL;
	char sqlStr[300];
	int res=0;
	sprintf(sqlStr, "INSERT INTO Table_group_list (groupId,deviceId,endpoint,subType) VALUES (\'%s\',\'%s\',\'%s\',%d);", groupId, address, endpoint,subDevType);
	res = sqlite3_exec(gData, sqlStr, 0, 0, &errorStr);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] Insert device=%s groupId=%s into Table_group_list failed: %s\n\n", address, groupId, errorStr);
		return -1;
	}
	return 0;
}


int groupDeleteItem(char *deviceId,char *endpoint,char *groupId)
{
	GroupManage *currentNode;
	GroupManage *deleteNode = NULL;

	pthread_mutex_lock(&mutexGroup);
	currentNode = groupHead;
	GroupManage *preNode= groupHead;
	

	while (currentNode != NULL)
	{
		if ((groupId==NULL ||strcmp(currentNode->groupId,groupId)==0) \
			&& strcmp(deviceId, currentNode->address) == 0 \
			&& (endpoint==NULL || strcmp(currentNode->endpoint, endpoint) == 0 ) )
		{
			if (currentNode == groupHead && currentNode == groupTail)
			{

				groupHead = NULL;
				groupTail = NULL;
				deleteNode = currentNode;
				currentNode = NULL;
			}
			else if (currentNode == groupHead && currentNode != groupTail)
			{

				groupHead = currentNode->next;
				deleteNode = currentNode;
				preNode= currentNode->next;
				currentNode= currentNode->next;
			}
			else if (currentNode != groupHead && currentNode != groupTail)
			{

				preNode->next = currentNode->next;
				deleteNode = currentNode;
				currentNode = currentNode->next;
			}
			else if (currentNode != groupHead && currentNode == groupTail)
			{
				preNode->next = NULL;
				groupTail = preNode;
				deleteNode = currentNode;
				currentNode = NULL;
			}

			if (deleteNode != NULL)
				free(deleteNode);
		}
		else
		{
			preNode = currentNode;
			currentNode = currentNode->next;
		}
			
	}
	pthread_mutex_unlock(&mutexGroup);

	char *errorStr = NULL;
	char sqlStr[300];
	int res=0;
	if (groupId==NULL && endpoint==NULL)
		sprintf(sqlStr, "DELETE FROM Table_group_list WHERE deviceId=\'%s\';", deviceId);
	else if (groupId==NULL && endpoint!=NULL)
		sprintf(sqlStr, "DELETE FROM Table_group_list WHERE deviceId=\'%s\' AND endpoint=\'%s\';", deviceId,endpoint);
	else
		sprintf(sqlStr, "DELETE FROM Table_group_list WHERE deviceId=\'%s\' AND endpoint=\'%s\' AND groupId=\'%s\';", deviceId,endpoint,groupId);
	res = sqlite3_exec(gData, sqlStr, 0, 0, &errorStr);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] Delete device=%s groupId=%s from Table_group_list failed: %s\n\n", deviceId, groupId, errorStr);
		return -1;
	}
	return 0;
}


int groupGetGroupList()
{
	return 0;
}



int groupGetGroupRexType(char *groupId)
{
	int rexType = 0;
	GroupManage *currentNode;
	pthread_mutex_lock(&mutexGroup);
	currentNode = groupHead;

	while (currentNode != NULL)
	{
		if (strcmp(groupId,currentNode->groupId)==0)
		{
			rexType = currentNode->subDevType;
			break;
		}
		else
			currentNode = currentNode->next;
	}
	pthread_mutex_unlock(&mutexGroup);
	return rexType;
}

void groupClearList()
{
	GroupManage *currentNode;
	GroupManage *deleteNode;
	currentNode = groupHead;
	while(currentNode!=NULL)
	{
		deleteNode=currentNode;
		currentNode=currentNode->next;
		free(deleteNode);
		deleteNode=NULL;
	}
	char sqlStr[300];
	char *zErrMsg = NULL;
	sprintf(sqlStr, "DELETE FROM Table_group_list;");
	sqlite3_exec(gData, sqlStr, NULL, NULL, &zErrMsg);
	if (zErrMsg!=NULL)
		free(zErrMsg);

}


int packagegroupCommand(char *key,char *value,char *outbuff)
{
	cJSON *root;
	root=cJSON_CreateObject();
	if (root==0)
		return -1;
	cJSON_AddStringToObject(root,key,value);
	char *strJson;
	strJson=cJSON_PrintUnformatted(root);
	sprintf(outbuff,"%s",strJson);
	free(strJson);
	cJSON_Delete(root);
	return 0;

}

int groupCtrlCommand(char *geoupId,char *type,char *key,char *value)
{
	unsigned short gId=1;
	gId=atoi(geoupId);
	unsigned short commandType;

	if (strcmp(type,"A")==0)
	{
		commandType=0x0105;
	}
	else if(strcmp(type,"B")==0)
	{
		if (strcmp(key,"Level")==0)
			commandType=0x010C;
		else if(strcmp(key,"Operate")==0)
			commandType=0x010D;
		else
			return -1;
	}
	else if(strcmp(type,"C")==0)
	{
		if (strcmp(key,"State")==0)
			commandType=0x0106;
		else if(strcmp(key,"Level")==0)
			commandType=0x0107;
		else if(strcmp(key,"Temperature")==0)
			commandType=0x0108;
		else
			return -1;
	}
	char command[100]={0};
	packagegroupCommand(key,value,command);

	return rex_send_group_command_data(gId,1,commandType,command);
}





























