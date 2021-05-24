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
#ifdef DEBUG
#include <mcheck.h>
#endif // DEBUG

static int zigbee_init_flag = 0;
int hylink_connect(void)
{
    printf("---hylink_connect ...\n");
    if (!zigbee_init_flag)
    {
        zigbee_init_flag = 1;
        // usleep(2000);
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
#ifdef DEBUG
    muntrace();
    unsetenv("MALLOC_TRACE");
#endif // DEBUG
    return 0;
}

int main()
{
#ifdef DEBUG
    logInfo("tuyazigbee debug app main start");
    setenv("MALLOC_TRACE", "./memleak.log", 1);
    mtrace();
#endif // DEBUG
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