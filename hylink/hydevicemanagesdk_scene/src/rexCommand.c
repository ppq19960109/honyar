#include "rex_export_gateway.h"
#include "rex_export_type.h"
#include "cJSON.h"
#include "common.h"
#include "hyprotocol.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "log.h"
#include <pthread.h>
#include <sys/time.h>
#include <sys/stat.h>
/***********************************HSL light************************/
char lightnessValue[4];
char hueValue[4];
char saturationValue[4];
int hueFlag;
int satFlag;

unsigned long sendTimeInterval=200;/*单位毫秒，初始默认为200ms*/
struct timeval timeLast;
unsigned long intervalSec;
unsigned long intervalUSec;
pthread_mutex_t mutex_rex_send_lock;


void initRexSendLock()
{
	intervalSec=sendTimeInterval/1000;
	intervalUSec=(sendTimeInterval%1000)*1000;
	pthread_mutex_init(&mutex_rex_send_lock,NULL);
	memset(&timeLast,0,sizeof(struct timeval));
}

int setSendRexCommandInterval(unsigned long msInterval)
{
	sendTimeInterval=msInterval;
	initRexSendLock();
	return 0;
}


int sendRexCommand(char *deviceId,unsigned char endpointId,unsigned short commandType,char *command)
{
	struct timeval timeNow;
	int res=0;

	pthread_mutex_lock(&mutex_rex_send_lock);
	while(1)
	{
		gettimeofday(&timeNow,NULL);
		if((timeNow.tv_sec-timeLast.tv_sec)>intervalSec|| \
				((timeNow.tv_sec-timeLast.tv_sec)==intervalSec && \
				(timeNow.tv_usec-timeLast.tv_usec)>=intervalUSec)
				)
		{
			break;

		}
		usleep(10*1000);
	}
	gettimeofday(&timeLast,NULL);
	res=rex_send_command_data(deviceId, endpointId, commandType, command);
	pthread_mutex_unlock(&mutex_rex_send_lock);

	return res;
}




int sendCommandGetZigbeeNetworkInfo()
{
	int res;
	res = sendRexCommand("0000000000000000", 1, COMMAND_TYPE_GET_ZIGBEE_NETWORK_INFO, "{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send get zigbee network info command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get zigbee network info command success\n\n");
		return 0;
	}

}


int sendCommandSetupZigbeeNetwork()
{
	int res;
	res = sendRexCommand("0000000000000000", 1, COMMAND_TYPE_SETUP_NETWORK, "{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send setup network command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send setup network command success\n\n");
		return 0;
	}

}

int sendCommandGetGatewayMAC()
{
	int res;

	res = sendRexCommand("0000000000000000", 1, COMMAND_TYPE_GET_COO_SIGNATURE_DATA, "{}");

	if (res != 0)
	{
		WARN("[ZIGBEE] Send the command of getting gateway MAC error!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send the command of getting gateway MAC success!\n\n");
		return 0;
	}
}

int sendCommandGetCooVer()
{
	int res;
	res = sendRexCommand("0000000000000000", 1, COMMAND_TYPE_GET_COO_VERSION, "{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send get coo version command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get coo version command success\n\n");
		return 0;
	}
}

int sendCommandAllowDevJoin(char* time)
{
	int res = 0;
	cJSON *root;
	char *commandString;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Duration", time);
	commandString = cJSON_Print(root);

	res = sendRexCommand("0000000000000000", 1, COMMAND_TYPE_ALLOW_TO_JOIN_NETWORK, commandString);
	cJSON_Delete(root);
	if (commandString != NULL)
	{
		free(commandString);
		commandString = NULL;
	}

	return res;
}

int sendCommandDeleteDevice(char *address)
{
	int res;
	res = sendRexCommand(address, 1, COMMAND_TYPE_CANCELLATION, "{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send command of deleting device error!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send command of deleting device success\n\n");
		return 0;
	}
}

int sendCommandGetDeviceState(char *deviceId, int endpoint)
{
	int res = 0;
	res = sendRexCommand(deviceId, endpoint, COMMAND_TYPE_GET_DEVICE_STATE, "{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send command of get device state error\n\n");
	}
	else
	{
		INFO("[ZIGBEE] Send the command of device %s state success!\n\n",deviceId);
	}
	return res;
}

int sendCommandCtrlSwitchState(char *address, unsigned char endpoint_id, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	//cJSON_AddStringToObject(root, "Type", rexDevType);
	cJSON_AddStringToObject(root, "State", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s, endpoint=%d: %s\n\n", address,endpoint_id,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),endpoint_id, COMMAND_TYPE_LIGHT_SWITCH, jsonString);
	}
	else
		res = sendRexCommand(address, endpoint_id, COMMAND_TYPE_LIGHT_SWITCH, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control switch command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control switch command success\n\n");
		return 0;
	}
}

int sendCommandCtrlLuminance(char *address, unsigned char endpoint_id, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Level", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s, endpoint=%d: %s\n\n", address,endpoint_id,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),endpoint_id, COMMAND_TYPE_LIGHT_LEVEL, jsonString);
	}
	else
		res = sendRexCommand(address, endpoint_id, COMMAND_TYPE_LIGHT_LEVEL, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control luminance command failed\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control luminance command success\n\n");
		return 0;
	}
}

int sendCommandCtrlColorTemperature(char *address, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Temperature", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_LIGHT_TEMPERATURE, jsonString);
	}
	else
		res = sendRexCommand(address, 1, COMMAND_TYPE_LIGHT_TEMPERATURE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control color temperature command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control color temperature command success\n\n");
		return 0;
	}
}

int sendCommandCtrlHUE(char *address, char *state)
{
	int res;
	int hue;
	char realstate[7];
	hue = atoi(state);
	hue = (hue * 254) / 360;
	memset(realstate, 0, 7);
	sprintf(realstate, "%d", hue);
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "HUE", realstate);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_SET_HUE, jsonString);
	}
	else
		res = sendRexCommand(address, 1, COMMAND_TYPE_SET_HUE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control HUE command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control HUE command success\n\n");
		return 0;
	}
}

int sendCommandCtrlSaturation(char *address, char *state)
{
	int res;
	int saturation;
	char realstate[7];
	saturation = atoi(state);
	saturation = (saturation * 254) / 100;
	//printf("[Debug] saturation is %d\n", saturation);
	memset(realstate, 0, 7);
	sprintf(realstate, "%d", saturation);
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Saturation", realstate);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_SET_STAURATION, jsonString);
	}
	else
		res = sendRexCommand(address, 1, COMMAND_TYPE_SET_STAURATION, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control Saturation command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control Saturation command success\n\n");
		return 0;
	}
}

int sendCommandCtrlHSL(char *address, char *level, char *saturation, char *hue)
{

	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Level", level);
	cJSON_AddStringToObject(root, "Saturation", saturation);
	cJSON_AddStringToObject(root, "HUE", hue);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_LIGHT_HSL, jsonString);
	}
	else
		res = sendRexCommand(address, 1, COMMAND_TYPE_LIGHT_HSL, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control HSL command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control HSL command success\n\n");
		return 0;
	}
}

