/*
 * signature.c
 *
 *  Created on: Nov 9, 2018
 *      Author: jerry
 */

#include "hyprotocol.h"
#include "rexCommand.h"
#include "cJSON.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include "common.h"
#include "deviceList.h"



/*************************
 *函数名：handleSetGatewaySignaturCommand
 *描述： 添加网关签名信息指令;
 *参数：@ RecvCommand *command
 *返回：0成功，-1失败;
 ************************/
int handleSetGatewaySignaturCommand(KeyValue *command)
{
	int res=0;
	KeyValue *currentNode=command;

	while(currentNode!=NULL)
	{
		if (strcmp(currentNode->key,"Clear")==0)
		{
			res=sendCommandSetSignature("0","");
		}
		else if (strcmp(currentNode->key,"ProductModel")==0)
		{
			res=sendCommandSetSignature("1",currentNode->value);
		}
		else if (strcmp(currentNode->key,"ProductKey")==0)
		{
			res=sendCommandSetSignature("2",currentNode->value);
		}
		else if (strcmp(currentNode->key,"ProductSecret")==0)
		{
			res=sendCommandSetSignature("3",currentNode->value);
		}
		else if (strcmp(currentNode->key,"ProductName")==0)
		{
			res=sendCommandSetSignature("4",currentNode->value);
		}
		else if (strcmp(currentNode->key,"DeviceSecret")==0)
		{
			res=sendCommandSetSignature("5",currentNode->value);
		}
		else if (strcmp(currentNode->key,"DeviceKey")==0)
		{
			res=sendCommandSetSignature("6",currentNode->value);
		}

		currentNode=currentNode->next;
	}
	return res;

}

/*************************
 *函数名：handleGetGatewaySignaturCommand
 *描述： 获取网关签名信息指令;
 *参数：@ RecvCommand *command
 *返回：0成功，-1失败;
 ************************/
int handleGetGatewaySignaturCommand()
{
	return sendCommandGetSignature();
}

/*************************
 *函数名：unpackAndReportSetSignaturReport
 *描述： 解析设置网关签名信息结果指令，并打包向平台上报的信息;
 *参数：@ cJSON *input：输入cJOSN;
 *参数：@ char *outBuff: 输出打包的信息
 *返回：0成功，-1失败;
 ************************/
int unpackAndReportSetSignaturReport(cJSON *input,char *outBuff)
{
	cJSON *respType=cJSON_GetObjectItem(input,"ResponseType");
	cJSON *result=cJSON_GetObjectItem(input,"Result");
	if (respType==NULL || result==NULL)
		return -1;

	char keyName[40];
	if (strcmp(respType->valuestring,"0")==0)
		sprintf(keyName,"Clear");
	else if (strcmp(respType->valuestring,"1")==0)
		sprintf(keyName,"ProductModel");
	else if (strcmp(respType->valuestring,"2")==0)
		sprintf(keyName,"ProductKey");
	else if (strcmp(respType->valuestring,"3")==0)
		sprintf(keyName,"ProductSecret");
	else if (strcmp(respType->valuestring,"4")==0)
		sprintf(keyName,"ProductName");
	else if (strcmp(respType->valuestring,"5")==0)
		sprintf(keyName,"DeviceSecret");
	else if (strcmp(respType->valuestring,"6")==0)
		sprintf(keyName,"DeviceKey");
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Command","Report");
	cJSON_AddStringToObject(root,"FrameNumber","00");
	cJSON_AddStringToObject(root,"Type","SetSig");

	cJSON *data;
	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	cJSON *dataList;
	cJSON_AddItemToArray(data, dataList = cJSON_CreateObject());
	cJSON_AddStringToObject(dataList, "DeviceId", "0000000000000000");
	cJSON_AddStringToObject(dataList, "Key",keyName );
	cJSON_AddStringToObject(dataList, "Value", result->valuestring);

	char *temStr=cJSON_Print(root);
	sprintf(outBuff,"%s",temStr);
	cJSON_Delete(root);
	free(temStr);

	return 0;
}

/*********************
 *函数名：packGatewaySignatureInfoReport
 *描述： 解析设置网关签名信息结果指令，并打包向平台上报的信息;
 *参数：@ cJSON *input：输入cJOSN;
 *参数：@ char *outBuff: 输出打包的信息
 *返回：0成功，-1失败;
 */
int packGatewaySignatureInfoReport(char *input,char *outbuff)
{
	cJSON *tmp=cJSON_Parse(input);
	if (tmp==NULL)
		return -1;
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Command","Report");
	cJSON_AddStringToObject(root,"FrameNumber","00");
	cJSON_AddStringToObject(root,"Type","GetSig");
	char macBuff[DEVICE_DEVICE_ID_LENGTH]={0};
	getCooMac(macBuff);
	cJSON_AddStringToObject(root, "GatewayId",macBuff);


	cJSON *data;
	cJSON_AddItemToObject(root, "Data", data = cJSON_CreateArray());
	//cJSON *dataList;
	cJSON_AddItemToArray(data, tmp);
	cJSON_AddStringToObject(tmp,"DeviceId","0000000000000000");


	char *temStr=cJSON_Print(root);
	sprintf(outbuff,"%s",temStr);
	cJSON_Delete(root);
	free(temStr);
	return 0;
}


