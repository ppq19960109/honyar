/*
 * bindAirConditioning.c
 *
 *  Created on: Sep 20, 2018
 *      Author: jerry
 */

#include <pthread.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include "sqlite3.h"
#include "cJSON.h"
#include "log.h"


typedef struct bindManage
{
	char pannelAddr[18];
	char targetAddr[18];
	int targetEndpoint;
	struct bindManage  *next;
}BindManage;

static BindManage *bindHead = NULL;
static BindManage *bindTail = NULL;
static pthread_mutex_t mutexBind;
static sqlite3 *bData;

int bindListInit(sqlite3 *pd)
{
	pthread_mutex_init(&mutexBind, NULL);

	bData = pd;
	char *errorStr = NULL;
	char **azResult;
	int nRow = 0;
	int nColum = 0;

	int res=0;
	char sqlStr[300];
	sprintf(sqlStr, "%s","CREATE TABLE Table_bind_list( \
			targetAddr CHAR(16),\
			targetEndpoint CHAR(4),\
	        pannelAddr CHAR(16));");
	res = sqlite3_exec(bData, sqlStr, 0, 0, &errorStr);


	sprintf(sqlStr, "%s", "SELECT *FROM Table_bind_list");
	res = sqlite3_get_table(bData, sqlStr, &azResult, &nRow, &nColum, &errorStr);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] %s:%s\n\n", sqlStr, errorStr);
		sqlite3_free_table(azResult);
		return -1;
	}

	if (nRow == 0)
	{
		INFO("[ZIGBEE] Table_bind_list is empty\n\n");
	}
	else
	{
		int i = 0;
		for (i = 0; i < nRow; i++)
		{
			BindManage *newNode;
			newNode = (BindManage*)malloc(sizeof(BindManage));

			sprintf(newNode->targetAddr, azResult[(1 + i)*nColum + 0]);
			newNode->targetEndpoint=atoi(azResult[(1+i)*nColum+1]);
			sprintf(newNode->pannelAddr, azResult[(1+i)*nColum+2]);
			newNode->next = NULL;

			if (bindHead == NULL)
			{
				bindHead = newNode;
				bindTail = newNode;
			}
			else
			{
				bindTail->next = newNode;
				bindTail = newNode;
			}
		}
		sqlite3_free_table(azResult);
	}
	return 0;

}

int bindAddItem(char *targetAddr,int targetEndpoint,char *pannelAddr)
{

	BindManage *newNode;
	newNode=(BindManage*)malloc(sizeof(BindManage));

	sprintf(newNode->targetAddr,"%s",targetAddr);
	newNode->targetEndpoint=targetEndpoint;
	sprintf(newNode->pannelAddr,"%s",pannelAddr);
	newNode->next = NULL;

	pthread_mutex_lock(&mutexBind);
	BindManage *currentNode;
	currentNode = bindHead;

	while (currentNode != NULL)
	{
		if (strcmp(targetAddr, currentNode->targetAddr) == 0 && strcmp(pannelAddr, currentNode->pannelAddr) == 0\
				&& targetEndpoint==currentNode->targetEndpoint)
		{
			free(newNode);
			pthread_mutex_unlock(&mutexBind);
			return 0;
		}
		else
			currentNode = currentNode->next;
	}



	if (bindHead == NULL)
	{
		bindHead = newNode;
		bindTail = newNode;
	}
	else
	{
		bindTail->next = newNode;
		bindTail = newNode;
	}

	pthread_mutex_unlock(&mutexBind);

	int res=0;
	char *errorStr = NULL;
	char sqlStr[300];

	sprintf(sqlStr, "INSERT INTO Table_bind_list (targetAddr,targetEndpoint,pannelAddr) VALUES (\'%s\',%d,\'%s\');", targetAddr, targetEndpoint, pannelAddr);
	res = sqlite3_exec(bData, sqlStr, 0, 0, &errorStr);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] INSERT targetAddr=%s endpoint=%d pannelAddr=%s : %s\n\n", targetAddr, targetEndpoint,pannelAddr, errorStr);
		res=-1;
	}

	if (errorStr!=NULL)
		free(errorStr);
	return res;
}