int sendCommandCurtainOperation(char *address, unsigned char endpointId,char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Operate", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_CURTAIN_OPERATION, jsonString);
	}
	else
		res = sendRexCommand(address, endpointId, COMMAND_TYPE_CURTAIN_OPERATION, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control curtain operation command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control curtain operation command success\n\n");
		return 0;
	}
}

int sendCommandSetCurtainPosition(char *address,unsigned char endpointId, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Level", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);


	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_CURTAIN_SET_LEVEL, jsonString);
	}
	else
		res = sendRexCommand(address, endpointId, COMMAND_TYPE_CURTAIN_SET_LEVEL, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control curtain position command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control curtain position command success\n\n");
		return 0;
	}
}

int sendCommandSetCurtainRunMode(char *address,unsigned char endpointId, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "RunMode", state);
	cJSON_AddStringToObject(root, "CalibrateMode", "0");
	cJSON_AddStringToObject(root, "WorkMode", "0");
	cJSON_AddStringToObject(root, "LEDMode", "0");
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);


	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_SET_CURTAIN_MODE, jsonString);
	}
	else
		res = sendRexCommand(address, endpointId, COMMAND_TYPE_SET_CURTAIN_MODE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send Set curtain runMOde command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send Set curtain runMOde command success\n\n");
		return 0;
	}
}




int sendCommandSoundLightAlarm(char *address, const char *alarmType, const char *twinkele, const char *duration)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Alarm", alarmType);
	cJSON_AddStringToObject(root, "Twinkle", twinkele);
	cJSON_AddStringToObject(root, "Duration", duration);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);


	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_SOUND_AND_LIGHT_ALARM, jsonString);
	}
	else
		res = sendRexCommand(address, 1, COMMAND_TYPE_SOUND_AND_LIGHT_ALARM, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control sound and light alarm command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control sound and light alarm command success\n\n");
		return 0;
	}
}

int sendCommandFreshFanMode(char *address, unsigned char endpoint_id, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();

	cJSON_AddStringToObject(root, "FanMode", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s endpoint=%d: %s\n\n", address,endpoint_id,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),endpoint_id, COMMAND_TYPE_SET_FAN_MODE, jsonString);
	}
	else
		res = sendRexCommand(address, endpoint_id, COMMAND_TYPE_SET_FAN_MODE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control fresh fan mode command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control fresh fan mode command success\n\n");
		return 0;
	}
}

int sendCommandSetSocketState(char *address, unsigned char endpoint_id, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "State", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s endpoint=%d: %s\n\n", address,endpoint_id,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),endpoint_id, COMMAND_TYPE_SET_POWER_SUPPLY_STATUS, jsonString);
	}
	else
		res = sendRexCommand(address, endpoint_id, COMMAND_TYPE_SET_POWER_SUPPLY_STATUS, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control socket command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control socket command success\n\n");
		return 0;
	}
}

int sendCommandWorkMode(char *address, unsigned char endpoint_id, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "WorkMode", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s endpoint=%d: %s\n\n", address,endpoint_id,jsonString);


	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),endpoint_id, COMMAND_TYPE_SET_WORK_MODE, jsonString);
	}
	else
		res = sendRexCommand(address, endpoint_id, COMMAND_TYPE_SET_WORK_MODE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}

	if (res != 0)
	{
		WARN("[ZIGBEE] Send control work mode command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control work mode command success\n\n");
		return 0;
	}
}

int sendCommandSetTemperature(char *address, unsigned char endpoint_id, char *state)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	//cJSON_AddStringToObject(root, "Type", rexDevType);
	cJSON_AddStringToObject(root, "Temperature", state);
	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s endpoint=%d: %s\n\n", address,endpoint_id,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),endpoint_id, COMMAND_TYPE_SET_TEMPERATURE, jsonString);
	}
	else
		res = sendRexCommand(address, endpoint_id, COMMAND_TYPE_SET_TEMPERATURE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send set temperature command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send set temperature command success\n\n");
		return 0;
	}
}

//int sendCommandSetSwitchChildLock(char *address, char *state)
int sendCommandWriteCustomParameters(char *address, char *paramName,char *paramValue)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Action", "W");
	cJSON *subItem = cJSON_CreateObject();
	cJSON_AddItemToObject(root, "WriteParameter", subItem);
	cJSON_AddStringToObject(subItem, "Name", paramName);
	cJSON_AddStringToObject(subItem, "Value", paramValue);

	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s: %s\n\n", address,jsonString);

	if (strlen(address)<16)
	{
		res=rex_send_group_command_data((unsigned short)atoi(address),1, COMMAND_TYPE_OPERATE_ATTRIBUTE, jsonString);
	}
	else
		res = sendRexCommand(address, 1, COMMAND_TYPE_OPERATE_ATTRIBUTE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send set switch child lock command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send set switch child lock command success\n\n");
		return 0;
	}
}

/*
int sendCommandSmartLockOperate(char *address, const char * action, const char *value)
{
	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Action", action);
	cJSON_AddStringToObject(root, "Value", value);

	char *jsonString;
	jsonString = cJSON_Print(root);
	printf("[Debug] Send JSON String=%s\n\n", jsonString);

	res = sendRexCommand(address, 1, COMMAND_TYPE_OPERATE_ATTRIBUTE, jsonString);
	cJSON_Delete(root);
	if (res != 0)
	{
		printf("[Debug] Send smart lock command failed!\n\n");
		return -1;
	}
	else
	{
		printf("[Debug] Send smart lock command success\n\n");
		return 0;
	}
}
*/

int sendCommandSetZigbeeNetworkParams(char *channel, char *panId, char* extPanId, char *NetworkKey)
{

	int res;
	cJSON *root;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Channel", channel);
	cJSON_AddStringToObject(root, "PanId", panId);
	cJSON_AddStringToObject(root, "ExtPanId", extPanId);
	cJSON_AddStringToObject(root, "NetworkKey", NetworkKey);

	char *jsonString;
	jsonString = cJSON_Print(root);
	INFO("[ZIGBEE] Set zigbee network params command=%s\n\n", jsonString);

	res = sendRexCommand("0000000000000000", 1, COMMAND_TYPE_SET_ZIGBEE_NETWORK, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send set Zigbee network params command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send set Zigbee network params command failed\n\n");
		return 0;
	}
}

int sendCommandGetDevVersion(char *devId)
{
	int res;
	res = sendRexCommand(devId, 1, COMMAND_TYPE_GET_DEVICE_INFORMATION, "{}");
	if (res!=0)
	{
		WARN("[ZIGBEE] Send get device version command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get device version command success\n\n");
		return 0;
	}
}

