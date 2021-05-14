
#include "hyprotocol.h"
#include "cJSON.h"
#include "string.h"
#include "stdio.h"
#include "stdlib.h"
#include "log.h"
#include "common.h"
#include "deviceList.h"

#include <stdarg.h>

extern int devcieNumber;

static int alreadRepNum = 0;

int packageReportMessageToJson(const char *reportType, void* reportMsgStruct, char *outBuff)
{
	cJSON *root;
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");/*��������FrameNumber����*/
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);

	cJSON *data;
	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());


	cJSON *dataList;
	int res = -1;

	if (strcmp(reportType, REPORT_TYPE_REGISTER) == 0)
	{
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_REGISTER);

		ReportRegister *p;
		p = (ReportRegister *)reportMsgStruct;
		while (p != NULL)
		{
			cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
			cJSON_AddStringToObject(dataList, "DeviceId", p->deviceId);
			cJSON_AddStringToObject(dataList, "ModelId", p->modelId);
			cJSON_AddStringToObject(dataList, "RealModelId", p->realModelId);
			cJSON_AddStringToObject(dataList, "Secret", p->secret);
			//cJSON_AddStringToObject(dataList, "DeviceType", p->subDevType);
			p = p->next;
		}
		res = 0;
	}
	else if (strcmp(reportType, REPORT_TYPE_UNREGISTER) == 0)
	{
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_UNREGISTER);
		cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());

		char *address;
		address = (char*)reportMsgStruct;

		cJSON_AddStringToObject(dataList, "DeviceId", address);
		res = 0;
	}
	else if (strcmp(reportType, REPORT_TYPE_ATTRIBUTE) == 0 \
			|| strcmp(reportType, REPORT_TYPE_BIND) == 0 \
			|| strcmp(reportType, REPORT_TYPE_SUBNAME) == 0 \
			|| strcmp(reportType,REPORT_TYPE_EXT_QUERY)==0)
	{
		if (strcmp(reportType, REPORT_TYPE_BIND) == 0)
			cJSON_AddStringToObject(root, "Type", REPORT_TYPE_BIND);
		else if(strcmp(reportType, REPORT_TYPE_SUBNAME) == 0)
			cJSON_AddStringToObject(root, "Type", REPORT_TYPE_SUBNAME);
		else if (strcmp(reportType,REPORT_TYPE_EXT_QUERY)==0)
		{
			cJSON_AddStringToObject(root, "Type", REPORT_TYPE_ATTRIBUTE);
			cJSON_AddStringToObject(root, "ExtType", REPORT_TYPE_EXT_QUERY);
		}
		else
			cJSON_AddStringToObject(root, "Type", REPORT_TYPE_ATTRIBUTE);
		ReportAttrEventOnline *p;
		p = (ReportAttrEventOnline *)reportMsgStruct;

		while (p != NULL)
		{
			cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
			cJSON_AddStringToObject(dataList, "DeviceId", p->deviceId);
			cJSON_AddStringToObject(dataList, "ModelId", p->modelId);
			cJSON_AddStringToObject(dataList, "Key", p->key);
			cJSON_AddStringToObject(dataList, "Value", p->value);
			p = p->next;
		}
		res = 0;
	}
	else if(strcmp(reportType, REPORT_TYPE_DEVATTRI) == 0)
	{
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_DEVATTRI);

		ReportAttrEventOnline *p;
		p = (ReportAttrEventOnline *)reportMsgStruct;

		while (p != NULL)
		{
			cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
			cJSON_AddStringToObject(dataList, "DeviceId", p->deviceId);
			cJSON_AddStringToObject(dataList, "ModelId", p->modelId);
			cJSON_AddStringToObject(dataList, "Key", p->key);
			cJSON_AddStringToObject(dataList, "Value", p->value);
			p = p->next;
		}
		res = 0;
	}
	else if (strcmp(reportType, REPORT_TYPE_EVENT) == 0)
	{
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_EVENT);

		ReportAttrEventOnline *p;
		p = (ReportAttrEventOnline *)reportMsgStruct;

		while (p != NULL)
		{
			cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
			cJSON_AddStringToObject(dataList, "DeviceId", p->deviceId);
			cJSON_AddStringToObject(dataList, "ModelId", p->modelId);
			cJSON_AddStringToObject(dataList, "Key", p->key);
			cJSON_AddStringToObject(dataList, "Value", p->value);
			p = p->next;
		}
		res = 0;
	}
	else if (strcmp(reportType, REPORT_TYPE_ONOFF) == 0)
	{
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_ONOFF);

		ReportAttrEventOnline *p;
		p = (ReportAttrEventOnline *)reportMsgStruct;

		while (p != NULL)
		{
			cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
			cJSON_AddStringToObject(dataList, "DeviceId", p->deviceId);
			cJSON_AddStringToObject(dataList, "ModelId", p->modelId);
			cJSON_AddStringToObject(dataList, "Key", p->key);
			cJSON_AddStringToObject(dataList, "Value", p->value);
			p = p->next;
		}
		res = 0;
	}
	else if (strcmp(reportType, REPORT_TYPE_REFACTORY) == 0)
	{
		//cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_REFACTORY);
		res = 0;
	}
	else if (strcmp(reportType, REPORT_TYPE_DEVLIST) == 0)
	{
		char tmp[4]={0};
		ReportRegister *p;
		p = (ReportRegister *)reportMsgStruct;

		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_DEVLIST);
		if (p == NULL)
		{
			cJSON_AddStringToObject(root, "TotalNumber", "0");
			p = NULL;
		}
		else
		{
			sprintf(tmp, "%d", devcieNumber);
			cJSON_AddStringToObject(root, "TotalNumber", tmp);
		}
		

		int count = 0;
		while (p != NULL)
		{
			cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
			cJSON_AddStringToObject(dataList, "DeviceId", p->deviceId);
			printf("deviceId = %s\n", p->deviceId);
			cJSON_AddStringToObject(dataList, "ModelId", p->modelId);
			cJSON_AddStringToObject(dataList, "RealModelId", p->realModelId);
			cJSON_AddStringToObject(dataList, "HardwareVersion", p->hardwareVersion);
			//cJSON_AddStringToObject(dataList, "Secret", p->secret);

			cJSON_AddStringToObject(dataList, "Name", p->name);
			cJSON_AddStringToObject(dataList, "Version", p->version);
			char online[2];
			sprintf(online,"%d",p->online);
			cJSON_AddStringToObject(dataList, "Online", online);
			char reg[2];
			sprintf(reg,"%d",p->registerStatus);
			cJSON_AddStringToObject(dataList, "RegisterStatus", reg);

			p = p->next;
			count = count + 1;

			//if (p != NULL)
				//cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
		}

		alreadRepNum = alreadRepNum + count;
		sprintf(tmp, "%d", alreadRepNum);

		cJSON_AddStringToObject(root, "AlreadyReportNumber", tmp);
		if (alreadRepNum == devcieNumber)
		{
			alreadRepNum = 0;
			res = 0;
		}
		else if (alreadRepNum < devcieNumber)
		{
			res = 0;
		}
		else if (alreadRepNum > devcieNumber)
		{
			ERROR("[ZIGBEE] alreadRepNum > devcieNumber\n\n");
			res=-1;
		}

	}
	else if (strcmp(reportType, REPORT_TYPE_COO_INFO) == 0)
	{
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_COO_INFO);

		char *p = (char *)reportMsgStruct;

		cJSON *cooInfo;
		cooInfo=cJSON_Parse(p);
		if (cooInfo!=NULL)
		{
			cJSON_AddItemToArray(data, dataList = cooInfo);
			res=0;
		}
		else
			res=-1;

	}
	else if (strcmp(reportType, REPORT_TYPE_NEIGHBOR_INFO) == 0)
	{
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_NEIGHBOR_INFO);

		char *p = (char *)reportMsgStruct;

		cJSON *neighbor;
		neighbor=cJSON_Parse(p);
		if (neighbor!=NULL)
		{
			cJSON_AddItemToArray(data, dataList = neighbor);
			res=0;
		}
		else
			res=-1;
	}
	else if (strcmp(reportType, REPORT_TYPE_CHILDREN_INFO) == 0)
	{
		cJSON_AddStringToObject(root, "Type", REPORT_TYPE_CHILDREN_INFO);

		char *p = (char *)reportMsgStruct;

		cJSON *children;
		children=cJSON_Parse(p);
		if (children!=NULL)
		{
			cJSON_AddItemToArray(data, dataList = children);
			res=0;
		}
		else
			res=-1;
	}

	if (res == 0)
	{
		char *jsonString;
		jsonString = cJSON_Print(root);
		sprintf(outBuff, "%s", jsonString);
		if (jsonString)
		{
			free(jsonString);
		}
	}
	cJSON_Delete(root);
	return res;
}

