#ifndef _MQTT_SERVER_H_
#define _MQTT_SERVER_H_

#ifdef __cplusplus
extern "C"
{
#endif
    int mqtt_serverOpen(void);
    int mqtt_serverClose(void);
#ifdef __cplusplus
}
#endif
#endif