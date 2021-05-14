
#include "scene.h"

typedef int(*DevReport)(const char *);
DevReport reportDeviceMessage;

int setMessageReportFunHandler(void* funName)
{
	reportDeviceMessage = funName;
	return  0;
}


typedef int(*ResetButten8s)(void);
ResetButten8s resetButten8sEvent;

int serResetButen8sFunHandler(void* funName)
{
	resetButten8sEvent = funName;
	return  0;
}



int messageReportFun(char *inputBuff)
{
	scene_event(inputBuff);
	return  (*reportDeviceMessage)(inputBuff);
}


int resetButen8sEventFun()
{
	return  (*resetButten8sEvent)();
}