int sendCommandCallDeviceResponse(char *dev)
{
	int res;
	res = sendRexCommand(dev, 1, COMMAND_TYPE_CALL_DEVICE_RESPONSE, "{}");
	if (res!=0)
	{
		WARN("[ZIGBEE] Send call device %s response command failed!\n\n",dev);
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send call device %s response command success\n\n",dev);
		return 0;
	}
}

int sendCommandGetCooInfo()
{
	int res;
	res = sendRexCommand("0000000000000000", 1, COMMAND_TYPE_GET_ZIGBEE_NETWORK_INFO, "{}");
	if (res!=0)
	{
		WARN("[ZIGBEE] Send get coo info command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get coo info command success\n\n");
		return 0;
	}
}

int sendCommandGetNeighborInfo(char *devId)
{
	int res;
	res = sendRexCommand(devId, 1, COMMAND_TYPE_GET_NEIGHBOR_INFORMATION, "{}");
	if (res!=0)
	{
		WARN("[ZIGBEE] Send get device neighbor information command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get device neighbor information command success\n\n");
		return 0;
	}
}

int sendCommandGetSubNodeInfo(char *devId)
{
	int res;
	res = sendRexCommand(devId, 1, COMMAND_TYPE_GET_SUBNODE_INFORMATION, "{}");
	if (res!=0)
	{
		WARN("[ZIGBEE] Send get device subnode info command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get device subnode info command success\n\n");
		return 0;
	}
}



int devCtrlSecurityEquipment(KeyValue *currentKeyValue, DevInfo* currentNode)
{
	int res = -1;
	if (strcmp(currentKeyValue->key, "ProtectionStatus") == 0)
	{
		Attribute *nownode;
		nownode = currentNode->attrHead;
		while (nownode != NULL)
		{
			if (strcmp(nownode->key, "ProtectionStatus") == 0)
			{
				sprintf(nownode->value, "%s", currentKeyValue->value);

				return 0;
			}
			else
				nownode = nownode->next;
		}
	}
	return res;
}

int sendCommandLedEnable(char *address,unsigned char endpoint,char *value)
{
	int res;
	cJSON *root,*writePam;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Action", "W");
	cJSON_AddItemToObject(root,"WriteParameter",writePam=cJSON_CreateObject());
	cJSON_AddStringToObject(writePam, "Name", "LedEnable");
	cJSON_AddStringToObject(writePam, "Value", value);

	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s, endpoint=%d: %s\n\n", address,endpoint,jsonString);

	res = sendRexCommand(address, endpoint, COMMAND_TYPE_OPERATE_ATTRIBUTE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control switch led enable=%s command failed!\n\n",value);
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control switch led enable=%s command success\n\n",value);
		return 0;
	}
}

int sendCommandPowerOffProtection(char *address,unsigned char endpoint,char *value)
{
	int res;
	cJSON *root,*writePam;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Action", "W");
	cJSON_AddItemToObject(root,"WriteParameter",writePam=cJSON_CreateObject());
	cJSON_AddStringToObject(writePam, "Name", "PowerOffProtection");
	cJSON_AddStringToObject(writePam, "Value", value);

	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s, endpoint=%d: %s\n\n", address,endpoint,jsonString);

	res = sendRexCommand(address, endpoint, COMMAND_TYPE_OPERATE_ATTRIBUTE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control power off protection led enable=%s command failed!\n\n",value);
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control power off protection led enable=%s command success\n\n",value);
		return 0;
	}
}

int sendCommandKeyMode(char *address,unsigned char endpoint,char *value)
{
	int res;
	cJSON *root,*writePam;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Action", "W");
	cJSON_AddItemToObject(root,"WriteParameter",writePam=cJSON_CreateObject());
	cJSON_AddStringToObject(writePam, "Name", ATTRIBUTE_NAME_KEYMODE);
	cJSON_AddStringToObject(writePam, "Value", value);

	char *jsonString;
	jsonString = cJSON_Print(root);
	DEBUG("[ZIGBEE] Send rex command to address=%s, endpoint=%d: %s\n\n", address,endpoint,jsonString);

	res = sendRexCommand(address, endpoint, COMMAND_TYPE_OPERATE_ATTRIBUTE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send control key Mode=%s command failed!\n\n",value);
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send control key Mode=%s command success\n\n",value);
		return 0;
	}
}

int devCtrlSwitchPannel123(KeyValue *currentKeyValue)
{
	int res = -1;
	unsigned char endpoint = 1;
	if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH) == 0 \
		|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_1) == 0 \
		|| strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_1) == 0 \
		|| strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH) == 0 \
		|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LED_ENABLE) == 0 \
		||strcmp(currentKeyValue->key, ATTRIBUTE_NAME_POWER_OFF_PROTECTION) == 0\
		||strcmp(currentKeyValue->key, ATTRIBUTE_NAME_KEYMODE) == 0 \
		||strcmp(currentKeyValue->key, ATTRIBUTE_NAME_DCDELAY) == 0\
		||strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LOCAL_CONFIG) == 0)
		endpoint = 1;
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_2) == 0 \
		|| strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_2) == 0)
		endpoint = 2;
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_3) == 0 \
		|| strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_3) == 0)
		endpoint = 3;
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_4) == 0 \
		|| strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_4) == 0)
		endpoint = 4;
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_ALL) == 0)
		endpoint = 255;
	else
		return -1;

	if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LED_ENABLE) == 0 )
	{
		res= sendCommandLedEnable(currentKeyValue->address,endpoint,currentKeyValue->value);
		return res;
	}
	if ( strcmp(currentKeyValue->key, ATTRIBUTE_NAME_POWER_OFF_PROTECTION) == 0)
	{
		res=sendCommandPowerOffProtection(currentKeyValue->address,endpoint,currentKeyValue->value);
		return res;
	}
	if( strcmp(currentKeyValue->key, ATTRIBUTE_NAME_KEYMODE) == 0)
	{
		sendCommandKeyMode(currentKeyValue->address,endpoint,currentKeyValue->value);
		return res;
	}

	if ( strcmp(currentKeyValue->key, ATTRIBUTE_NAME_DCDELAY) == 0)
	{
		sendCommandWriteCustomParameters(currentKeyValue->address,ATTRIBUTE_NAME_DCDELAY,currentKeyValue->value);
		return res;
	}
	if ( strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LOCAL_CONFIG) == 0)
	{
		char tmp[2]={0};
		if (strcmp(currentKeyValue->value,"0")==0)
			sprintf(tmp,"%d",0);
		else
			sprintf(tmp,"%d",16);
		sendCommandWriteCustomParameters(currentKeyValue->address,ATTRIBUTE_NAME_LOCAL_CONFIG,tmp);
		return res;
	}

	if (strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_1) == 0 \
		|| strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_2) == 0 \
		|| strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_3) == 0 \
		|| strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH) == 0)
	{
		sprintf(currentKeyValue->value, "%d", 2);
	}

	res = sendCommandCtrlSwitchState(currentKeyValue->address, endpoint, currentKeyValue->value);
	return res;
}


