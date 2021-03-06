#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"
#include "cmd_run.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkListFunc.h"
#include "hylinkSend.h"

#include "zigbeeListFunc.h"
#include "database.h"

static int addDevToHyList(const char *devId, const char *modelId)
{
    if (devId == NULL || modelId == NULL)
        return -1;
    HylinkDev *hylinkDev = (HylinkDev *)malloc(sizeof(HylinkDev));
    memset(hylinkDev, 0, sizeof(HylinkDev));
    strcpy(hylinkDev->DeviceId, devId);
    strcpy(hylinkDev->ModelId, modelId);
    hylinkListAdd(hylinkDev);
    return 0;
}

void hyDevInitOnline(void)
{
    HylinkDev *hyDev = NULL;
    hyLink_kh_foreach_value(hyDev)
    {
        if (hyDev == NULL)
            continue;
        logWarn("hyLink_kh_foreach_value hyDevInitOnline:%s", hyDev->DeviceId);
        runZigbeeCb((void *)hyDev->DeviceId, (void *)hyDev->ModelId, STR_VERSION, NULL, ZIGBEE_DEV_DISPATCH);
    }
}

/*********************************************************************************
  *Function:  hylinkDevJoin
  * Description： report zigbee device registriation information
  *Input:  
    devId:device id
    modelId:invalid parameter
    version:version information
    manuName:tuya zigbee device model id
  *Return:  0:success -1:fail
**********************************************************************************/
static int hylinkDevJoin(void *devId, void *modelId, void *version, void *manuName)
{
    if (zigbeeListGet(manuName) == NULL)
    {
        logError("modelId is null");
        runCmdCb(devId, CMD_DELETE_DEV);
        return -1;
    }

    HylinkDev *hyDev = (HylinkDev *)malloc(sizeof(HylinkDev));
    memset(hyDev, 0, sizeof(HylinkDev));
    strcpy(hyDev->DeviceId, devId);
    strcpy(hyDev->ModelId, manuName);
    strcpy(hyDev->Version, version);
    hylinkListAdd(hyDev);
    insertDatabse(hyDev->DeviceId, hyDev->ModelId);

    // HylinkSend hylinkSend = {0};
    // HylinkSendData hylinkSendData = {0};
    // hylinkSend.Data = &hylinkSendData;
    // strcpy(hylinkSend.Type, STR_REGISTER);
    // hylinkSend.DataSize = 1;

    // strcpy(hylinkSendData.DeviceId, hyDev->DeviceId);
    // strcpy(hylinkSendData.ModelId, hyDev->ModelId);

    // hylinkSendFunc(&hylinkSend);

    // strcpy(hylinkSend.Type, STR_ATTRIBUTE);
    // strcpy(hylinkSendData.Key, STR_VERSION);
    // strcpy(hylinkSendData.Value, hyDev->Version);
    // hylinkSendFunc(&hylinkSend);

    hylinkSendSingleFunc(hyDev->DeviceId, NULL, STR_REGISTER, NULL, NULL);
    hylinkSendSingleFunc(hyDev->DeviceId, NULL, STR_ATTRIBUTE, STR_VERSION, hyDev->Version);

    runZigbeeCb((void *)hyDev->DeviceId, (void *)hyDev->ModelId, NULL, NULL, ZIGBEE_DEV_DISPATCH);
    return 0;
}

static int hylinkOnlineFresh(void *devId, void *null, void *version, void *null1)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        // runCmdCb(devId, CMD_DELETE_DEV);
        return -1;
    }
    logInfo("hylinkOnlineFresh");
    strcpy(hyDev->Version, version);
    //-------------------------------
    if (hyDev->activeTime == 0 || hyDev->version > 0)
    {
        hyDev->version = 0;
        logInfo("hylinkOnlineFresh:%s,online", devId);
        // HylinkSend hylinkSend = {0};
        // HylinkSendData hylinkSendData = {0};
        // hylinkSend.Data = &hylinkSendData;
        // hylinkSend.DataSize = 1;

        // strcpy(hylinkSendData.DeviceId, hyDev->DeviceId);
        // strcpy(hylinkSendData.ModelId, hyDev->ModelId);

        // strcpy(hylinkSend.Type, STR_ONOFF);
        // strcpy(hylinkSendData.Key, STR_ONLINE);
        // strcpy(hylinkSendData.Value, "1");
        // hylinkSendFunc(&hylinkSend);

        // strcpy(hylinkSend.Type, STR_ATTRIBUTE);
        // strcpy(hylinkSendData.Key, STR_VERSION);
        // strcpy(hylinkSendData.Value, hyDev->Version);
        // hylinkSendFunc(&hylinkSend);
        hylinkSendSingleFunc(hyDev->DeviceId, NULL, STR_ONOFF, STR_ONLINE, "1");
        hylinkSendSingleFunc(hyDev->DeviceId, NULL, STR_ATTRIBUTE, STR_VERSION, hyDev->Version);
    }
    hyDev->activeTime = time(NULL);
    return 0;
}

