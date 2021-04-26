#include "main.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <arpa/inet.h>

static int udp_broadcast_response(const int fd, struct sockaddr_in *from, const char *json, int len)
{
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
    else if (strcmp("RequestTcp", Command->valuestring))
    {
        logDebug("Command is not RequestTcp\n");
        goto fail;
    }
    cJSON_Delete(root);
    //------------------------------
    char gateway_ip[18] = {0};
    char gateway_mac[18] = {0};
    char port[8] = {0};
    if (getNetworkIp(ETH_NAME, gateway_ip, sizeof(gateway_ip)) == NULL)
    {
        fprintf(stderr, "get ip error\n");
        goto fail;
    }
    if (getNetworkMac(ETH_NAME, gateway_mac, sizeof(gateway_mac), ".") == NULL)
    {
        fprintf(stderr, "get mac error\n");
        goto fail;
    }

    cJSON *response = cJSON_CreateObject();
    cJSON_AddStringToObject(response, STR_COMMAND, "Response");
    cJSON_AddStringToObject(response, "IP", gateway_ip);
    sprintf(port, "%d", LOCAL_TCP_PORT);
    cJSON_AddStringToObject(response, "Port", port);
    cJSON_AddStringToObject(response, "Mac", gateway_mac);
    cJSON_AddStringToObject(response, "Model", "U2-86GW-AC(TY)");

    char *response_json = cJSON_PrintUnformatted(response);
    sendto(fd, response_json, strlen(response_json), 0, (struct sockaddr *)from, sizeof(struct sockaddr));

    cJSON_free(response_json);
    cJSON_Delete(response);
    //------------------------------

    return 0;
fail:
    cJSON_Delete(root);
    return -1;
}

static void *udp_broadcast_thread(void *arg)
{

    int sock = -1;
    if ((sock = socket(AF_INET, SOCK_DGRAM, 0)) == -1)
    {
        perror("socket error:");
        return NULL;
    }

    // 绑定地址
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(8000);
    if (bind(sock, (struct sockaddr *)&(addr), sizeof(struct sockaddr_in)) == -1)
    {
        perror("bind error...:\n");
        return NULL;
    }

    struct sockaddr_in from;
    socklen_t from_len;
    char buf[256] = {0};
    ssize_t recv_len;
    while (1)
    {
        //从广播地址接受消息
        recv_len = recvfrom(sock, buf, sizeof(buf), 0, (struct sockaddr *)&from, &from_len);
        if (recv_len <= 0)
        {
            perror("read error....");
            continue;
        }
        udp_broadcast_response(sock, &from, buf, recv_len);
    }

    return 0;
}

int udp_broadcast(void)
{
    pthread_t tid;
    pthread_create(&tid, NULL, (void *)udp_broadcast_thread, NULL);
    //要将id分配出去。
    pthread_detach(tid);
    return 0;
}