int devCtrlSwitchDLT(KeyValue *currentKeyValue)
{
	int res = -1;
	if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH) == 0)
	{
		res = sendCommandCtrlSwitchState(currentKeyValue->address, 1, currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_COLOR_TEMPERATURE) == 0)
	{
		res = sendCommandCtrlColorTemperature(currentKeyValue->address, currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LUMINANCE) == 0)
	{
		res = sendCommandCtrlLuminance(currentKeyValue->address, 1, currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH) == 0)
	{
		sprintf(currentKeyValue->value, "%d", 2);
		res = sendCommandCtrlSwitchState(currentKeyValue->address, 1, currentKeyValue->value);
	}
	return res;
}


int devCtrlLightHSL(KeyValue *currentKeyValue)
{
	int res = -1;
	if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH) == 0)
	{
		res = sendCommandCtrlSwitchState(currentKeyValue->address, 1, currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LUMINANCE) == 0)
	{
		res = sendCommandCtrlLuminance(currentKeyValue->address, 1, currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_COLOR_TEMPERATURE) == 0)
	{
		res = sendCommandCtrlColorTemperature(currentKeyValue->address, currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_HUE) == 0)
	{

		res=sendCommandCtrlHUE(currentKeyValue->address, currentKeyValue->value);
		/*
		int hue;
		hue = atoi(currentKeyValue->value);
		hue = (hue * 254) / 360;
		sprintf(hueValue, "%d", hue);
		hueFlag = 1;
		if (hueFlag == 1 && satFlag == 1)
		{
			res = sendCommandCtrlHSL(currentKeyValue->address, lightnessValue, saturationValue, hueValue);
			hueFlag = 0;
			satFlag = 0;
		}
		else
			res = 0;
		*/
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SATURATION) == 0)
	{
		res=sendCommandCtrlSaturation(currentKeyValue->address, currentKeyValue->value);
		/*
		int saturation;
		saturation = atoi(currentKeyValue->value);
		saturation = (saturation * 254) / 100;
		//printf("[Debug] saturation is %d\n", saturation);
		sprintf(saturationValue, "%d", saturation);
		satFlag = 1;
		if (hueFlag == 1 && satFlag == 1)
		{
			res = sendCommandCtrlHSL(currentKeyValue->address, lightnessValue, saturationValue, hueValue);
			hueFlag = 0;
			satFlag = 0;
		}
		else
			res = 0;
		*/
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LIGHTNESS) == 0)
	{
		sprintf(lightnessValue, "%s", currentKeyValue->value);
		res = sendCommandCtrlLuminance(currentKeyValue->address, 1, currentKeyValue->value);
	}
	return res;
}

/********************************************************
函数名： sendCommandGetMcuVersion
描  述：获取设备mcu版本；
参  数：@char *address:设备地址或mac
返  回：0成功，-1失败
*********************************************************/
int sendCommandGetMcuVersion(char *address)
{
	int res;
	cJSON *root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Action","R");
	cJSON *param;
	cJSON_AddItemToObject(root,"ReadParameter",param=cJSON_CreateObject());
	cJSON *array;
	array=cJSON_CreateArray();
	cJSON_AddItemToArray(array,cJSON_CreateString("MCUVersion"));
	cJSON_AddItemToObject(param,"Names",array);

	char *tmpStr=NULL;
	tmpStr=cJSON_PrintUnformatted(root);

	res=sendRexCommand(address, 1, 0x01EF,tmpStr);
	cJSON_Delete(root);
	free(tmpStr);

	if (res != 0)
	{
		WARN("[ZIGBEE] Send get deviceId=%s MCU Version command failed!\n\n",address);
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get deviceId=%s MCU Version command success\n\n",address);
		return 0;
	}
}

/********************************************************
函数名： sendCommandGetCurtainRainSensorParam
描  述：获取窗帘电机风雨传感器属性；
参  数：@char *address:设备地址或mac;

返  回：0成功，-1失败
*********************************************************/
int sendCommandGetCurtainRainSensorParam(char *address)
{
	int res;
	cJSON *root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Action","R");
	cJSON *param;
	cJSON_AddItemToObject(root,"ReadParameter",param=cJSON_CreateObject());
	cJSON *array;
	array=cJSON_CreateArray();
	cJSON_AddItemToArray(array,cJSON_CreateString("WindRainSensor"));
	cJSON_AddItemToObject(param,"Names",array);

	char *tmpStr=NULL;
	tmpStr=cJSON_Print(root);

	res=sendRexCommand(address, 1, 0x01EF,tmpStr);
	cJSON_Delete(root);
	free(tmpStr);

	if (res != 0)
	{
		WARN("[ZIGBEE] Send get deviceId=%s WindRainSensor param command failed!\n\n",address);
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get deviceId=%s WindRainSensor param command success\n\n",address);
		return 0;
	}
}

/********************************************************
函数名： sendCommandSetCurtainRainSensorParam
描  述：获取窗帘电机风雨传感器属性；
参  数：@char *address:设备地址或mac;
       @char *value: 写入值 1使能风雨传感器，0关闭风雨传感器
返  回：0成功，-1失败
*********************************************************/
int sendCommandSetCurtainRainSensorParam(char *address,char *value)
{
	int res;
	cJSON *root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Action","W");
	cJSON *param;
	cJSON_AddItemToObject(root,"WriteParameter",param=cJSON_CreateObject());

	cJSON_AddStringToObject(param,"Name","WindRainSensor");
	cJSON_AddStringToObject(param,"Value",value);

	char *tmpStr;
	tmpStr=cJSON_Print(root);

	res=sendRexCommand(address, 1, 0x01EF,tmpStr);
	cJSON_Delete(root);
	free(tmpStr);

	if (res != 0)
	{
		WARN("[ZIGBEE] Send Set deviceId=%s WindRainSensor param command failed!\n\n",address);
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send Set deviceId=%s WindRainSensor param command success\n\n",address);
		return 0;
	}
}


int sendCommandSetCurtainAtomizationParam(char *address,unsigned char endpoint,char *value)
{
	int res;
	cJSON *root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Action","W");
	cJSON *param;
	cJSON_AddItemToObject(root,"WriteParameter",param=cJSON_CreateObject());

	cJSON_AddStringToObject(param,"Name",ATTRIBUTE_NAME_ATOMIZATION);
	cJSON_AddStringToObject(param,"Value",value);

	char *tmpStr;
	tmpStr=cJSON_Print(root);

	res=sendRexCommand(address, endpoint, 0x01EF,tmpStr);
	cJSON_Delete(root);
	free(tmpStr);

	if (res != 0)
	{
		WARN("[ZIGBEE] Send Set deviceId=%s endpoint=%d Atomization param command failed!\n\n",address,endpoint);
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send Set deviceId=%s endpoint=%d Atomization param command success\n\n",address,endpoint);
		return 0;
	}
}

