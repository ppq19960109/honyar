/*
 * custonm_attribute.c
 *
 *  Created on: Jul 29, 2019
 *      Author: jerry
 */

#include <dirent.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "cJSON.h"
#include "log.h"
#include "common.h"
#include "rex_export_gateway.h"


#define CUSTOM_ATTRIBUTE_FILE  "custom_attribute"
#define MAX_ATTRI_LENGTH    (1024)


extern char configurFilePath[DEV_CONFIGUR_FILE_PATH_MAX_LENGTH];



int setCustomAttributes()
{
	int fd;
	char filePath[200];
	char buff[MAX_ATTRI_LENGTH];


	sprintf(filePath,"%s/%s", configurFilePath, CUSTOM_ATTRIBUTE_FILE);

	if ((fd = open(filePath, O_RDONLY)) == -1)
	{
		ERROR("Open custom attribute configuration file failed: %s\n\n",filePath);
		return -1;
	}

	read(fd, buff, MAX_ATTRI_LENGTH);
	close(fd);

	cJSON *root;
	root=cJSON_Parse(buff);

	cJSON *customAttri;
	customAttri=cJSON_GetObjectItem(root,"CustomAttri");
	if (customAttri==NULL)
	{
		cJSON_Delete(root);
		return -1;
	}


	int num=cJSON_GetArraySize(customAttri);
	int i=0;
	cJSON *item;
	cJSON *cluserId;
	cJSON *attributrId;
	cJSON *attributeType;
	cJSON *name;
	int res=0;
	for(i=0;i<num;i++)
	{
		item=cJSON_GetArrayItem(customAttri,i);
		cluserId=cJSON_GetObjectItem(item,"ClusterId");
		attributrId=cJSON_GetObjectItem(item,"AttributeId");
		attributeType=cJSON_GetObjectItem(item,"AttributeType");
		name=cJSON_GetObjectItem(item,"AttributeName");

		if(cluserId==NULL || attributrId==NULL || attributeType==NULL || name==NULL)
			continue;

		res=rex_configure_attribute((unsigned short)cluserId->valueint,(unsigned short)attributrId->valueint,name->valuestring,(unsigned char)attributeType->valueint);
		if(res==1)
			ERROR("Attribute Name repeats:%s\n\n",name->valuestring);
		else if(res==2)
			ERROR("CluserId %d or AttributeId %d repeats\n\n",cluserId->valueint,attributrId->valueint);
		else if (res==3)
			ERROR("Unknow error\n\n");
		else
			DEBUG("Set custom attribute CluserId=%d,AttributeId=%d,attributeName=%s\n\n",\
					cluserId->valueint,attributrId->valueint,name->valuestring);
	}
	cJSON_Delete(root);
	return 0;
}







