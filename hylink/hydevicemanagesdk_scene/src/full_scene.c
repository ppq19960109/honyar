/*
 * full_scene.c
 *
 *  Created on: Nov 4, 2019
 *      Author: jerry
 */
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "full_scene.h"
#include "rex_export_gateway.h"
#include "log.h"
#include "callback.h"
#include "sqlite3.h"
#include "deviceList.h"
#include "rexCommand.h"

#define FRAME_LENGTH (68)


 /**************************************************************
 函数名：checkXOR
 描述：异或校验;
 参数：@char *input：输入;
 返回：0成功，-1失败；
 **************************************************************/
static unsigned char checkXOR(unsigned char *input,int length)
{
	unsigned char checkChar;
	checkChar=*input^*(input+1);
	int i=0;
	for(i=0;i<length-2;i++)
	{

		checkChar=checkChar^*(input+2+i);
	}

	return checkChar;
}


/**************************************************************
函数名：fullscene_SetSceneName
描述：设置场景按键名称
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int fullscene_SetSceneName(char *devicId,char *keyNumber,char *name)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0xA1;

	unsigned char len=strlen(name);
	if (len<=0)
		return -1;
	strncpy(&outbuff[10],name,len);
	outbuff[8]=len+1;

	if(strcmp(keyNumber,"SceName_1")==0)
		outbuff[9]=0x01;
	else if(strcmp(keyNumber,"SceName_2")==0)
		outbuff[9]=0x02;
	else if(strcmp(keyNumber,"SceName_3")==0)
		outbuff[9]=0x03;
	else if(strcmp(keyNumber,"SceName_4")==0)
		outbuff[9]=0x04;
	else if(strcmp(keyNumber,"SceName_5")==0)
		outbuff[9]=0x05;
	else if(strcmp(keyNumber,"SceName_6")==0)
		outbuff[9]=0x06;
	else if(strcmp(keyNumber,"SceName_7")==0)
		outbuff[9]=0x07;
	else if(strcmp(keyNumber,"SceName_8")==0)
		outbuff[9]=0x08;
	else if(strcmp(keyNumber,"SceName_9")==0)
		outbuff[9]=0x09;
	else if(strcmp(keyNumber,"SceName_10")==0)
		outbuff[9]=0x0A;
	else if(strcmp(keyNumber,"SceName_11")==0)
		outbuff[9]=0x0B;
	else if(strcmp(keyNumber,"SceName_12")==0)
		outbuff[9]=0x0C;
	else
		return -1;

	outbuff[1]=checkXOR((unsigned char*)outbuff+2,8+len);
	LOG_HEX(LOG_DEBUG,outbuff,0,10+len);
	return rex_send_self_defining_data(devicId,0,1,outbuff,10+len);
}

/**************************************************************
函数名：fullscene_GetSceneName
描述：查询场景按键名称
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetSceneName(char *devicId,char *keyNumber)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0xA3;

	outbuff[8]=0x01;

	if(keyNumber==NULL)
		outbuff[9]=0x00;
	else if (strcmp(keyNumber,"1")==0)
		outbuff[9]=0x01;
	else if(strcmp(keyNumber,"2")==0)
		outbuff[9]=0x02;
	else if(strcmp(keyNumber,"3")==0)
		outbuff[9]=0x03;
	else if(strcmp(keyNumber,"4")==0)
		outbuff[9]=0x04;
	else if(strcmp(keyNumber,"5")==0)
		outbuff[9]=0x05;
	else if(strcmp(keyNumber,"6")==0)
		outbuff[9]=0x06;
	else if(strcmp(keyNumber,"7")==0)
		outbuff[9]=0x07;
	else if(strcmp(keyNumber,"8")==0)
		outbuff[9]=0x08;
	else if(strcmp(keyNumber,"9")==0)
		outbuff[9]=0x09;
	else if(strcmp(keyNumber,"10")==0)
		outbuff[9]=0x0A;
	else if(strcmp(keyNumber,"11")==0)
		outbuff[9]=0x0B;
	else if(strcmp(keyNumber,"12")==0)
		outbuff[9]=0x0C;
	else if(strcmp(keyNumber,"13")==0)
		outbuff[9]=0x0D;
	else if(strcmp(keyNumber,"14")==0)
		outbuff[9]=0x0E;
	else if(strcmp(keyNumber,"15")==0)
		outbuff[9]=0x0F;
	else if(strcmp(keyNumber,"16")==0)
		outbuff[9]=0x10;
	else if(strcmp(keyNumber,"17")==0)
		outbuff[9]=0x11;
	else if(strcmp(keyNumber,"18")==0)
		outbuff[9]=0x12;
	else
		outbuff[9]=0x00;

	outbuff[1]=checkXOR((unsigned char*)outbuff+2,8);
	return rex_send_self_defining_data(devicId,0,1,outbuff,10);
}


/**************************************************************
函数名：fullscene_SetScenePhoto
描述：设置场景按键图片
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int fullscene_SetScenePhoto(char *devicId,char *keyNumber,int photoNumber)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0xA2;

	outbuff[8]=2;

	if(strcmp(keyNumber,"ScePhoto_1")==0)
		outbuff[9]=0x01;
	else if(strcmp(keyNumber,"ScePhoto_2")==0)
		outbuff[9]=0x02;
	else if(strcmp(keyNumber,"ScePhoto_3")==0)
		outbuff[9]=0x03;
	else if(strcmp(keyNumber,"ScePhoto_4")==0)
		outbuff[9]=0x04;
	else if(strcmp(keyNumber,"ScePhoto_5")==0)
		outbuff[9]=0x05;
	else if(strcmp(keyNumber,"ScePhoto_6")==0)
		outbuff[9]=0x06;
	else if(strcmp(keyNumber,"ScePhoto_7")==0)
		outbuff[9]=0x07;
	else if(strcmp(keyNumber,"ScePhoto_8")==0)
		outbuff[9]=0x08;
	else if(strcmp(keyNumber,"ScePhoto_9")==0)
		outbuff[9]=0x09;
	else if(strcmp(keyNumber,"ScePhoto_10")==0)
		outbuff[9]=0x0A;
	else if(strcmp(keyNumber,"ScePhoto_11")==0)
		outbuff[9]=0x0B;
	else if(strcmp(keyNumber,"ScePhoto_12")==0)
		outbuff[9]=0x0C;
	else if(strcmp(keyNumber,"ScePhoto_13")==0)
		outbuff[9]=0x0D;
	else if(strcmp(keyNumber,"ScePhoto_14")==0)
		outbuff[9]=0x0E;
	else if(strcmp(keyNumber,"ScePhoto_15")==0)
		outbuff[9]=0x0F;
	else if(strcmp(keyNumber,"ScePhoto_16")==0)
		outbuff[9]=0x10;
	else if(strcmp(keyNumber,"ScePhoto_17")==0)
		outbuff[9]=0x11;
	else if(strcmp(keyNumber,"ScePhoto_18")==0)
		outbuff[9]=0x12;
	else
		return -1;

	outbuff[10]=(unsigned char)photoNumber;
	outbuff[1]=checkXOR((unsigned char*)outbuff+2,9);

	return rex_send_self_defining_data(devicId,0,1,outbuff,11);
}

/**************************************************************
函数名：fullscene_GetScenePhoto
描述：查询场景按键图片
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetScenePhoto(char *devicId,char *keyNumber)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0xA4;

	outbuff[8]=0x01;

	if(keyNumber==NULL)
		outbuff[9]=0x00;
	else if(strcmp(keyNumber,"1")==0)
		outbuff[9]=0x01;
	else if(strcmp(keyNumber,"2")==0)
		outbuff[9]=0x02;
	else if(strcmp(keyNumber,"3")==0)
		outbuff[9]=0x03;
	else if(strcmp(keyNumber,"4")==0)
		outbuff[9]=0x04;
	else if(strcmp(keyNumber,"5")==0)
		outbuff[9]=0x05;
	else if(strcmp(keyNumber,"6")==0)
		outbuff[9]=0x06;
	else if(strcmp(keyNumber,"7")==0)
		outbuff[9]=0x07;
	else if(strcmp(keyNumber,"8")==0)
		outbuff[9]=0x08;
	else if(strcmp(keyNumber,"9")==0)
		outbuff[9]=0x09;
	else if(strcmp(keyNumber,"10")==0)
		outbuff[9]=0x0A;
	else if(strcmp(keyNumber,"11")==0)
		outbuff[9]=0x0B;
	else if(strcmp(keyNumber,"12")==0)
		outbuff[9]=0x0C;
	else if(strcmp(keyNumber,"13")==0)
		outbuff[9]=0x0D;
	else if(strcmp(keyNumber,"14")==0)
		outbuff[9]=0x0E;
	else if(strcmp(keyNumber,"15")==0)
		outbuff[9]=0x0F;
	else if(strcmp(keyNumber,"16")==0)
		outbuff[9]=0x10;
	else if(strcmp(keyNumber,"17")==0)
		outbuff[9]=0x11;
	else if(strcmp(keyNumber,"18")==0)
		outbuff[9]=0x12;
	else
		outbuff[9]=0x00;

	outbuff[1]=checkXOR((unsigned char*)outbuff+2,8);
	//LOG_HEX(LOG_DEBUG,outbuff,0,10);
	return rex_send_self_defining_data(devicId,0,1,outbuff,10);
}

/**************************************************************
函数名：fullscene_EnableDevie
描述：设置场景按键图片
参数：@char *devicId;
@char *attriNumber
@,char enableCode
返回：0成功，-1失败；
**************************************************************/
int fullscene_EnableDevie(char *devicId,char *attriNumber,char enableCode)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0xA5;

	outbuff[8]=2;

	if(strncmp(attriNumber,"Enable_1",8)==0)
		outbuff[9]=0x01;
	else if(strncmp(attriNumber,"Enable_2",8)==0)
		outbuff[9]=0x03;
	else if(strncmp(attriNumber,"Enable_3",8)==0)
		outbuff[9]=0x02;
	else if(strncmp(attriNumber,"Enable_4",8)==0)
		outbuff[9]=0x04;
	else if(strncmp(attriNumber,"Enable_5",8)==0)
		outbuff[9]=0x05;
	else if(strncmp(attriNumber,"Enable_6",8)==0)
		outbuff[9]=0x06;
	else
		return -1;

	outbuff[10]=enableCode;
	outbuff[1]=checkXOR((unsigned char*)outbuff+2,9);

	return rex_send_self_defining_data(devicId,0,1,outbuff,11);
}