int devCtrlElectricCurtain(KeyValue *currentKeyValue)
{
	int res = -1;
	unsigned char endpointId=1;
	if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_OPERATION) == 0 \
		|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_ACTUATOR_OPERATION) == 0 \
		|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_OPERATION_1) == 0 \
		|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_OPERATION_2) == 0)
	{
		int value = atoi(currentKeyValue->value);
		if (0 == value)
			sprintf(currentKeyValue->value, "%d", 1);
		else if (1 == value)
			sprintf(currentKeyValue->value, "%d", 0);

		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_OPERATION_2) == 0)
			endpointId=2;
		res = sendCommandCurtainOperation(currentKeyValue->address, endpointId,currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_MODE) == 0)/*�����������ģʽ��0������1��ת*/
	{
		int value = atoi(currentKeyValue->value);
		if (0==value)
			sprintf(currentKeyValue->value, "%d", 1);
		else
			sprintf(currentKeyValue->value, "%d", 0);

		res = sendCommandCurtainOperation(currentKeyValue->address, endpointId,currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_RUN_MODE) == 0 \
			|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_RUN_MODE_1) == 0 \
			|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_RUN_MODE_2) == 0)
	{
		int value = atoi(currentKeyValue->value);
		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_RUN_MODE_2) == 0)
			endpointId=2;

		sprintf(currentKeyValue->value, "%d", value);

		res = sendCommandSetCurtainRunMode(currentKeyValue->address,endpointId, currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WINRAINSENSOR) == 0)
	{
		res = sendCommandSetCurtainRainSensorParam(currentKeyValue->address,currentKeyValue->value);
	}
	else if (strncmp(currentKeyValue->key, ATTRIBUTE_NAME_ATOMIZATION,11) == 0)
	{
		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_ATOMIZATION_1) == 0)
			endpointId=1;
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_ATOMIZATION_2) == 0)
			endpointId=2;
		res = sendCommandSetCurtainAtomizationParam(currentKeyValue->address,endpointId,currentKeyValue->value);
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_POSITION) == 0 \
		|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_ACTUATOR_POSITION) == 0 \
		|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_POSITION_1) == 0 \
		|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_POSITION_2) == 0)
	{
		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURTAIN_POSITION_2) == 0)
			endpointId=2;
		res = sendCommandSetCurtainPosition(currentKeyValue->address,endpointId, currentKeyValue->value);
	}
	else if(strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LED_ENABLE) == 0)
	{
		res=sendCommandWriteCustomParameters(currentKeyValue->address, currentKeyValue->key,currentKeyValue->value);
	}
	return res;
}

int devCtrlSecurityAlarm(KeyValue *currentKeyValue)
{
	int res = -1;
	if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SIREN_WARNING_MODE) == 0)
	{
		int alarmType = atoi(currentKeyValue->value);
		switch (alarmType)
		{
		case 0:/*ֹͣ����*/
		{
			res = sendCommandSoundLightAlarm(currentKeyValue->address, "0", "0", "0");
			break;
		}
		case 1:/*ϵͳ����*/
		{
			res = sendCommandSoundLightAlarm(currentKeyValue->address, "9", "0", "2");
			break;
		}
		case 2:/*ϵͳ����*/
		{
			res = sendCommandSoundLightAlarm(currentKeyValue->address, "8", "0", "2");
			break;
		}
		case 3:/*��������*/
		{
			res = sendCommandSoundLightAlarm(currentKeyValue->address, "1", "1", "240");
			break;
		}
		case 4:/*��*/
		{
			res = sendCommandSoundLightAlarm(currentKeyValue->address, "2", "1", "240");
			break;
		}
		case 5:/*�����������*/
		{
			res = sendCommandSoundLightAlarm(currentKeyValue->address, "3", "1", "240");
			break;
		}
		case 6:/*110����*/
		{
			res = sendCommandSoundLightAlarm(currentKeyValue->address, "3", "1", "240");
			break;
		}
		default:
			break;
		}
		//if (strcmp(currentKeyValue->value, "0") == 0)
		//	res = sendCommandCtrlSwitchState(currentKeyValue->address, 1, currentKeyValue->value);
	}
	return res;
}

int devCtrlSocket12(KeyValue *currentKeyValue)
{
	int res = -1;
	unsigned char endpoint = 1;
	if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH) == 0  ||\
		strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_1) == 0 ||\
		 strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITCH_MULTIPLE_1) == 0 ||\
		 strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_1) == 0 )
	{
		endpoint = 1;
	}
	else if(strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITCH_CHILD_LOCK)== 0 ||\
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_POWER_CONSUMPTIONCLEAR) == 0 ||\
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_POWER_OFF_PROTECTION) == 0 || \
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_LED_ENABLE) == 0 || \
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_VOLTAGE_COFFICIENT) == 0 ||\
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURRENT_COFFICIENT) == 0 ||\
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_POWER_COFFICIENT) == 0 ||\
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_POWER_WASTER_COFFICIENT) == 0 ||\
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_OVER_LOAD_PROTECT_ENABLE) == 0 ||\
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_VOLTAGE_OVER_LOAD) == 0 )
	{

		res=sendCommandWriteCustomParameters(currentKeyValue->address,currentKeyValue->key,currentKeyValue->value);
		return res;
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_CURRENT_OVER_LOAD) == 0)
	{
		char tmp[10]={0};
		sprintf(tmp,"%.0f",1000*atof(currentKeyValue->value));
		res=sendCommandWriteCustomParameters(currentKeyValue->address,currentKeyValue->key,tmp);
		return res;
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITCH_MULTIPLE_2) == 0 ||\
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_2) == 0 ||\
			strcmp(currentKeyValue->key, INSTRUCTION_NAME_TUMBLER_SWITCH_2) == 0)
	{
		endpoint = 2;
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITCH_MULTIPLE_ALL) == 0)
		endpoint = 255;
	else
		return -1;
	res = sendCommandSetSocketState(currentKeyValue->address, endpoint, currentKeyValue->value);

	return res;
}

int devCtrlFreshAirSystem(KeyValue *currentKeyValue)
{
	int res = -1;
	if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH) == 0)
	{
		if (strcmp(currentKeyValue->value, "1") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, 1, "4");
		}
		else if (strcmp(currentKeyValue->value, "0") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, 1, "0");
		}
	}
	else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED) == 0 || \
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_1) == 0 || \
			strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_2) == 0)
	{
		unsigned char endpoint=1;
		if( strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_2) == 0)
			endpoint=2;
		if (strcmp(currentKeyValue->value, "2") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, endpoint, "1");
		}
		else if (strcmp(currentKeyValue->value, "3") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, endpoint, "2");
		}
		else if (strcmp(currentKeyValue->value, "4") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, endpoint, "3");
		}
		else if (strcmp(currentKeyValue->value, "10") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, endpoint, "10");
		}
	}
	return res;
}


