#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "heartbeat.h"

#include "hylinkListFunc.h"
#include "hylinkSend.h"
#include "hylinkRecv.h"

#include "zigbeeManage.h"
#include "zigbeeListFunc.h"

int heartbeat(void)
{
    logDebug("heartbeat.........");
    time_t curTime = time(NULL);

    HylinkDev *hyDev = NULL;
    hyLink_kh_foreach_value(hyDev)
    {
        logWarn("hyLink_kh_foreach_value heartbeat");
        if (hyDev != NULL && hyDev->activeTime != 0)
        {
            zigbeeDev *zDev = (zigbeeDev *)zigbeeListGet(hyDev->ModelId);
            if (zDev == NULL)
            {
                logError("zDev is null");
                continue;
            }
            if (curTime > hyDev->activeTime + zDev->heartbeatTime)
            {
                logWarn("heartbeat timeout zigbee dev id:%s,model id:%s\n", hyDev->DeviceId, hyDev->ModelId);
                hyDev->activeTime = 0;

                // HylinkSend hylinkSend = {0};
                // strcpy(hylinkSend.Type, STR_ONOFF);
                // hylinkSend.DataSize = 1;
                // HylinkSendData hylinkSendData = {0};
                // hylinkSend.Data = &hylinkSendData;

                // strcpy(hylinkSendData.DeviceId, hyDev->DeviceId);
                // strcpy(hylinkSendData.ModelId, hyDev->ModelId);
                // strcpy(hylinkSendData.Key, STR_ONLINE);
                // strcpy(hylinkSendData.Value, "0");

                // hylinkSendFunc(&hylinkSend);
                hylinkSendSingleFunc(hyDev->DeviceId, NULL, STR_ONOFF, STR_ONLINE, "0");
            }
            else if (curTime > hyDev->activeTime + HEART_TIMER)
            {
                if (zDev->heart_type != HEART_AUTO_REPORT)
                    runZigbeeCb((void *)hyDev->DeviceId, (void *)hyDev->ModelId, STR_VERSION, NULL, ZIGBEE_DEV_DISPATCH);
            }
            else
            {
                logWarn("hy dev online %s,%s", hyDev->DeviceId, hyDev->ModelId);
            }
        }
    }
    return 0;
}