/**************************************************************
函数名：checkReportData
描述：校验全面屏上报自定义数据
参数：@char *data：输入数据首地址;
@ unsigned char length:数据长度.
返回：0成功，-1失败；
**************************************************************/
static int checkReportData(unsigned char *data, unsigned char length)
{
	if (*data!=0x23 || *(data+2)!=0xE1)
	{
		ERROR("Frame[0] != 0x23 or Frame[2]!=0xE1: %X %X\n\n",*data,data[2]);
		return -1;
	}


	if (*(data+1)!=checkXOR(data+2,length-2))
	{
		ERROR("Full Scene custom data check XOR failed\n\n");
		return -1;
	}

	if (*(data+3)!=0x00)
	{
		ERROR("Custom data have more than 1 package!\n\n");
		return -1;
	}
	return 0;

}


int getFullScreenAttri(unsigned char commandType,unsigned char *data,int datalen,char *attri,char *attriValue )
{
	if (commandType==0xB3)
	{
		sprintf(attri,"%s_%d",ATTRIBUTE_NAME_FULL_SCENE_SCENE_NAME,*data);
		memcpy(attriValue,data+1,datalen-1);
	}
	else if(commandType==0xB4)
	{
		sprintf(attri,"%s_%d",ATTRIBUTE_NAME_FULL_SCENE_SCENE_PHOTO,*data);
		sprintf(attriValue,"%d",*(data+1));
	}

	else if (commandType==0xB5)
	{
		if(0x01==*data)
			sprintf(attri,"%s_%d",ATTRIBUTE_NAME_FULL_SCENE_ENABLE,1);
		else if(0x02==*data)
			sprintf(attri,"%s_%d",ATTRIBUTE_NAME_FULL_SCENE_ENABLE,3);
		else if(0x03==*data)
			sprintf(attri,"%s_%d",ATTRIBUTE_NAME_FULL_SCENE_ENABLE,2);
		else if(0x04==*data)
			sprintf(attri,"%s",ATTRIBUTE_NAME_FULL_SCENE_DLT_GROUP_ENABLE);
		else if(0x05==*data)
			sprintf(attri,"%s",ATTRIBUTE_NAME_FULL_SCENE_SWITCH_GROUP_ENABLE);
		else if(0x06==*data)
			sprintf(attri,"%s",ATTRIBUTE_NAME_FULL_SCENE_CURTAIN_ENABLE);
		else
			return -1;
		sprintf(attriValue,"%d",*(data+1));

	}
	else if (commandType==0xB9)
	{
		sprintf(attri,"%s",ATTRIBUTE_NAME_FULL_SCENE_REFACTORY);
		sprintf(attriValue,"%s","");
	}
	else if (commandType==0xB7 && 0x00==*(data))
	{
		if  (0x01==*(data+1))
			sprintf(attri,"%s","FirmwareVersion");
		else
			return -1;
		sprintf(attriValue,"%s",data+2);

		return 0;
	}
	else if (0x12==commandType)
	{
		return 0;
	}
	else if (0x14==commandType) //设备路数上报
	{
		if(0x05==*(data+1))
			sprintf(attri,"%s",ATTRIBUTE_NAME_FULL_SCENE_SWITCH_GROUP_ENDPOINT);
		else if(0x06==*(data+1))
			sprintf(attri,"%s",ATTRIBUTE_NAME_FULL_SCENE_CURTAIN_ENDPOINT);
		else
			return -1;
		sprintf(attriValue,"%d",*(data+2));
	}
	else if (commandType==0x18) //调光器状态上报
	{
		if(0x01==*(data+2))
			sprintf(attri,"%s_%d",ATTRIBUTE_NAME_FULL_SCENE_DLT_SWITCH,*(data));
		else if (0x02==*(data+2))
			sprintf(attri,"%s_%d",ATTRIBUTE_NAME_FULL_SCENE_DLT_LEVEL,*(data));
		else if (0x03==*(data+2))
			sprintf(attri,"%s_%d",ATTRIBUTE_NAME_FULL_SCENE_DLT_TEMPERATURE,*(data));
		else
			return -1;
		unsigned int value;
		memcpy(&value,data+3,4);
		sprintf(attriValue,"%d",value);
		return 0;
	}
	else
		return -1;

	return 0;
}