int devCtrlAirConditioner(KeyValue *currentKeyValue)
{
	int res = -1;
	unsigned char endpoint = 1;
	if (strncmp(currentKeyValue->key, "Switch", 6) == 0)
	{
		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH) == 0 \
			|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_1) == 0)
		{
			endpoint = 1;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_2) == 0)
		{
			endpoint = 2;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_3) == 0)
		{
			endpoint = 3;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_4) == 0)
		{
			endpoint = 4;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_5) == 0)
		{
			endpoint = 5;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_6) == 0)
		{
			endpoint = 6;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_7) == 0)
		{
			endpoint = 7;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_8) == 0)
		{
			endpoint = 8;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_9) == 0)
		{
			endpoint = 9;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_10) == 0)
		{
			endpoint = 10;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_11) == 0)
		{
			endpoint = 11;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_12) == 0)
		{
			endpoint = 12;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_13) == 0)
		{
			endpoint = 13;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_14) == 0)
		{
			endpoint = 14;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_15) == 0)
		{
			endpoint = 15;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_SWITHCH_16) == 0)
		{
			endpoint = 16;
		}
		else
			return res;

		//char devType[5];
		//sprintf(devType, "%04X", currentNode->subDevType);

		if (strcmp(currentKeyValue->value, "0") == 0)
		{
			res = sendCommandWorkMode(currentKeyValue->address, endpoint, "0");
		}
		else
		{
			res = sendCommandWorkMode(currentKeyValue->address, endpoint, "10");
		}

	}
	else if (strncmp(currentKeyValue->key, "WindSpeed", 9) == 0)
	{
		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED) == 0 \
			|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_1) == 0)
		{
			endpoint = 1;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_2) == 0)
		{
			endpoint = 2;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_3) == 0)
		{
			endpoint = 3;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_4) == 0)
		{
			endpoint = 4;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_5) == 0)
		{
			endpoint = 5;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_6) == 0)
		{
			endpoint = 6;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_7) == 0)
		{
			endpoint = 7;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_8) == 0)
		{
			endpoint = 8;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_9) == 0)
		{
			endpoint = 9;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_10) == 0)
		{
			endpoint = 10;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_11) == 0)
		{
			endpoint = 11;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_12) == 0)
		{
			endpoint = 12;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_13) == 0)
		{
			endpoint = 13;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_14) == 0)
		{
			endpoint = 14;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_15) == 0)
		{
			endpoint = 15;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_WIND_SPEED_16) == 0)
		{
			endpoint = 16;
		}
		else
			return res;

		//char devType[5];
		//sprintf(devType, "%04X", currentNode->subDevType);
		if (strcmp(currentKeyValue->value, "2") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, endpoint, "1");
		}
		else if (strcmp(currentKeyValue->value, "3") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, endpoint, "2");
		}
		else if (strcmp(currentKeyValue->value, "4") == 0)
		{
			res = sendCommandFreshFanMode(currentKeyValue->address, endpoint, "3");
		}
		else
			return -1;
	}
	else if (strncmp(currentKeyValue->key, "WorkMode", 8) == 0)
	{

		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE) == 0 \
			|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_1) == 0)
		{
			endpoint = 1;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_2) == 0)
		{
			endpoint = 2;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_3) == 0)
		{
			endpoint = 3;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_4) == 0)
		{
			endpoint = 4;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_5) == 0)
		{
			endpoint = 5;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_6) == 0)
		{
			endpoint = 6;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_7) == 0)
		{
			endpoint = 7;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_8) == 0)
		{
			endpoint = 8;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_9) == 0)
		{
			endpoint = 9;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_10) == 0)
		{
			endpoint = 10;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_11) == 0)
		{
			endpoint = 11;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_12) == 0)
		{
			endpoint = 12;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_13) == 0)
		{
			endpoint = 13;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_14) == 0)
		{
			endpoint = 14;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_15) == 0)
		{
			endpoint = 15;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_MODE_16) == 0)
		{
			endpoint = 16;
		}

		//char devType[5];
		//sprintf(devType, "%04X", currentNode->subDevType);
		if (strcmp(currentKeyValue->value, "0") == 0)
		{
			res = sendCommandWorkMode(currentKeyValue->address, endpoint, "10");
		}
		else if (strcmp(currentKeyValue->value, "1") == 0)
		{
			res = sendCommandWorkMode(currentKeyValue->address, endpoint, "3");
		}
		else if (strcmp(currentKeyValue->value, "2") == 0)
		{
			res = sendCommandWorkMode(currentKeyValue->address, endpoint, "4");
		}
		else if (strcmp(currentKeyValue->value, "3") == 0)
		{
			res = sendCommandWorkMode(currentKeyValue->address, endpoint, "7");
		}
		else if (strcmp(currentKeyValue->value, "4") == 0)
		{
			res = sendCommandWorkMode(currentKeyValue->address, endpoint, "8");
		}
		else
			return -1;

	}
	else if (strncmp(currentKeyValue->key, "Target", 6) == 0 || (strncmp(currentKeyValue->key, "Temper", 6) == 0))
	{
		if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TEMPERATUR) == 0 \
			|| strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_1) == 0)
		{
			endpoint = 1;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_2) == 0)
		{
			endpoint = 2;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_3) == 0)
		{
			endpoint = 3;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_4) == 0)
		{
			endpoint = 4;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_5) == 0)
		{
			endpoint = 5;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_6) == 0)
		{
			endpoint = 6;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_7) == 0)
		{
			endpoint = 7;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_8) == 0)
		{
			endpoint = 8;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_9) == 0)
		{
			endpoint = 9;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_10) == 0)
		{
			endpoint = 10;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_11) == 0)
		{
			endpoint = 11;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_12) == 0)
		{
			endpoint = 12;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_13) == 0)
		{
			endpoint = 13;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_14) == 0)
		{
			endpoint = 14;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_15) == 0)
		{
			endpoint = 15;
		}
		else if (strcmp(currentKeyValue->key, ATTRIBUTE_NAME_TARGET_TEMPERATURE_16) == 0)
		{
			endpoint = 16;
		}
		else
			return -1;

		//char devType[5];
		//sprintf(devType, "%04X", currentNode->subDevType);

		res = sendCommandSetTemperature(currentKeyValue->address, endpoint, currentKeyValue->value);
	}
	return res;
}


int sendBindOperationZHA(char *address,char *srcEndpoint,char *key,char *function,char *dstAddr,char *dstEndpoint)
{
	int res=0;
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Action",key);
	cJSON_AddStringToObject(root,"Function",function);
	cJSON_AddStringToObject(root,"DstAddrMode","3");
	cJSON_AddStringToObject(root,"DstAddr",dstAddr);
	cJSON_AddStringToObject(root,"DstEndpointId",dstEndpoint);
	char *jsonString=cJSON_Print(root);

	res = sendRexCommand(address, atoi(srcEndpoint), COMMAND_TYPE_BIND_ZHA, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send auxiliary switch bind operation command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send auxiliary switch bind operation command success\n\n");
		return 0;
	}
}