void freeReportAttrList(ReportAttrEventOnline *reportAttrHead)
{
	ReportAttrEventOnline *currentNode;
	ReportAttrEventOnline *deleteNode;
	currentNode = reportAttrHead;
	reportAttrHead = NULL;
	while (currentNode != NULL)
	{
		deleteNode = currentNode;
		currentNode = currentNode->next;
		free(deleteNode);
	}
}


int unpackReceiveCommandString(char *recvJsonString, RecvCommand *command)
{
	command->number = 0;

	cJSON *root;
	root = cJSON_Parse(recvJsonString);

	if (root == NULL)
	{
		ERROR("[ZIGBEE] Received string Json_Parse failed : %s\n\n",recvJsonString);
		return -1;
	}


	cJSON *item;

	item = cJSON_GetObjectItem(root, "Type");
	if (item)
	{

		sprintf(command->commandType, "%s", item->valuestring);
		if (strcmp(command->commandType,"FastJoin")==0)
		{
			/*快速入网指令不符合以下解析规则，直接跳出*/
			cJSON_Delete(root);
			return 0;
		}

	}
	else
	{
		ERROR("The command doestn't comply with agreement\n\n");
		cJSON_Delete(root);
		return -1;
	}


	cJSON *data;
	data = cJSON_GetObjectItem(root, "Data");

	int arraySize = cJSON_GetArraySize(data);

	cJSON *subData;
	cJSON *subItem;
	int i = 0;
	KeyValue *pKeyValueHead = NULL;
	KeyValue *pKeyValueTail = NULL;

	for (i = 0; i < arraySize; i++)
	{

		subData = cJSON_GetArrayItem(data, i);
		KeyValue *p;
		p = (KeyValue *)malloc(sizeof(KeyValue));
		memset(p, 0, sizeof(KeyValue));
		p->next = NULL;
		subItem = cJSON_GetObjectItem(subData, "DeviceId");
		if (subItem != NULL)
		{
			sprintf(p->address, "%s", subItem->valuestring);
		}
		subItem = cJSON_GetObjectItem(subData, "GroupId");
		if (subItem != NULL)
		{
			//sprintf(p->address, "%s", subItem->valuestring);
			sprintf(p->groupId, "%s", subItem->valuestring);
		}
		subItem = cJSON_GetObjectItem(subData, "Type");
		if (subItem != NULL)
		{
			sprintf(p->type, "%s", subItem->valuestring);
		}

		subItem = cJSON_GetObjectItem(subData, "Op");
		if (subItem != NULL)
		{
			sprintf(p->type, "%s", subItem->valuestring);
		}


		subItem = cJSON_GetObjectItem(subData, "Endpoint");
		if (subItem != NULL)
		{
			sprintf(p->endpoint, "%s", subItem->valuestring);
		}

		subItem = cJSON_GetObjectItem(subData, "Key");
		if (subItem != NULL)
		{
			sprintf(p->key, "%s", subItem->valuestring);
		}
		subItem = cJSON_GetObjectItem(subData, "Value");
		if (subItem != NULL)
		{
			if (subItem->type==cJSON_Object)
			{
				char *strJson=NULL;
				strJson=cJSON_Print(subItem);
				sprintf(p->value, "%s", strJson);
				if (strJson!=NULL)
					free(strJson);
			}
			else if (subItem->type==cJSON_String)
			{
				sprintf(p->value, "%s", subItem->valuestring);
			}

		}

		subItem = cJSON_GetObjectItem(subData, "Name");
		if (subItem != NULL)
		{
			sprintf(p->name, "%s", subItem->valuestring);
		}

		subItem = cJSON_GetObjectItem(subData, "Enable");
		if (subItem != NULL)
		{
			sprintf(p->enable, "%s", subItem->valuestring);
		}

		subItem = cJSON_GetObjectItem(subData, "Items");
		if (subItem != NULL && subItem->type==cJSON_Array)
		{
			char *strJson=NULL;
			strJson=cJSON_Print(subItem);
			sprintf(p->value, "%s", strJson);
			if (strJson!=NULL)
				free(strJson);
		}


		if (pKeyValueHead == NULL)
		{
			pKeyValueHead = p;
			pKeyValueTail = p;
		}
		else
		{
			pKeyValueTail->next = p;
			pKeyValueTail = p;
		}
		command->number++;
	}

	command->keyValueHead = pKeyValueHead;

	cJSON_Delete(root);
	return 0;

}