/**************************************************************
函数名：full_HandleSceneCustomReport
描述：处理全面屏自定义数据上报解析
参数：@char *data：输入数据首地址;
@ unsigned char length:数据长度.
返回：0成功，-1失败；
**************************************************************/
int full_HandleSceneCustomReport(DevInfo *pCurrentNode,unsigned char *data, unsigned char length)
{
	if (strcmp(pCurrentNode->realModelId,"HY0160")!=0 &&\
			strcmp(pCurrentNode->realModelId,"HY0112")!=0 &&\
			strcmp(pCurrentNode->realModelId,"HY0134")!=0)
		return -1;

	/*判断是否为组播按键josn格式反馈数据上报*/
	if (0!=checkReportData(data,length))
	{
		return -1;
	}

	/*以下为全名屏串口定义数据解析上报*/
	char attriName[32]={0};
	char attriValue[40]={0};
	if (0!=getFullScreenAttri(*(data+4),data+9,*(data+8),attriName,attriValue))
	{
		ERROR("getSceAttri failed\n\n");
		return -1;
	}


	Attribute *pAttri=pCurrentNode->attrHead;
	while(pAttri!=NULL)
	{
		if(strcmp(pAttri->key,attriName)==0)
		{
			memset(pAttri->value,0,ATTRIBUTE_VALUE_MAX_LENGTH);
			sprintf(pAttri->value,"%s",attriValue);
/*
			if (strncmp(pAttri->key,"Enable",6)==0)
			{
				if (*(data+10)==0)
					sprintf(pAttri->value,"%d",0);
				else
					sprintf(pAttri->value,"%d",1);
			}
			else if (strncmp(pAttri->key,"ScePhoto",8)==0)
				sprintf(pAttri->value,"%d",*(data+10));
			else
				strncpy(pAttri->value,(char*)(data+10),*(data+8)-1);
			*/
			ReportAttrEventOnline param;
			memset(&param,0,sizeof(param));
			sprintf(param.deviceId,"%s",pCurrentNode->deviceId);
			sprintf(param.modelId,"%s",pCurrentNode->modelId);
			sprintf(param.key,"%s",pAttri->key);
			sprintf(param.value,"%s",pAttri->value);
			char outBuff[400];
			packageReportMessageToJson(REPORT_TYPE_ATTRIBUTE, (void*)&param, outBuff);
			messageReportFun(outBuff);
			return 0;
		}
		else if (strcmp("FirmwareVersion",attriName)==0)
		{
			snprintf(pCurrentNode->hardwareVersion,32,"%s",data+11);
			updateDevVersionDB(pCurrentNode->deviceId,"hardwareVersion",pCurrentNode->hardwareVersion);
			ReportAttrEventOnline param;
			memset(&param,0,sizeof(param));
			sprintf(param.deviceId,"%s",pCurrentNode->deviceId);
			sprintf(param.modelId,"%s",pCurrentNode->modelId);
			sprintf(param.key,"%s",attriName);
			sprintf(param.value,"%s",pCurrentNode->hardwareVersion);
			char outBuff[400];
			packageReportMessageToJson(REPORT_TYPE_ATTRIBUTE, (void*)&param, outBuff);
			messageReportFun(outBuff);
			return 0;
		}
		else
			pAttri=pAttri->next;
	}
	return -1;
}



