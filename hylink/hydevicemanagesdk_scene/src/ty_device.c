/*
 * ty_device.c
 *
 *  Created on: Nov 2, 2020
 *      Author: jerry
 */
#include <string.h>
#include <stdio.h>


#include "common.h"

#include "rexCommand.h"
#include "ty_device.h"

int isTYDeviceFirmware(char *realModelId)
{
	int res=0;

	if (0==strncmp(realModelId,"TS",2) || \
			0==strncmp(realModelId,"TY",2)|| \
			0==strncmp(realModelId,"TZ",2)|| \
			0==strncmp(realModelId,"_T",2))
		res=1;
	else if(0==strncmp(realModelId,"_T",2))
		res=1;
	else
		res=0;

	return res;
}

