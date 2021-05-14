#include <stdio.h>
#include <dirent.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include "common.h"
#include "cJSON.h"
#include "log.h"

extern char configurFilePath[DEV_CONFIGUR_FILE_PATH_MAX_LENGTH];


#define BUF_MAX_LENGTH		(5000)


int readJsonString(char *fileName,char *buf,int sizeBuf)
{

	int fd;
	char filePath[200];
	sprintf(filePath,"%s/%s", configurFilePath, fileName);

	if ((fd = open(filePath, O_RDONLY)) == -1)
	{
		ERROR("[ZIGBEE] Open configuration file failed: %s\n\n",filePath);
		return -1;
	}

	read(fd, buf, BUF_MAX_LENGTH-1);
	close(fd);
	return 0;
}

DevInfo *josnToStruct(char *buf)
{
	DevInfo *newNode;
	newNode = (DevInfo *)malloc(sizeof(DevInfo));

	memset(newNode, 0, sizeof(DevInfo));

	cJSON *root=NULL;
	root = cJSON_Parse(buf);
	if (root==NULL)
	{
		ERROR("[ZIGBEE] Parse configure file failed\n\n");
		free(newNode);
		return NULL;
	}


	cJSON *basic;
	basic = cJSON_GetObjectItem(root, "Basic");

	cJSON *sub;
	sub = cJSON_GetObjectItem(basic, "SubDeviceType");
	if (sub != NULL)
	{

		newNode->subDevType=sub->valueint;
	}

	sub = cJSON_GetObjectItem(basic, "Name");
	if (sub != NULL)
	{
		sprintf(newNode->name, "%s", sub->valuestring);
	}

	sub = cJSON_GetObjectItem(basic, "ModelId");
	if (sub != NULL)
	{
		sprintf(newNode->modelId, sub->valuestring);
	}

	sub = cJSON_GetObjectItem(basic, "Secret");
	if (sub != NULL)
	{
		sprintf(newNode->secret, "%s", sub->valuestring);
	}

	sub=cJSON_GetObjectItem(basic,"VerKey");
	if (sub != NULL)
	{
		sprintf(newNode->verKey, "%s", sub->valuestring);
	}
	else
	{
		sprintf(newNode->verKey, "%s", "DateCode");
	}

	sub = cJSON_GetObjectItem(basic, "HardwareVersion");
	if (sub != NULL)
	{
		if (0==strlen(sub->valuestring))
			sprintf(newNode->hardwareVersion, "%s", "1.0.0");
		else
			sprintf(newNode->hardwareVersion, "%s", sub->valuestring);
	}

	sub = cJSON_GetObjectItem(basic, "SoftewareVersion");
	if (sub != NULL)
	{
		if (0==strlen(sub->valuestring))
			sprintf(newNode->softwareVersion, "%s", "20180101");
		else
			sprintf(newNode->softwareVersion, "%s", sub->valuestring);
	}


	//newNode->reportTime = 300;
	newNode->registerStatus = 0;

	DevOnlineInfo *newOnlineNode;
	newOnlineNode = (DevOnlineInfo *)malloc(sizeof(DevOnlineInfo));

	newOnlineNode->online = 0;
	newOnlineNode->offCount = 0;
	newOnlineNode->leaveGateway = 1;
	gettimeofday(&newOnlineNode->lastUpdateTime, NULL);

	sub = cJSON_GetObjectItem(basic, "ReportPeriod");
	if (sub != NULL)
		newOnlineNode->reportTime = atoi(sub->valuestring);
	else
		newOnlineNode->reportTime = 300;

	newOnlineNode->next = NULL;
	newNode->onlineInfo = newOnlineNode;

	newNode->next=NULL;

	cJSON *propertys;
	cJSON *item;
	cJSON *subItem;
	propertys = cJSON_GetObjectItem(root, "Propertys");

	int arraySize = cJSON_GetArraySize(propertys);
	int i = 0;
	Attribute *tempAttrTail=NULL;
	Attribute *attr = NULL;
	newNode->attrNum = 0;
	newNode->attrHead = NULL;


	for (i = 0; i < arraySize; i++)
	{
		item = cJSON_GetArrayItem(propertys, i);
		subItem= cJSON_GetObjectItem(item, "Key");

		
		attr = (Attribute *)malloc(sizeof(Attribute));
		memset(attr, 0, sizeof(Attribute));
		sprintf(attr->key, "%s", subItem->valuestring);


		subItem = cJSON_GetObjectItem(item, "unit");
		if(subItem->valuestring[0]!='\0')
		{

			sprintf(attr->unit, "%s", subItem->valuestring);
		}

		subItem = cJSON_GetObjectItem(item, "value");
		if(subItem!=NULL && subItem->valuestring[0]!='\0')
		{
			sprintf(attr->value, "%s", subItem->valuestring);
		}
		else
			sprintf(attr->value, "%s", "0");

		//gettimeofday(&attr->tv,NULL);
		attr->next = NULL;

		if (newNode->attrHead == NULL)
		{
			newNode->attrHead = attr;
			tempAttrTail = attr;
		}
		else
		{
			tempAttrTail->next = attr;
			tempAttrTail = attr;
		}
		newNode->attrNum++;
	}
	cJSON_Delete(root);
	root = NULL;
	return newNode;
}