void freeCommandKeyValueList(KeyValue *keyValueHead)
{
	KeyValue *deleteNode;

	KeyValue *currentNode;
	currentNode = keyValueHead;
	keyValueHead = NULL;

	while (currentNode != NULL)
	{
		deleteNode = currentNode;
		currentNode = currentNode->next;
		free(deleteNode);
	}
}



void packageBindTalbeToJson(char *deviceId, char* modelId, cJSON *rexBindTable,char *outBuff)
{
	char *strJson=cJSON_Print(rexBindTable);

	cJSON *list=cJSON_Parse(strJson);
	cJSON_AddStringToObject(list, "DeviceId", deviceId);
	cJSON_AddStringToObject(list, "ModelId", modelId);
	cJSON_AddStringToObject(list, "Key", "Q");

	cJSON *root;
	root = cJSON_CreateObject();


	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "Bind");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);
	cJSON *data;
	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());

	cJSON_AddItemToArray(data, list);

	char *strTmp=cJSON_Print(root);
	sprintf(outBuff,"%s",strTmp);
	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
	if (strTmp!=NULL)
		free(strTmp);
}

void packageGroupResult(char *deviceId, char* modelId, char *key, char *groupid,char *result,unsigned char endpoint_id,char*outBuff)
{
	cJSON *root;
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "Group");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);
	cJSON *data;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	cJSON *list;
	cJSON_AddItemToArray(data, list=cJSON_CreateObject());

	cJSON_AddStringToObject(list, "DeviceId", deviceId);
	cJSON_AddStringToObject(list, "ModelId", modelId);
	if(groupid!=NULL)
		cJSON_AddStringToObject(list, "GroupId", groupid);

	char endpoint[4];
	sprintf(endpoint,"%d",endpoint_id);
	cJSON_AddStringToObject(list, "Endpoint", endpoint);

	cJSON_AddStringToObject(list, "Key", key);
	cJSON_AddStringToObject(list, "Value", result);

	char *strJson=cJSON_Print(root);
	sprintf(outBuff,"%s",strJson);

	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);

}

