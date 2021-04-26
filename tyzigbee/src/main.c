#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"

#include "hylink.h"
#include "zigbee.h"
#include "cpython.h"
#include "heartbeat.h"

#include "hytool.h"

static int zigbee_init_flag = 0;
int hylink_connect(void)
{
    printf("---hylink_connect ...\n");
    if (!zigbee_init_flag)
    {
        zigbee_init_flag = 1;
        zigbeeOpen();
    }
    return 0;
}

static int mainClose(void)
{
    hytoolClose();
    zigbeeClose();
    hylinkClose();
    cpythonDestroy();
    return 0;
}

int main()
{
    cpythonInit();
    // return pythonTest();
    registerSystemCb(heartbeat, SYSTEM_HEARTBEAT);
    registerSystemCb(mainClose, SYSTEM_CLOSE);
    hylinkOpen();
    hytoolOpen();

    logInfo("tuyazigbee app main start");
    while (1)
    {
        sleep(1);
    }
    mainClose();
    return 0;
}