int bindDeleteAllItem(char *address)
{
	BindManage *newNode;
	BindManage *preNode;
	BindManage *deleteNode;

	pthread_mutex_lock(&mutexBind);

	newNode=bindHead;
	preNode=bindHead;

	while(newNode!=NULL)
	{
		if (strcmp(newNode->pannelAddr,address)==0 \
				|| strcmp(newNode->targetAddr,address)==0)
		{

			if (newNode == bindHead && newNode == bindTail)
			{

				bindHead = NULL;
				bindTail = NULL;
				deleteNode = newNode;
				newNode = NULL;
			}
			else if (newNode == bindHead && newNode != bindTail)
			{

				bindHead = newNode->next;
				deleteNode = newNode;
				preNode= newNode->next;
				newNode= newNode->next;
			}
			else if (newNode != bindHead && newNode != bindTail)
			{

				preNode->next = newNode->next;
				deleteNode = newNode;
				newNode = newNode->next;
			}
			else if (newNode != bindHead && newNode == bindTail)
			{
				preNode->next = NULL;
				bindTail = preNode;
				deleteNode = newNode;
				newNode = NULL;
			}

			if (deleteNode != NULL)
				free(deleteNode);
		}
		else
		{
			preNode = newNode;
			newNode = newNode->next;
		}
	}
	pthread_mutex_unlock(&mutexBind);

	int res=0;
	char *errorStr = NULL;
	char sqlStr[300];

	sprintf(sqlStr, "DELETE FROM Table_bind_list WHERE targetAddr=\'%s\' OR pannelAddr=\'%s\';", address,address);
	res = sqlite3_exec(bData, sqlStr, 0, 0, &errorStr);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] Delete device=%s from Table_bind_list failed: %s\n\n",address,errorStr);
		res=-1;
	}

	if (errorStr!=NULL)
		free(errorStr);
	return res;
}


int bindDeleteItem(char *targetAddr,int targetEndpoint,char *pannelAddr)
{

	BindManage *newNode;
	BindManage *preNode;
	BindManage *deleteNode;

	pthread_mutex_lock(&mutexBind);

	newNode=bindHead;
	preNode=bindHead;

	while(newNode!=NULL)
	{
		if (strcmp(newNode->pannelAddr,pannelAddr)==0 \
				&& strcmp(newNode->targetAddr,targetAddr)==0 \
				&& newNode->targetEndpoint==targetEndpoint)
		{

			if (newNode == bindHead && newNode == bindTail)
			{

				bindHead = NULL;
				bindTail = NULL;
				deleteNode = newNode;
				newNode = NULL;
			}
			else if (newNode == bindHead && newNode != bindTail)
			{

				bindHead = newNode->next;
				deleteNode = newNode;
				preNode= newNode->next;
				newNode= newNode->next;
			}
			else if (newNode != bindHead && newNode != bindTail)
			{

				preNode->next = newNode->next;
				deleteNode = newNode;
				newNode = newNode->next;
			}
			else if (newNode != bindHead && newNode == bindTail)
			{
				preNode->next = NULL;
				bindTail = preNode;
				deleteNode = newNode;
				newNode = NULL;
			}

			if (deleteNode != NULL)
				free(deleteNode);
		}
		else
		{
			preNode = newNode;
			newNode = newNode->next;
		}
	}

	pthread_mutex_unlock(&mutexBind);


	int res=0;
	char *errorStr = NULL;
	char sqlStr[300];

	sprintf(sqlStr, "DELETE FROM Table_bind_list WHERE targetAddr=\'%s\' AND pannelAddr=\'%s\' AND targetEndpoint=%d;",\
			targetAddr,pannelAddr,targetEndpoint);
	res = sqlite3_exec(bData, sqlStr, 0, 0, &errorStr);
	if (res != SQLITE_OK)
	{
		ERROR("[ZIGBEE] Delete targetAddr=%s pannelAddr=%s targetEndpoint=%d from Table_bind_list failed: %s\n\n",\
				targetAddr,pannelAddr,targetEndpoint,errorStr);
		res=-1;
	}

	if (errorStr!=NULL)
		free(errorStr);
	return res;
}


