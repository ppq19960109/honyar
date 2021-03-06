
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkSend.h"

static char hyLinkSendBuf[1024];
int hylinkDispatch(const char *str, const int str_len, const char dir)
{
    int ret = 0;

    hyLinkSendBuf[0] = 0x02;
    strncpy(&hyLinkSendBuf[1], str, str_len);
    hyLinkSendBuf[str_len + 1] = 0x03;
    hyLinkSendBuf[str_len + 2] = 0x00;

    logInfo("hylinkDispatch:%s,dir:%d\n", hyLinkSendBuf, dir);
    if (dir == 1)
    {
        ret = runTransferCb(hyLinkSendBuf, str_len + 2, TRANSFER_SERVER_ZIGBEE_WRITE);
    }
    else
    {
        ret = runTransferCb(hyLinkSendBuf, str_len + 2, TRANSFER_SERVER_HYLINK_WRITE);
    }
    return ret;
}

int hylinkSendFunc(HylinkSend *hylinkSend)
{
    if (hylinkSend == NULL)
        goto fail;

    cJSON *root = cJSON_CreateObject();
    if (hylinkSend->Command == 1)
    {
        cJSON_AddStringToObject(root, STR_COMMAND, STR_DISPATCH);
    }
    else if (hylinkSend->Command == 2)
    {
        cJSON_AddStringToObject(root, STR_COMMAND, "BeatHeartResponse");
        goto heart;
    }
    else
    {
        cJSON_AddStringToObject(root, STR_COMMAND, STR_REPORT);
    }

    cJSON_AddStringToObject(root, STR_FRAMENUMBER, "00");

    cJSON_AddStringToObject(root, STR_TYPE, hylinkSend->Type);

    cJSON *DataArray = cJSON_AddArrayToObject(root, STR_DATA);

    for (int i = 0; i < hylinkSend->DataSize; ++i)
    {
        cJSON *arrayItem = cJSON_CreateObject();
        cJSON_AddItemToArray(DataArray, arrayItem);

        if (strlen(hylinkSend->Data[i].DeviceId))
            cJSON_AddStringToObject(arrayItem, STR_DEVICEID, hylinkSend->Data[i].DeviceId);
        if (strlen(hylinkSend->Data[i].ModelId))
            cJSON_AddStringToObject(arrayItem, STR_MODELID, hylinkSend->Data[i].ModelId);
        if (strlen(hylinkSend->Data[i].Key))
            cJSON_AddStringToObject(arrayItem, STR_KEY, hylinkSend->Data[i].Key);
        if (strlen(hylinkSend->Data[i].Value))
            cJSON_AddStringToObject(arrayItem, STR_VALUE, hylinkSend->Data[i].Value);
        if (strlen(hylinkSend->Data[i].Online))
            cJSON_AddStringToObject(arrayItem, STR_ONLINE, hylinkSend->Data[i].Online);
    }
    char *json = NULL;
heart:
    json = cJSON_PrintUnformatted(root);
    logInfo("hylink send json:%s\n", json);

    int ret = hylinkDispatch(json, strlen(json), hylinkSend->Command);

    cJSON_free(json);
    cJSON_Delete(root);
    return ret;
fail:
    return -1;
}
