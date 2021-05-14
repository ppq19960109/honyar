/*
 * main.c
 *
 *  Created on: Feb 27, 2018
 *      Author: jerry
 */

#include "service.h"
//#include "udpDiscovery.h"
#include <pthread.h>
#include <stdio.h>
#include "hy_device_manage.h"
#include <unistd.h>



int main(int argc, char* argv[])
{
	int ch;
	hySetLogInit(5,"/tmp/log_hygateway");
	while((ch=getopt(argc,argv,"p::"))!=-1)
	{
		switch(ch)
		{
		case 'p':
			hySetLogInit(5,NULL);
			break;
		default:
			break;
		}
	}





	hySetResetButen8sFunHandler((void*)handler_8sButten);

	hySetDeviceManageDataBasePath("/usr/hygateway/");
	hySetConfigurFilePath("/usr/hygateway/configure");
	hySetFileDownLoadPath("/usr/hygateway/");
	hyQueryMinIntervalSet(200);
	hyLocalSceneInit("/usr/hygateway/sceneInfo.db", 0);
	hySetReportFunHandler((void*) handler_reportMessage);

	hyGatewayRun();

	service();


	return 0;

}
