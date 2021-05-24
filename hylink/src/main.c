#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"

#include "hylink.h"

#include "hytool.h"
#include "log_api.h"
#ifndef ARCH
#include "rkDriver.h"
#else
#include "u86_driver.h"
#endif // !ARCH
#ifdef DEBUG
#include <mcheck.h>
#endif // DEBUG
int mainClose(void)
{
    hytoolClose();

    hylinkClose();
#ifndef ARCH
    rkDriverClose();
#endif // !ARCH
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
#ifndef ARCH
    rkDriverOpen();
#else
    u86_diver();
#endif // !ARCH
    // InitLog(LOG_DEBUG, NULL);
    registerSystemCb(mainClose, SYSTEM_CLOSE);
    hylinkOpen();

    logInfo("hylinkapp main start");
    hytoolOpen();
    while (1)
    {
        sleep(1);
    }
    // logInfo("hylinkapp mainClose");
    // mainClose();
    return 0;
}