static int hylinkDevLeave(void *devId, void *null, void *null1, void *null2)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        return -1;
    }

    // HylinkSend hylinkSend = {0};
    // HylinkSendData hylinkSendData = {0};
    // hylinkSend.Data = &hylinkSendData;
    // hylinkSend.DataSize = 1;

    // strcpy(hylinkSend.Type, STR_UNREGISTER);

    // strcpy(hylinkSendData.DeviceId, hyDev->DeviceId);

    // hylinkSendFunc(&hylinkSend);

    hylinkSendSingleFunc(hyDev->DeviceId, NULL, STR_UNREGISTER, NULL, NULL);

    hylinkListDel(hyDev->DeviceId);
    deleteDatabse(hyDev->DeviceId);
    return 0;
}

static int hylinkDevZclReport(void *devId, void *hyKey, void *data, void *datalen)
{
    HylinkDev *hyDev = (HylinkDev *)hylinkListGet(devId);
    if (hyDev == NULL)
    {
        logError("hyDev is null");
        return -1;
    }

    // HylinkSend hylinkSend = {0};
    // HylinkSendData hylinkSendData = {0};
    // hylinkSend.Data = &hylinkSendData;
    // hylinkSend.DataSize = 1;

    // strcpy(hylinkSend.Type, STR_ATTRIBUTE);

    // strcpy(hylinkSendData.DeviceId, devId);
    // strcpy(hylinkSendData.ModelId, hyDev->ModelId);

    // strcpy(hylinkSendData.Key, hyKey);
    // strcpy(hylinkSendData.Value, data);
    // hylinkSendFunc(&hylinkSend);
    hylinkSendSingleFunc(hyDev->DeviceId, NULL, STR_ATTRIBUTE, hyKey, data);
    return 0;
}
//-----------------------------------------------------
static int hylinkReportDevInfo(void)
{
    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    hylinkSend.DataSize = 1;

    strcpy(hylinkSend.Type, STR_DEVSINFO);
    strcpy(hylinkSendData.DeviceId, STR_GATEWAY_DEVID);
    strcpy(hylinkSendData.Key, STR_DEVSINFO);

    return hylinkSendFunc(&hylinkSend);
}

static int hylinkReportNetAccess(void *data)
{
    unsigned char sec = *(unsigned char *)data;
    if (sec)
        sec = 1;
    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    hylinkSend.DataSize = 1;

    strcpy(hylinkSend.Type, STR_ATTRIBUTE);

    strcpy(hylinkSendData.DeviceId, STR_GATEWAY_DEVID);
    strcpy(hylinkSendData.ModelId, STR_GATEWAY_MODELID);

    strcpy(hylinkSendData.Key, STR_TIME);

    sprintf(hylinkSendData.Value, "%d", sec);

    return hylinkSendFunc(&hylinkSend);
}

static int hylinkZigbeeChannel(void)
{
    char buf[64] = {0};
    int ret = popenRun("grep -i 'channel' storage/zigbeeNetInfo.txt", "r", buf, sizeof(buf));
    if (ret < 0)
        return -1;
    long num = 0;
    strToNum(strchr(buf, ':') + 1, 10, &num);
    logWarn("popenRun: %s,%d", buf, num);

    //-------------------
    HylinkSend hylinkSend = {0};
    HylinkSendData hylinkSendData = {0};
    hylinkSend.Data = &hylinkSendData;
    strcpy(hylinkSend.Type, STR_ATTRIBUTE);
    hylinkSend.DataSize = 1;

    strcpy(hylinkSendData.DeviceId, STR_GATEWAY_DEVID);
    strcpy(hylinkSendData.ModelId, STR_GATEWAY_MODELID);

    strcpy(hylinkSendData.Key, "ZB_Channel");
    sprintf(hylinkSendData.Value, "%ld", num);

    return hylinkSendFunc(&hylinkSend);
}
//--------------------------------------------------------
int hylinkClose(void)
{
    databaseClose();
    hylinkListEmpty();
    return 0;
}

static int hylinkReset(void)
{
    HylinkDev *hylinkDev;
    hyLink_kh_foreach_value(hylinkDev)
    {
        runCmdCb(hylinkDev->DeviceId, CMD_DELETE_DEV);
    }

    hylinkClose();
    databseReset();
    return 0;
}

void hylinkOpen(void)
{
    registerSystemCb(hylinkReset, HYLINK_RESET);
    registerSystemCb(hylinkZigbeeChannel, HYLINK_ZB_CHANNEL);
    registerSystemCb(hylinkReportDevInfo, HYLINK_DEVSINFO);

    registerCmdCb(hylinkReportNetAccess, CMD_HYLINK_NETWORK_ACCESS);

    registerTransferCb(hylinkRecvManage, TRANSFER_CLIENT_READ);

    registerZigbeeCb(hylinkDevJoin, ZIGBEE_DEV_JOIN);
    registerZigbeeCb(hylinkOnlineFresh, ZIGBEE_DEV_ONLINE);
    registerZigbeeCb(hylinkDevLeave, ZIGBEE_DEV_LEAVE);
    registerZigbeeCb(hylinkDevZclReport, ZIGBEE_DEV_REPORT);

    hylinkListInit();
    databaseInit();
    selectDatabse(addDevToHyList);
}