void packageGroupNumber(char *deviceId, char* modelId, char *key, cJSON *number,unsigned char endpoint_id,char*outBuff)
{
	char *strTmp=cJSON_Print(number);

	cJSON *num=cJSON_Parse(strTmp);
	if (num!=NULL)
	{
		if (num->type!=cJSON_Array)
		{
			ERROR("[ZIGBEE] Input cJSON *number is not array\n\n");
			cJSON_Delete(num);
			if (strTmp!=NULL)
				free(strTmp);
			return ;
		}
	}
	else
	{
		ERROR("[ZIGBEE] string cJSON_Parse failed\n\n");
		if (strTmp!=NULL)
			free(strTmp);
		return ;
	}



	cJSON *root;
	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "Group");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);
	cJSON *data;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	cJSON *list;
	cJSON_AddItemToArray(data, list=cJSON_CreateObject());

	cJSON_AddStringToObject(list, "DeviceId", deviceId);
	cJSON_AddStringToObject(list, "ModelId", modelId);
	char endpoint[4];
	sprintf(endpoint,"%d",endpoint_id);
	cJSON_AddStringToObject(list, "Endpoint", endpoint);
	cJSON_AddStringToObject(list, "Key", key);
	cJSON_AddItemToObject(list,"Value",num);

	char *strJson=cJSON_Print(root);
	sprintf(outBuff,"%s",strJson);

	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
	if (strTmp!=NULL)
		free(strTmp);
}


