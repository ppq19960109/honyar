#ifndef __MAIN_H_
#define __MAIN_H_

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/reboot.h>
#include <pthread.h>
#include <errno.h>

#include "cJSON.h"
#include "curl/curl.h"

#include "logFunc.h"
#include "cmd_run.h"
#include "commonFunc.h"
#include "networkFunc.h"

#include "frameCb.h"

#include "hylink.h"
#include "hylinkListFunc.h"
#include "hylinkSubDev.h"
#include "hylinkSend.h"
#include "hylinkRecv.h"

#define ETH_NAME "eth0.2"
#define TOKEN_URL "http://home.sikelai.cn:8328/token/applyToken"
#define LOCAL_TCP_PORT 8001
#define SOFTWARE_VERSION "1.0.2"

#define MQTT_ADDRESS "post-cn-oew22m4al1d.mqtt.aliyuncs.com"
// #define MQTT_CLIENTID "GID_HONYAR@@@0003"
#define MQTT_USERNAME "Token|JNKMaQHBiFiwGPQJ|post-cn-oew22m4al1d"
// #define MQTT_PASSWORD "YRT773vNA3H/0IgbUeQSqbWyM/c="

#endif
