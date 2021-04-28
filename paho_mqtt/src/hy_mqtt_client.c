#include "main.h"
#include "hy_mqtt_client.h"
#include "MQTTClient.h"
#include "http_token.h"

#define QOS 1

static int gateway_online = 0;

static void delivered(void *context, MQTTClient_deliveryToken dt)
{
    printf("Message with token value %d delivery confirmed\n", dt);
}

static int msgarrvd(void *context, char *topicName, int topicLen, MQTTClient_message *message)
{
    // int i;
    // char *payloadptr;

    logInfo("Message arrived");
    logInfo("     topic: %s", topicName);
    // printf("   message: ");

    // payloadptr = message->payload;
    // for (i = 0; i < message->payloadlen; i++)
    // {
    //     putchar(*payloadptr++);
    // }
    // putchar('\n');
    logInfo("   message payload: %.*s\n", message->payloadlen, (char *)message->payload);
    // hylinkDispatch(message->payload, message->payloadlen);
    runTransferCb(message->payload, message->payloadlen, TRANSFER_CLIENT_WRITE);

    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

static void connlost(void *context, char *cause)
{
    logError("\nConnection lost\n");
    logError("     cause: %s\n", cause);
    mqtt_client_reconnect();
}
//------------------------------
#define CTRL_TOPIC "honyar/Dispatch/%s/#"
#define REPORT_TOPIC "honyar/Report/%s/%s/%s"
static MQTTClient client;
static char gateway_mac[18];
static char subscribe_topic[64];
static char publish_topic[100];

#define MQTT_SEND_BUF_LEN (2050 - 3)
static char mqttSendBuf[MQTT_SEND_BUF_LEN + 3];

int mqtt_client_publish(const char *topicName, char *payload, int payloadlen)
{
    int ret;
    MQTTClient_message pubmsg = MQTTClient_message_initializer;
    MQTTClient_deliveryToken token;

    pubmsg.payload = payload;
    pubmsg.payloadlen = payloadlen;
    pubmsg.qos = QOS;
    pubmsg.retained = 0;
    if (gateway_online)
        ret = MQTTClient_publishMessage(client, topicName, &pubmsg, &token);
    logInfo("publish topic:%s\npayload:%s\n", topicName, payload);
    return ret;
}

static int mqttSend(const char *str, const int str_len)
{
    int ret = -1;
    mqttSendBuf[0] = 0x02;

    strncpy(&mqttSendBuf[1], str, str_len > MQTT_SEND_BUF_LEN ? MQTT_SEND_BUF_LEN : str_len);
    mqttSendBuf[str_len + 1] = 0x03;
    mqttSendBuf[str_len + 2] = 0x00;

    ret = mqtt_client_publish(publish_topic, mqttSendBuf, str_len + 2);
    ret = runTransferCb(mqttSendBuf, str_len + 2, TRANSFER_SERVER_HYLINK_WRITE);
    return ret;
}

int mqtt_client_subscribe(const char *topicName)
{
    int ret;
    printf("mqtt client subsribe topic:%s\n", topicName);
    ret = MQTTClient_subscribe(client, topicName, QOS);
    if (ret != MQTTCLIENT_SUCCESS)
    {
        fprintf(stderr, "mqtt client subsribe topic fail:%d\n", ret);
    }
    return ret;
}

static int mqtt_Recv(void *recv, unsigned int len)
{
    char *json = (char *)recv;
    if (json == NULL)
        return -1;

    int ret = 0;
    cJSON *root = cJSON_Parse(json);
    if (root == NULL)
    {
        logError("root is NULL\n");
        return -1;
    }
    //command字段
    cJSON *Command = cJSON_GetObjectItem(root, STR_COMMAND);
    if (Command == NULL)
    {
        logError("Command is NULL\n");
        goto fail;
    }
    else if (strcmp(STR_BEATHEARTRESPONSE, Command->valuestring) == 0)
    {
        logDebug("Command is BeatHeartResponse\n");
        sprintf(publish_topic, REPORT_TOPIC, gateway_mac, gateway_mac, "heart");
        goto heart;
    }

    //Type字段
    cJSON *Type = cJSON_GetObjectItem(root, STR_TYPE);
    if (Type == NULL)
    {
        logError("Type is NULL\n");
        goto fail;
    }

    //Data字段
    cJSON *Data = cJSON_GetObjectItem(root, STR_DATA);
    if (Data == NULL)
    {
        logError("Data is NULL\n");
        goto fail;
    }

    char hyDevId[33] = {0};
    char *payload = NULL;

    int array_size = cJSON_GetArraySize(Data);
    if (array_size == 0)
        goto fail;
    cJSON *array_sub = cJSON_GetArrayItem(Data, 0);
    getStrForJson(array_sub, STR_DEVICEID, hyDevId);
    sprintf(publish_topic, REPORT_TOPIC, gateway_mac, hyDevId, Type->valuestring);

heart:
    payload = cJSON_PrintUnformatted(root);

    ret = mqttSend(payload, strlen(payload));

    cJSON_free(payload);

    cJSON_Delete(root);
    return ret;
fail:
    cJSON_Delete(root);
    return -1;
}

static int gateway_online_report(int onoff)
{
    gateway_online = onoff;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddStringToObject(root, STR_COMMAND, STR_REPORT);

    cJSON_AddStringToObject(root, STR_FRAMENUMBER, "0");

    cJSON_AddStringToObject(root, STR_TYPE, STR_ONOFF);

    cJSON *DataArray = cJSON_CreateArray();
    cJSON_AddItemToObject(root, STR_DATA, DataArray);

    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddItemToArray(DataArray, arrayItem);

    cJSON_AddStringToObject(arrayItem, STR_DEVICEID, gateway_mac);
    cJSON_AddStringToObject(arrayItem, STR_MODELID, STR_GATEWAY_MODELID);

    cJSON_AddStringToObject(arrayItem, STR_KEY, STR_ONLINE);
    if (onoff)
    {
        cJSON_AddStringToObject(arrayItem, STR_VALUE, "1");
    }
    else
    {
        cJSON_AddStringToObject(arrayItem, STR_VALUE, "0");
    }
    char *json = cJSON_PrintUnformatted(root);
    mqtt_Recv(json, strlen(json));
    cJSON_free(json);
    return 0;
}

//------------------------------
int mqtt_client_close(void)
{
    gateway_online_report(0);
    if (gateway_online)
    {
        MQTTClient_unsubscribe(client, subscribe_topic);
        MQTTClient_disconnect(client, 1000);
    }
    MQTTClient_destroy(&client);
    return 0;
}

int mqtt_client_reconnect(void)
{
    logWarn("mqtt_client_reconnect.........");
    mqtt_client_close();
    sleep(4);
    mqtt_client_open();
    return 0;
}

static char clientId[48];
static char password[1024];

int mqtt_client_open(void)
{
    const char *serverURI = MQTT_ADDRESS;
    const char *username = MQTT_USERNAME;

    // const char *clientId = MQTT_CLIENTID;
    // const char *password = MQTT_PASSWORD;

    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
    int rc;

    registerTransferCb(mqtt_Recv, TRANSFER_MQTT_REPORT);
    if (getNetworkMac(ETH_NAME, gateway_mac, sizeof(gateway_mac), "") == NULL)
    {
        fprintf(stderr, "get mac error\n");
        exit(1);
    }
    sprintf(clientId, "GID_HONYAR@@@%s", gateway_mac);

    MQTTClient_create(&client, serverURI, clientId,
                      MQTTCLIENT_PERSISTENCE_NONE, NULL);
reconnect:
    memset(password, 0, sizeof(password));
    curl_http_get_token(clientId, gateway_mac, password);
    printf("curl_http_get_token:%s,len:%d\n", password, strlen(password));
    conn_opts.keepAliveInterval = 20;
    conn_opts.cleansession = 1;
    conn_opts.username = username;
    conn_opts.password = password;
    MQTTClient_setCallbacks(client, NULL, connlost, msgarrvd, delivered);

    printf("mqtt server addr:%s\n", MQTT_ADDRESS);
    printf("mqtt server clientId:%s\n", clientId);
    printf("mqtt server username:%s\n", username);
    printf("mqtt server password:%s\n", password);
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS)
    {
        printf("Failed to connect, return code %d\n", rc);
        sleep(4);
        goto reconnect;
    }

    // if (mqtt_client_subscribe("honyar/Report/#") != MQTTCLIENT_SUCCESS)
    //     printf("mqtt_client_subscribe Report fail\n");

    sprintf(subscribe_topic, CTRL_TOPIC, gateway_mac);
    if (mqtt_client_subscribe(subscribe_topic) != MQTTCLIENT_SUCCESS)
        printf("mqtt_client_subscribe fail\n");
    else
        printf("mqtt_client_subscribe success:%s\n", subscribe_topic);
    gateway_online_report(1);
    return rc;
}