Attribute *jsonToAttrStruct(const char *buf, int *num)
{
	cJSON *root;
	root = cJSON_Parse(buf);

	cJSON *propertys;
	cJSON *item;
	cJSON *subItem;
	propertys = cJSON_GetObjectItem(root, "Propertys");

	int arraySize = cJSON_GetArraySize(propertys);
	int i = 0;
	Attribute *tempAttrTail=NULL;
	Attribute *tempAttrHead=NULL;
	Attribute *attr=NULL;
	*num=0;
	cJSON *value;
	for (i = 0; i < arraySize; i++)
	{
		item = cJSON_GetArrayItem(propertys, i);
		subItem= cJSON_GetObjectItem(item, "Key");

		value= cJSON_GetObjectItem(item, "Value");
		attr = (Attribute *)malloc(sizeof(Attribute));
		memset(attr, 0, sizeof(Attribute));
		sprintf(attr->key, "%s", subItem->valuestring);


		subItem = cJSON_GetObjectItem(item, "unit");
		if(subItem->valuestring[0]!='\0')
		{

			sprintf(attr->unit, "%s", subItem->valuestring);
		}

		if(value->valuestring[0]!='\0')
		{
			sprintf(attr->value, "%s", value->valuestring);
		}
		else
			sprintf(attr->value, "%s", "0");
		attr->next = NULL;

		if (tempAttrHead == NULL)
		{
			tempAttrHead = attr;
			tempAttrTail = attr;
		}
		else
		{
			tempAttrTail->next = attr;
			tempAttrTail = attr;
		}
		*num=*num+1;
	}
	cJSON_Delete(root);
	return tempAttrHead;
}

int tolower(int c)
{
	if (c >= 'A' && c <= 'Z')
	{
		return c + 'a' - 'A';
	}
	else
	{
		return c;
	}
}

int htoi(char s[])
{
	int i;
	int n = 0;
	if (s[0] == '0' && (s[1] == 'x' || s[1] == 'X'))
	{
		i = 2;
	}
	else
	{
		i = 0;
	}
	for (; (s[i] >= '0' && s[i] <= '9') || (s[i] >= 'a' && s[i] <= 'z') || (s[i] >= 'A' && s[i] <= 'Z'); ++i)
	{
		if (tolower(s[i]) > '9')
		{
			n = 16 * n + (10 + tolower(s[i]) - 'a');
		}
		else
		{
			n = 16 * n + (tolower(s[i]) - '0');
		}
	}
	return n;
}




DevInfo *ZHA_creatDevInfoStruct(char *modelId)
{
	DIR* dirptr;
	struct dirent* entry;
	char buf[BUF_MAX_LENGTH];

	if ((dirptr = opendir(configurFilePath)) == NULL)
	{
		ERROR("[ZIGBEE] Open configurain folder failed: %s\n\n",configurFilePath);
		return NULL;
	}

	DevInfo *newNode = NULL;
	int find = 0;
	for (entry = readdir(dirptr); NULL != entry; entry = readdir(dirptr))
	{
		if (strstr(entry->d_name, modelId) != NULL)
		{
			INFO("[ZIGBEE] Matching to configuration file:%s\n\n",entry->d_name);
			readJsonString(entry->d_name, buf, BUF_MAX_LENGTH);
			newNode = josnToStruct(buf);

			find = 1;
			break;
		}
	}
	if (find == 0)
		WARN("[ZIGBEE] No find configure file: modelId %s\n\n",modelId);

	closedir(dirptr);
	return newNode;
}



Attribute *ZHA_creatDevAttrStruct(const char *modelId, int *attrNum)
{

	DIR* dirptr;
	struct dirent* entry;
	char buf[BUF_MAX_LENGTH];

	if ((dirptr = opendir(configurFilePath)) == NULL)
	{
		ERROR("[ZIGBEE] open configurain directory failed: %s\n\n",configurFilePath);
		return NULL;
	}

	Attribute *newAttrHead = NULL;
	int find = 0;
	for (entry = readdir(dirptr); NULL != entry; entry = readdir(dirptr))
	{
		if (strstr(entry->d_name, modelId) != NULL)
		{
			INFO("[ZIGBEE] Matching to configuration file:%s\n\n",entry->d_name);

			readJsonString(entry->d_name, buf, BUF_MAX_LENGTH);

			newAttrHead = jsonToAttrStruct(buf, attrNum);
			find = 1;
			break;
		}
	}
	if (find == 0)
		WARN("[ZIGBEE] No find configure file: modelId %s\n\n",modelId);

	closedir(dirptr);
	return newAttrHead;
}

