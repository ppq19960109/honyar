#include "main.h"
#include "mqtt_server.h"
#include "hy_mqtt_client.h"
#include "udp_broadcast.h"

static int main_close(void)
{
  mqtt_client_close();
  mqtt_serverClose();

  hylinkClose();
  exit(0);
  return 0;
}
int main(void)
{
  registerSystemCb(main_close, SYSTEM_CLOSE);
  registerSystemCb(main_close, SYSTEM_RESET);
  udp_broadcast();

  mqtt_client_open();

  hylinkOpen();

  mqtt_serverOpen();
  // while (1)
  // {
  //   sleep(1);
  // }
  // main_close();
  return 0;
}