int fullscene_GetEnable(char *devicId,int devTyoe)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0xA5;

	outbuff[8]=0x02;
	outbuff[10]=0x02;
	if (devTyoe==1)
		outbuff[9]=0x01;
	else if(devTyoe==2)
		outbuff[9]=0x02;
	else if(devTyoe==3)
		outbuff[9]=0x03;
	else if(devTyoe==4)
		outbuff[9]=0x04;
	else if(devTyoe==5)
		outbuff[9]=0x05;
	else if(devTyoe==6)
		outbuff[9]=0x06;
	else
		outbuff[9]=0xff;
	outbuff[1]=checkXOR((unsigned char*)outbuff+2,9);
	//LOG_HEX(LOG_DEBUG,outbuff,0,11);
	return rex_send_self_defining_data(devicId,0,1,outbuff,11);
}




/**************************************************************
函数名：fullscene_McuGetVersion
描述：查询mcu版本号
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int fullscene_McuGetVersion(char *devicId,int selectVer)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0xA7;

	outbuff[8]=0x034;
	outbuff[9]=0x00;
	if(selectVer==0x0)
		outbuff[10]=0x00;
	else if(selectVer==0x1)
		outbuff[10]=0x01;
	else if(selectVer==0x2)
		outbuff[10]=0x02;
	else if(selectVer==0x3)
		outbuff[10]=0x03;
	else if(selectVer==0x4)
		outbuff[10]=0x04;
	else if(selectVer==0x5)
		outbuff[10]=0x05;
	else
		outbuff[10]=0x00;

	outbuff[1]=checkXOR((unsigned char*)outbuff+2,41);
	return rex_send_self_defining_data(devicId,0,1,outbuff,43);
}


/**************************************************************
函数名：fullscene_GetSwitchEndpoints
描述：获取设备开关/窗帘路数
参数：@char *devicId
@int devType:0x05-开关，0x06-窗帘
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetSwitchEndpoints(char *devicId,int devType)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0x13;

	outbuff[8]=0x03;
	outbuff[9]=0x00;
	outbuff[10]=devType;
	outbuff[11]=0x00;

	outbuff[1]=checkXOR((unsigned char*)outbuff+2,10);
	return rex_send_self_defining_data(devicId,0,1,outbuff,12);
}

/**************************************************************
函数名：fullscene_SetSwitchEndpoints
描述：设置设备开关/窗帘路数
参数：@char *devicId
@int devType:0x05-开关，0x06-窗帘
@int endpoint：开关1-4,窗帘1-2
返回：0成功，-1失败；
**************************************************************/
int fullscene_SetSwitchEndpoints(char *devicId,int devType,int endpoint)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0x13;

	outbuff[8]=0x03;
	outbuff[9]=0x01;
	outbuff[10]=devType;
	outbuff[11]=endpoint;

	outbuff[1]=checkXOR((unsigned char*)outbuff+2,10);
	return rex_send_self_defining_data(devicId,0,1,outbuff,12);
}