void packageSceneResult(char *deviceId, char* modelId, char *key, char *result,char*outBuff)
{
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "Scene");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);
	cJSON *data;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	cJSON *list;
	cJSON_AddItemToArray(data, list=cJSON_CreateObject());

	cJSON_AddStringToObject(list, "DeviceId", deviceId);
	cJSON_AddStringToObject(list, "ModelId", modelId);
	cJSON_AddStringToObject(list, "Key", key);
	cJSON *value;
	value=cJSON_Parse(result);
	cJSON_AddItemToObject(list, "Value", value);

	char *strJson=cJSON_Print(root);
	sprintf(outBuff,"%s",strJson);

	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
}


void packageUnknowDeviceInfo(char *deviceId, char* modelId,char *type,char *outBuff)
{
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "Unknow");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);
	cJSON *data;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	cJSON *list;
	cJSON_AddItemToArray(data, list=cJSON_CreateObject());
	cJSON_AddStringToObject(list, "DeviceId", deviceId);
	cJSON_AddStringToObject(list, "ModelId", modelId);
	cJSON_AddStringToObject(list, "RexType", type);


	char *strJson=cJSON_Print(root);
	sprintf(outBuff,"%s",strJson);

	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
}

void  packageNeworkReset(char *outBuff)
{
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "ReNetwork");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);
	cJSON *data;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	char *strJson=cJSON_PrintUnformatted(root);
	sprintf(outBuff,"%s",strJson);

	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
}

void packageDeviceOtaResult(char *deviceId,char *modelId,char *errorCOde,char *outbuff)
{
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "DevOta");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);
	cJSON *data;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());

	cJSON *list;
	cJSON_AddItemToArray(data, list=cJSON_CreateObject());
	cJSON_AddStringToObject(list, "DeviceId", deviceId);
	cJSON_AddStringToObject(list, "ModelId", modelId);
	cJSON_AddStringToObject(list, "Key", "Result");
	cJSON_AddStringToObject(list, "Value", errorCOde);
	char *strJson=cJSON_PrintUnformatted(root);
	sprintf(outbuff,"%s",strJson);

	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
}

void packageRestartAck(char *outbuff)
{
	cJSON *root;
	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Command", "Report");
	cJSON_AddStringToObject(root, "FrameNumber", "00");
	cJSON_AddStringToObject(root, "Type", "Restart");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);
	cJSON *data;

	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	char *strJson=cJSON_PrintUnformatted(root);
	sprintf(outbuff,"%s",strJson);

	cJSON_Delete(root);
	if(strJson!=NULL)
		free(strJson);
}


