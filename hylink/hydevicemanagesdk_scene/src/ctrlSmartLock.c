/*
 * ctrlSmartLock.c
 *
 *  Created on: Nov 18, 2020
 *      Author: jerry
 */

#include "ctrlSmartLock.h"
#include "rex_export_gateway.h"
#include "rex_export_type.h"
#include "cJSON.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define SMART_LOCK_COMMAND_BUFF_LEN (500)

int packageSetKeyTime(char *command,int times,char *outbuff)
{
	cJSON *root=cJSON_Parse(command);
	if(!root)
		return -1;
	cJSON_DeleteItemFromObject(root,"KeyAuthority");
	cJSON_DeleteItemFromObject(root,"Password");
	cJSON *action=cJSON_GetObjectItem(root,"Action");
	if(!action)
	{
		cJSON_Delete(root);
		return -1;
	}

	sprintf(action->valuestring,"%s","12");
	char keyTime[8]={0};
	sprintf(keyTime,"%d",times);
	cJSON_AddStringToObject(root,"Times",keyTime);

	char *strJson=cJSON_PrintUnformatted(root);
	sprintf(outbuff,"%s",strJson);
	free(strJson);
	cJSON_Delete(root);
	return 0;

}


int handleCtrlLockCmd(KeyValue *head)
{
	if(!head)
		return -1;

	KeyValue *nowNode=head;
	int res=0;

	while(nowNode)
	{
		if (strcmp(nowNode->key,ADD_ONE_TIME_PASSWORD))
		{
			res = rex_send_command_data(nowNode->address, 1, COMMAND_TYPE_LOCK, nowNode->value);

			char outbuff[SMART_LOCK_COMMAND_BUFF_LEN]={0};
			if(0==packageSetKeyTime(nowNode->value,1,outbuff))
				rex_send_command_data(nowNode->address, 1, COMMAND_TYPE_LOCK, outbuff);

		}
		else if (strcmp(nowNode->key,CONFIGURE_KEY_TIME))
		{
			res = rex_send_command_data(nowNode->address, 1, COMMAND_TYPE_LOCK, nowNode->value);
		}
		else if (strcmp(nowNode->key,GET_KEY_TIME))
		{
			res = rex_send_command_data(nowNode->address, 1, COMMAND_TYPE_LOCK, nowNode->value);
		}
		else if (strcmp(nowNode->key,REMOTE_UNLOCK))
		{
			res = rex_send_command_data(nowNode->address, 1, COMMAND_TYPE_LOCK, nowNode->value);
		}
		else if (strcmp(nowNode->key,REMOVE_KEY_TIME))
		{
			res = rex_send_command_data(nowNode->address, 1, COMMAND_TYPE_LOCK, nowNode->value);
		}

		nowNode=nowNode->next;
	}

	return res;

}