int sendBindQueryZHA(char *address)
{
	int res=0;
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Action","Q");

	char *jsonString=cJSON_Print(root);

	res = sendRexCommand(address, 1, COMMAND_TYPE_BIND_ZHA, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send auxiliary switch bind operation command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send auxiliary switch bind operation command success\n\n");
		return 0;
	}
}


int sendGroupOperation(char *address,char *endpoint,char *action,char *group)
{
	int res=0;
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Action",action);
	if (group!=NULL)
		cJSON_AddStringToObject(root,"GroupId",group);
	else
		cJSON_AddStringToObject(root,"GroupId","");

	char *jsonString=cJSON_Print(root);

	res = sendRexCommand(address, atoi(endpoint), COMMAND_TYPE_MANAGE_GROUP, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}
	if (res != 0)
	{
		WARN("[ZIGBEE] Send manage group command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send manage group command success\n\n");
		return 0;
	}
}


int sendSceneOperation(char *address,char *endpoint,char *action,char *groupId,char *sceneId,\
		char * sceneName,char *buttonCode)
{
	int res=0;
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"Action",action);
	cJSON_AddStringToObject(root,"GroupId",groupId);
	if (sceneId!=NULL)
			cJSON_AddStringToObject(root,"SceneId",sceneId);
	if (sceneName!=NULL)
	{
		cJSON_AddStringToObject(root,"SceneName",sceneName);
	}
	if (buttonCode!=NULL)
		cJSON_AddStringToObject(root,"ButtonCode",buttonCode);

	char *jsonString=cJSON_Print(root);

	if (strncmp(action,"E",1)==0)
		res = sendRexCommand(address, 255, COMMAND_TYPE_MANAGE_SCENE, jsonString);
	else
		res = sendRexCommand(address, atoi(endpoint), COMMAND_TYPE_MANAGE_SCENE, jsonString);
	cJSON_Delete(root);
	if (jsonString!=NULL)
	{
		free(jsonString);
		jsonString=NULL;
	}

	if (res != 0)
	{
		WARN("[ZIGBEE] Send scene manage command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send scene manage command success\n\n");
		return 0;
	}

}


int sendCommandCloseFastJoin()
{
	int res;
	res=sendRexCommand("0000000000000000", 1, COMMAND_TYPE_CLOSE_FAST_JOIN, "{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send close fast join command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send close fast join command success\n\n");
		return 0;
	}
}


int sendCommandReBuildNetwark()
{
	int res;
	res=sendRexCommand("0000000000000000", 1, 0x01F0, "{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send rebuild network command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send rebuild network command success\n\n");
		return 0;
	}
}


/********************************************************
函数名： sendCommandOpenFastJoin
描  述：下发快速入网功能；
参  数：@char *command：下发地址数组JSON
返  回：0成功，-1失败
*********************************************************/
int sendCommandOpenFastJoin(char *command)
{
	int res;
	res=sendRexCommand("0000000000000000", 1, COMMAND_TYPE_OPEN_FAST_JOIN, command);
	if (res != 0)
	{
		WARN("[ZIGBEE] Send open fast join network command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send open fast join network command success\n\n");
		return 0;
	}
}

/********************************************************
函数名： sendCommandClearGatewayNetInfo
描  述：发送清除网关网络参数信息；
参  数：
返  回：0成功，-1失败
*********************************************************/
int sendCommandClearGatewayNetInfo()
{
	int res;
	res=sendRexCommand("0000000000000000", 1, 0x01F1, "{\"Command\":\"AT+LEAVE\"}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send clear gateway network info command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send clear gateway network info command success\n\n");
		return 0;
	}
}


/********************************************************
函数名： sendCommandBuildNetwork
描  述：建立网络；
参  数：
返  回：0成功，-1失败
*********************************************************/
int sendCommandBuildNetwork()
{
	int res;
	res=sendRexCommand("0000000000000000", 1, 0x01F1, "{\"Command\":\"AT+FORM=02\"}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send build network command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send build network info command success\n\n");
		return 0;
	}
}


/********************************************************
函数名： sendCommandSetSignature
描  述：设置网关签名信息；
参  数：@const char *type:签名类型
			  0：擦出所有签名;
              1：写入ProductModel;
              2:写入ProductKey;
              3:写入ProductSecret;
              4:写入ProductName;
              5:写入DeviceSecret;
              6:写入DeviceKey;
      @char *data: 数据
返  回：0成功，-1失败
*********************************************************/
int sendCommandSetSignature(const char *type,char *data)
{
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root,"DataType",type);
	int len=strlen(data);
	if (strcmp(type,"ProductModel")==0)
	{
		if(len>80)
		{
			ERROR("[ZIGBEE] ProductModel length > 80 characters\n\n");
			return -1;
		}
	}
	else if (strcmp(type,"ProductKey")==0)
	{
		if(len>20)
		{
			ERROR("[ZIGBEE] ProductKey length > 20 characters\n\n");
			return -1;
		}
	}
	else if (strcmp(type,"ProductSecret")==0)
	{
		if(len>40)
		{
			ERROR("[ZIGBEE] ProductSecret length > 40 characters\n\n");
			return -1;
		}
	}
	else if (strcmp(type,"ProductName")==0)
	{
		if(len>32)
		{
			ERROR("[ZIGBEE] ProductName length > 32 characters\n\n");
			return -1;
		}
	}
	else if (strcmp(type,"DeviceSecret")==0)
	{
		if(len>32)
		{
			ERROR("[ZIGBEE] DeviceSecret length > 32 characters\n\n");
			return -1;
		}
	}
	else if (strcmp(type,"DeviceKey")==0)
	{
		if(len>20)
		{
			ERROR("[ZIGBEE] DeviceKey length > 20 characters\n\n");
			return -1;
		}
	}

	cJSON_AddStringToObject(root,"Data",data);

	char *tmp;
	tmp=cJSON_Print(root);


	int res;
	res=sendRexCommand("0000000000000000", 1, 0x01FB, tmp);
	free(tmp);
	cJSON_Delete(root);
	if (res != 0)
	{
		WARN("[ZIGBEE] Send set gateway signature command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send set gateway signature command success\n\n");
		return 0;
	}
}


/********************************************************
函数名： sendCommandGetSignature
描  述：获取网关签名信息；
参  数：
返  回：0成功，-1失败
*********************************************************/
int sendCommandGetSignature()
{
	int res;
	res=sendRexCommand("0000000000000000", 1, 0x0111,"{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send get gateway signature command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get gateway signature command success\n\n");
		return 0;
	}
}

/********************************************************
函数名： sendCommandGetRexSDKVersion
描  述：获取rexSDK版本号信息指令；
参  数：
返  回：0成功，-1失败
*********************************************************/
int sendCommandGetRexSDKVersion()
{
	int res;
	res=sendRexCommand("0000000000000000", 1, 0x010F,"{}");
	if (res != 0)
	{
		WARN("[ZIGBEE] Send get rexgatewaysdk version command failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send get rexgatewaysdk version command success\n\n");
		return 0;
	}
}

