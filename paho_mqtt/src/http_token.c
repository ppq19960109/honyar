#include "main.h"

static size_t get_write_cb(void *ptr, size_t size, size_t nmemb, void *stream)
{
    printf("get_write_cb size:%u,nmemb:%u\n", size, nmemb);
    printf("get_write_cb data:%s\n", (char *)ptr);
    char *passwd = (char *)stream;

    cJSON *root = cJSON_Parse(ptr);
    if (root == NULL)
        return -1;
    cJSON *statusCode = cJSON_GetObjectItem(root, "statusCode");
    if (statusCode->valueint != 200)
        goto fail;
    cJSON *message = cJSON_GetObjectItem(root, "message");
    printf("get_write_cb message:%s\n", message->valuestring);
    cJSON *result = cJSON_GetObjectItem(root, "result");
    cJSON *expireTime = cJSON_GetObjectItem(result, "expireTime");
    printf("get_write_cb expireTime:%d\n", expireTime->valueint);
    cJSON *token = cJSON_GetObjectItem(result, "token");
    sprintf(passwd, "R|%s|W|%s", token->valuestring, token->valuestring);
    // sprintf(passwd, "%s", token->valuestring);
    cJSON_Delete(root);
    return size * nmemb;
fail:
    cJSON_Delete(root);
    return -1;
}

int curl_http_get_token(const char *clientId, const char *mac, char *passwd)
{
    char *http_body = NULL;

    cJSON *root = cJSON_CreateObject();
    cJSON *headerInfo = cJSON_AddObjectToObject(root, "headerInfo");
    cJSON_AddObjectToObject(root, "result");

    cJSON_AddStringToObject(headerInfo, "appName", "HONYAN");
    cJSON_AddStringToObject(headerInfo, "devMac", mac);
    cJSON_AddStringToObject(headerInfo, "clientId ", clientId);

    http_body = cJSON_PrintUnformatted(root);
    if (http_body == NULL)
        goto fail;
    //-------------------------------
    CURLcode res;

    curl_global_init(CURL_GLOBAL_DEFAULT);

    // get a curl handle
    CURL *curl = curl_easy_init();
    if (curl)
    {
        struct curl_slist *pList = NULL;
        pList = curl_slist_append(pList, "Content-Type:application/json;charset=UTF-8");
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, pList);

        curl_easy_setopt(curl, CURLOPT_URL, TOKEN_URL);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, http_body);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, get_write_cb);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, passwd);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
        curl_easy_setopt(curl, CURLOPT_HEADER, 0);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);

        // perform the request, res will get the return code
        res = curl_easy_perform(curl);
        // check for errors
        if (res != CURLE_OK)
        {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }
        else
        {
            fprintf(stderr, "curl_easy_perform() success.\n");
        }

        // always cleanup
        curl_easy_cleanup(curl);
    }

    curl_global_cleanup();
    if (http_body)
    {
        cJSON_free(http_body);
    }
    if (strlen(passwd) == 0)
        goto fail;
    return 0;
fail:
    printf("http get token fail...........\n");
    return -1;
}