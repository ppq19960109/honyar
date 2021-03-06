#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#include "cJSON.h"
#include "commonFunc.h"
#include "frameCb.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkSend.h"
#include "hylinkSubDev.h"
#include "hylinkListFunc.h"
#include "logFunc.h"

#include "hytool.h"
#ifndef ARCH
#include "rkDriver.h"
#endif // !ARCH

int hylinkInit(void)
{
    hylinkListInit();

    HyLinkDev *gwHyLinkDev = addProfileDev(HYLINK_PROFILE_PATH, STR_GATEWAY_DEVID, STR_GATEWAY_MODELID, hyLinkParseJson);
    if (gwHyLinkDev != NULL)
        runTransferCb(gwHyLinkDev, ATTR_REPORT_ALL, TRANSFER_CLOUD_REPORT);
    return 0;
}

int hylinkClose(void)
{
    hytoolClose();

    hylinkListEmpty();
#ifndef ARCH
    rkDriverClose();
#endif // !ARCH
    return 0;
}

void hylinkOpen(void)
{
#ifndef ARCH
    rkDriverOpen();
#endif // !ARCH

    registerTransferCb(hylinkRecv, TRANSFER_CLIENT_READ);
    registerTransferCb(hylinkSendDevAttr, TRANSFER_DEVATTR);
    registerSystemCb(hylinkHeart, CMD_HEART);

    registerSystemCb(hylinkSendDevInfo, CMD_DEVSINFO);

    hylinkInit();

    hytoolOpen();
}
