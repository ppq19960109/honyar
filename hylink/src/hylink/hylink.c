#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/reboot.h>

#include "frameCb.h"
#include "logFunc.h"
#include "cJSON.h"
#include "commonFunc.h"

#include "hylink.h"
#include "hylinkRecv.h"
#include "hylinkListFunc.h"
#include "hylinkSend.h"

#include "database.h"
#include "scene_adapter.h"

int hylinkReportHeart(void)
{
  HylinkSend hylinkSend = {0};
  hylinkSend.Command = 2;
  hylinkSend.DataSize = 0;
  return hylinkSendFunc(&hylinkSend);
}
// static int hylinkSendQueryVersion(void *devId)
// {
//   HylinkSend hylinkSend = {0};
//   hylinkSend.Command = 1;
//   strcpy(hylinkSend.Type, STR_DEVATTRI);
//   hylinkSend.DataSize = 1;
//   HylinkSendData hylinkSendData = {0};
//   hylinkSend.Data = &hylinkSendData;

//   strcpy(hylinkSendData.DeviceId, devId);
//   strcpy(hylinkSendData.Key, STR_VERSION);
//   return hylinkSendFunc(&hylinkSend);
// }
int addDevToHyList(const char *devId, const char *modelId)
{
  if (devId == NULL || modelId == NULL)
    return -1;
  HylinkDev *hylinkDev = (HylinkDev *)malloc(sizeof(HylinkDev));
  memset(hylinkDev, 0, sizeof(HylinkDev));
  strcpy(hylinkDev->DeviceId, devId);
  strcpy(hylinkDev->ModelId, modelId);
  hylinkListAdd(hylinkDev);
  // hylinkSendQueryVersion(hylinkDev->DeviceId);
  return 0;
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
int hylinkNetAccess(void *data)
{
  int sec = *(unsigned char *)data;

  HylinkSend hylinkSend = {0};
  hylinkSend.Command = 1;
  strcpy(hylinkSend.Type, STR_ADD);
  hylinkSend.DataSize = 1;
  HylinkSendData hylinkSendData = {0};
  hylinkSend.Data = &hylinkSendData;

  strcpy(hylinkSendData.DeviceId, STR_GATEWAY_DEVID);
  strcpy(hylinkSendData.ModelId, STR_GATEWAY_MODELID);
  strcpy(hylinkSendData.Key, STR_TIME);
  sprintf(hylinkSendData.Value, "%d", sec);

  return hylinkSendFunc(&hylinkSend);
}

int systemReset(void)
{
  HylinkSend hylinkSend = {0};
  hylinkSend.Command = 0;
  strcpy(hylinkSend.Type, STR_REFACTORY);
  hylinkSendFunc(&hylinkSend);
  //-------------------------------
  hylinkSend.Command = 1;
  hylinkSendFunc(&hylinkSend);
  printf("scene_adapter_reset......\n");
  scene_adapter_reset();
  sleep(1);
  printf("databseReset......\n");
  databseReset();
  printf("SYSTEM_CLOSE......\n");
  runSystemCb(SYSTEM_CLOSE);
  sleep(10);
  sync();
  printf("reboot......\n");
  reboot(RB_AUTOBOOT);
  return 0;
}
//--------------------------------------------------------
int hylinkClose(void)
{
  printf("hylinkClose......\n");
  scene_adapter_close();
  hylinkListEmpty();
  databaseClose();
  return 0;
}

void hylinkOpen(void)
{
  registerSystemCb(systemReset, SYSTEM_RESET);

  registerTransferCb(hylinkRecvManage, TRANSFER_SERVER_HYLINK_READ);
  registerTransferCb(hylinkRecvManage, TRANSFER_SERVER_ZIGBEE_READ);
  registerCmdCb(hylinkNetAccess, CMD_NETWORK_ACCESS);
  hylinkListInit();

  databaseInit();
  selectDatabse(addDevToHyList);
  scene_adapter_open();
}