int bindClearList()
{
	BindManage *newNode;
	BindManage *deleteNode;

	pthread_mutex_lock(&mutexBind);
	newNode=bindHead;
	bindHead=NULL;
	bindTail=NULL;
	pthread_mutex_unlock(&mutexBind);
	while(newNode!=NULL)
	{
		deleteNode=newNode;
		newNode=newNode->next;
		free(deleteNode);
	}

	char *zErrMsg = NULL;
	char sqlStr[300];
	sprintf(sqlStr, "DELETE FROM Table_bind_list;");
	sqlite3_exec(bData, sqlStr, NULL, NULL, &zErrMsg);
	if (zErrMsg!=NULL)
		free(zErrMsg);
	return 0;
}



int bindCheck(char *pannelAddr,char *targetAddr,int *targetEndpoint)
{

	int res=-1;
	BindManage *newNode;

	pthread_mutex_lock(&mutexBind);

	newNode=bindHead;

	while(newNode!=NULL)
	{
		if (strcmp(newNode->pannelAddr,pannelAddr)==0)
		{
			sprintf(targetAddr,"%s",newNode->targetAddr);
			*targetEndpoint=newNode->targetEndpoint;
			res=0;
			break;
		}
		else
		{
			newNode = newNode->next;
		}
	}
	pthread_mutex_unlock(&mutexBind);

	return res;
}


int bindQueryList(char *targetAddr,char *outBuff)
{
	BindManage *newNode;

	cJSON *root;
	root=cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "AirBind");
	cJSON *data;
	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	cJSON *list;
	cJSON_AddItemToArray(data, list=cJSON_CreateObject());

	cJSON_AddStringToObject(list, "DeviceId", targetAddr);
	cJSON_AddStringToObject(list, "Key", "Q");

	cJSON *value;
	cJSON_AddItemToObject(list,"Value",value=cJSON_CreateArray());

	pthread_mutex_lock(&mutexBind);

	newNode=bindHead;
	while(newNode!=NULL)
	{
		if (strcmp(targetAddr,newNode->targetAddr)==0)
		{
			cJSON *item;
			item=cJSON_CreateObject();

			cJSON_AddStringToObject(item,"PannelAddr",newNode->pannelAddr);
			char endpoint[4]={0};
			sprintf(endpoint,"%d",newNode->targetEndpoint);
			cJSON_AddStringToObject(item,"Endpoint",endpoint);
			cJSON_AddItemToArray(value,item);
		}
		newNode=newNode->next;
	}
	pthread_mutex_unlock(&mutexBind);

	char *strJson;
	strJson=cJSON_Print(root);
	DEBUG("[ZIGBEE] The length of reported bind list string is %d\n\n",strlen(strJson));

	sprintf(outBuff,"%s",strJson);
	cJSON_Delete(root);
	if (strJson!=NULL)
		free(strJson);

	return 0;

}

void packageAirBindResult(char *deviceId, char *key, char *pannelAddr,char *endpont,char *result,char*outBuff)
{
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "AirBind");
	cJSON *data;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	cJSON *list;
	cJSON_AddItemToArray(data, list=cJSON_CreateObject());

	cJSON_AddStringToObject(list, "DeviceId", deviceId);
	cJSON_AddStringToObject(list, "Key", key);

	cJSON *value;
	if (strcmp(key,"A")==0 || strcmp(key,"D")==0)
	{
		cJSON_AddItemToObject(list,"Value",value=cJSON_CreateObject());
		cJSON_AddStringToObject(value, "PannelAddr", pannelAddr);
		cJSON_AddStringToObject(value, "Endpoint", endpont);
		cJSON_AddStringToObject(value, "Result", result);
	}
	else if (strcmp(key,"C")==0)
	{
		cJSON_AddItemToObject(list,"Value",value=cJSON_CreateObject());
		cJSON_AddStringToObject(value, "Result", result);
	}

	char *strJson=cJSON_Print(root);
	sprintf(outBuff,"%s",strJson);

	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);


}