/**************************************************************
函数名：fullscene_GetDltStatus
描述：获取DLT状态指令
参数：@char *devicId
@endponit:1-5
@int attriType:0xFF-所有属性，0x01-开关属性，0x02-亮度属性，0x03-色温属性
返回：0成功，-1失败；
**************************************************************/
int fullscene_GetDltStatus(char *devicId,int endponit,int attriType)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0x17;

	outbuff[8]=0x03;
	outbuff[9]=endponit;
	outbuff[10]=attriType;
	outbuff[11]=0x00;

	outbuff[1]=checkXOR((unsigned char*)outbuff+2,10);
	return rex_send_self_defining_data(devicId,0,1,outbuff,12);
}

/**************************************************************
函数名：fullscene_SetEnableSwitch
描述：设置使能开关
参数：@char *devicId
@int devType：0x01空调，0x02地暖，0x03新风，0x04DLT,0X05开关，0x06窗帘：
@int onoff：0x00禁用，0x01启用
返回：0成功，-1失败；
**************************************************************/
int fullscene_SetEnableSwitch(char *devicId,int devType,int onoff)
{
	char outbuff[FRAME_LENGTH]={0};
	memset(&outbuff,0,FRAME_LENGTH);
	outbuff[0]=0x23;
	outbuff[2]=0xE1;
	outbuff[3]=0x00;
	outbuff[4]=0xA5;

	outbuff[8]=0x02;
	outbuff[9]=devType;
	outbuff[10]=onoff;

	outbuff[1]=checkXOR((unsigned char*)outbuff+2,9);
	return rex_send_self_defining_data(devicId,0,1,outbuff,11);
}