int sendCommandReadCustomParameters(char *deviceID,unsigned char endpoint,char *paramName1,char *paramName2,char *paramName3,char *paramName4)
{
	cJSON *root;
	cJSON *readParam;
	cJSON *name;
	char *commandString;
	int res;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Action", "R");
	cJSON_AddItemToObject(root,"ReadParameter",readParam=cJSON_CreateObject());
	cJSON_AddItemToObject(readParam,"Names",name=cJSON_CreateArray());
	cJSON_AddItemToArray(name,cJSON_CreateString(paramName1));
	if (paramName2!=NULL)
		cJSON_AddItemToArray(name,cJSON_CreateString(paramName2));
	if (paramName3!=NULL)
		cJSON_AddItemToArray(name,cJSON_CreateString(paramName3));
	if (paramName4!=NULL)
		cJSON_AddItemToArray(name,cJSON_CreateString(paramName4));

	commandString = cJSON_Print(root);
	res=sendRexCommand(deviceID, endpoint, 0x01EF,commandString);

	if (res != 0)
	{
		WARN("[ZIGBEE] Send query custom parameter command failed!\n\n");
		res= -1;
	}
	else
	{
		INFO("[ZIGBEE] Send query custom parameter command success:>>%s\n\n",commandString);
		res= 0;
	}
	free(commandString);
	cJSON_Delete(root);
	return res;
}

int sendCommandSetJoinNetWorkMode(int mode)
{
	cJSON *root;
	root=cJSON_CreateObject();

	char tmp[2]={0};
	sprintf(tmp,"%d",mode);
	cJSON_AddStringToObject(root, "JoinMode", tmp);
	char *strJson=cJSON_PrintUnformatted(root);

	int res;
	res=sendRexCommand("0000000000000000", 1, COMMAND_TYPE_SET_JOIN_MODE, strJson);
	free(strJson);
	cJSON_Delete(root);
	if (res != 0)
	{
		WARN("[ZIGBEE] Send set fast join network mode failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send set fast join network mode success\n\n");
		return 0;
	}
}

int sendCommandSetAtHoldRPTOD()
{
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Command", "AT+HOLDRPTOD");
	char *strJson=cJSON_PrintUnformatted(root);

	int res;
	res=sendRexCommand("0000000000000000", 1, COMMAND_TYPE_DEBUG_COMMAND, strJson);
	free(strJson);
	cJSON_Delete(root);
	if (res != 0)
	{
		WARN("[ZIGBEE] Send AT+HOLDRPTOD failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send AT+HOLDRPTOD success\n\n");
		return 0;
	}
}

int sendCommandDevOta(char *deviceID,char *filePath)
{
	if (deviceID==NULL || filePath==NULL)
		return -1;

	char *p;
	char fileName[60]={0};
	char fileType[10]={0};
	char version[10]={0};
	char path[40]={0};

	p=strrchr(filePath,'/');
	if(p)
		p=p+1;
	else
		p=filePath;
	strcpy(fileName,p);
	strncpy(path,filePath,p-filePath);


	char *q=strdup(fileName);
	char *q_tmp=q;
	char *cut="_";
	char *d=".";
	strsep(&q,cut);
	sprintf(fileType,"%s",strsep(&q,cut));
	sprintf(version,"%s",strsep(&q,d));
	if(NULL!=q_tmp)
	{
		free(q_tmp);
	}


	if(0!=rex_set_ota_image_file_path(path))
	{
		ERROR("Set device update file path failed!\n\n");
		//free(q);
		return -1;
	}


	struct stat statbuff;
	unsigned long filesize = 0;
	char fileOfSize[12];

	if (stat(filePath, &statbuff) == -1)
		return -1;
	filesize = statbuff.st_size;
	sprintf(fileOfSize, "%lu", filesize);

	int res;
	cJSON *root, *writeParam;

	root = cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Action", "W");

	cJSON_AddItemToObject(root, "WriteParameter", writeParam = cJSON_CreateObject());
	cJSON_AddStringToObject(writeParam, "ManufactureCode", "4451");
	cJSON_AddStringToObject(writeParam, "ImageType", fileType);
	cJSON_AddStringToObject(writeParam, "ImageVersion", version);
	cJSON_AddStringToObject(writeParam, "ImageLength", fileOfSize);
	cJSON_AddStringToObject(writeParam, "ReceivedLength", "0");
	cJSON_AddStringToObject(writeParam, "UpgradeType", "2");
	char *jsonString;
	jsonString = cJSON_Print(root);
	INFO("[ZIGBEE] device firmware informtion:\n");
	INFO("[ZIGBEE] %s\n\n", jsonString);

	res = rex_send_command_data(deviceID, 1, COMMAND_TYPE_DEVICE_OTA, jsonString);
	cJSON_Delete(root);
	if (jsonString)
	{
		free(jsonString);
	}

	if (res != 0)
	{
		WARN("[ZIGBEE] Send device %s ota command failed!\n\n",deviceID);
		return -1;
	}
	else
	{
		INFO("[ZIBGEE] Send device %s ota command success\n\n",deviceID);
		return 0;
	}

	return 0;
}

int sendCommandSetNetworkChannle(char *channel)
{

	if (atoi(channel)>26 || atoi(channel)<11)
		return -1;
	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root, "Channel", channel);
	char *strJson=cJSON_PrintUnformatted(root);

	int res;
	res=sendRexCommand("0000000000000000", 1, COMMAND_TYPE_SET_ZIGBEE_CHANNEL, strJson);
	free(strJson);
	cJSON_Delete(root);
	if (res != 0)
	{
		WARN("[ZIGBEE] Send set network channel failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send network channel success\n\n");
		return 0;
	}
}

int sendCommandSetReportAttribute(char *devID,\
		char *clusterId,\
		char *attributeId,\
		char *attributeType,\
		char *interval,\
		char *changeValue)
{


	cJSON *root;
	root=cJSON_CreateObject();
	cJSON_AddStringToObject(root, "CategoryCode", clusterId);

	cJSON *cfg;
	cJSON_AddItemToObject(root, "ReportingCfgs", cfg=cJSON_CreateArray());

	cJSON *item;
	cJSON_AddItemToArray(cfg, item=cJSON_CreateObject());
	cJSON_AddStringToObject(item, "AttributeCode", attributeId);
	cJSON_AddStringToObject(item, "AttributeType", attributeType);
	cJSON_AddStringToObject(item, "Interval", interval);
	cJSON_AddStringToObject(item, "ChangeValue", changeValue);

	char *strJson=cJSON_PrintUnformatted(root);
	DEBUG("%s\n",strJson);

	int res;
	res=sendRexCommand(devID, 1, COMMAND_TYPE_SET_REPROTING_PERIOD, strJson);
	free(strJson);
	cJSON_Delete(root);
	if (res != 0)
	{
		WARN("[ZIGBEE] Send set report attribute report failed!\n\n");
		return -1;
	}
	else
	{
		INFO("[ZIGBEE] Send report attribute report success\n\n");
		return 0;
	}
}