/**************************************************************
函数名：full_SceneCtrlCommand
描述：设置全面屏场景名称\图片\空调,新风,地暖
参数：@char *input：输入;
返回：0成功，-1失败；
**************************************************************/
int full_SceneCtrlCommand(KeyValue *currentKeyValue)
{
	int res = -1;
	if(0 == strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_ENABLE, \
			strlen(ATTRIBUTE_NAME_FULL_SCENE_ENABLE)))
	{
		res=fullscene_EnableDevie(currentKeyValue->address,currentKeyValue->key,atoi(currentKeyValue->value));
	}
	else if(0==strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_SCENE_NAME, \
			strlen(ATTRIBUTE_NAME_FULL_SCENE_SCENE_NAME)))
	{
		res=fullscene_SetSceneName(currentKeyValue->address,currentKeyValue->key,currentKeyValue->value);
	}
	else if(0==strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_SCENE_PHOTO, \
			strlen(ATTRIBUTE_NAME_FULL_SCENE_SCENE_PHOTO)))
	{
		res=fullscene_SetScenePhoto(currentKeyValue->address,currentKeyValue->key,atoi(currentKeyValue->value));
	}
	else if(0 == strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_DLT_GROUP_ENABLE, \
			strlen(ATTRIBUTE_NAME_FULL_SCENE_DLT_GROUP_ENABLE)))
	{
		res=fullscene_SetEnableSwitch(currentKeyValue->address,0x04,atoi(currentKeyValue->value));
	}
	else if(0==strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_SWITCH_GROUP_ENABLE,\
			strlen(ATTRIBUTE_NAME_FULL_SCENE_SWITCH_GROUP_ENABLE)))
	{
		res=fullscene_SetEnableSwitch(currentKeyValue->address,0x05,atoi(currentKeyValue->value));
	}
	else if(0==strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_CURTAIN_ENABLE,\
			strlen(ATTRIBUTE_NAME_FULL_SCENE_CURTAIN_ENABLE)))
	{
		res=fullscene_SetEnableSwitch(currentKeyValue->address,0x06,atoi(currentKeyValue->value));
	}
	else if(0==strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_SWITCH_GROUP_ENDPOINT,\
			strlen(ATTRIBUTE_NAME_FULL_SCENE_SWITCH_GROUP_ENDPOINT)))
	{
		res=fullscene_SetSwitchEndpoints(currentKeyValue->address,0x05,atoi(currentKeyValue->value));
	}
	else if(0==strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_CURTAIN_ENDPOINT,\
			strlen(ATTRIBUTE_NAME_FULL_SCENE_CURTAIN_ENDPOINT)))
	{
		res=fullscene_SetSwitchEndpoints(currentKeyValue->address,0x06,atoi(currentKeyValue->value));
	}
	else if(0==strncmp(currentKeyValue->key, ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH,\
			strlen(ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH)))
	{
		if(1==atoi(currentKeyValue->key+strlen(ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH)+1))
			res=sendCommandCtrlSwitchState(currentKeyValue->address, 5, currentKeyValue->value);
		else if (2==atoi(currentKeyValue->key+strlen(ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH)+1))
			res=sendCommandCtrlSwitchState(currentKeyValue->address, 6, currentKeyValue->value);
		else if (3==atoi(currentKeyValue->key+strlen(ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH)+1))
			res=sendCommandCtrlSwitchState(currentKeyValue->address, 7, currentKeyValue->value);
		else if (4==atoi(currentKeyValue->key+strlen(ATTRIBUTE_NAME_FULL_SCENE_LIGHT_SWITCH)+1))
			res=sendCommandCtrlSwitchState(currentKeyValue->address, 8, currentKeyValue->value);
	}
	return res;
}


