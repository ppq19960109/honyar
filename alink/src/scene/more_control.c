#include "cloudLinkListFunc.h"
#include "cloudLinkReport.h"
#include "cloudLinkCtrl.h"
#include "cloudLink.h"

#include "more_control.h"
#include "scene.h"
#include "scene_database.h"

int more_control_add_scene(const char *localSceneId)
{
    cJSON *updateArray = cJSON_CreateArray();
    cJSON *arrayItem = cJSON_CreateObject();
    cJSON_AddItemToArray(updateArray, arrayItem);

    cJSON_AddStringToObject(arrayItem, "Op", "AddScene");

    cJSON_AddStringToObject(arrayItem, "Id", localSceneId);
    cJSON_AddStringToObject(arrayItem, "Enable", "1");
    cJSON_AddStringToObject(arrayItem, "ExecDelayed", "0");
    return sceneHyDispatch(updateArray);
}

int more_control_add_triggers(const char *localSceneId, cJSON *triggersItems)
{
    int triggersItemsSize = cJSON_GetArraySize(triggersItems);
    if (triggersItemsSize == 0)
        return 0;
    more_control_add_scene(localSceneId);

    cJSON *triggersArray = cJSON_CreateArray();

    for (int i = 0; i < triggersItemsSize; ++i)
    {
        cJSON *triggersItemsSub = cJSON_GetArrayItem(triggersItems, i);
        if (triggersItemsSub == NULL)
            continue;
        cJSON *deviceName = cJSON_GetObjectItem(triggersItemsSub, "deviceName");
        cJSON *SwitchLoop = cJSON_GetObjectItem(triggersItemsSub, "SwitchLoop");

        CloudLinkDev *cloudLinkDev = cloudLinkListGetById(deviceName->valuestring);
        if (cloudLinkDev == NULL)
            return -1;
        int i;
        char *hyKey = NULL;
        for (i = 0; i < cloudLinkDev->attrLen; ++i)
        {
            if (strcmp(SwitchLoop->valuestring, cloudLinkDev->attr[i].cloudKey) == 0)
            {
                break;
            }
        }
        if (i == cloudLinkDev->attrLen)
        {
            return -1;
        }
        else
        {
            hyKey = cloudLinkDev->attr[i].hyKey;
        }

        cJSON *arrayItem = cJSON_CreateObject();
        cJSON_AddItemToArray(triggersArray, arrayItem);
        cJSON_AddStringToObject(arrayItem, "Op", "AddCond");
        cJSON_AddStringToObject(arrayItem, "Id", localSceneId);

        cJSON_AddStringToObject(arrayItem, "Logic", "Or");
        cJSON_AddStringToObject(arrayItem, "CondType", "Event");
        cJSON_AddStringToObject(arrayItem, "TriggerType", "StateSync");
        cJSON_AddStringToObject(arrayItem, "Active", "Equ");
        cJSON_AddStringToObject(arrayItem, "KeyCoding", "Original");
        cJSON_AddStringToObject(arrayItem, "DevId", deviceName->valuestring);
        cJSON_AddStringToObject(arrayItem, "Key", hyKey);
        cJSON_AddStringToObject(arrayItem, "Value", "0");
        cJSON_AddStringToObject(arrayItem, "ContinueTime", "0");
    }
    return sceneHyDispatch(triggersArray);
}

int more_control_report(char *report, unsigned int len, const char *resp_report)
{
#define MAX_TEXT_LEN 2000
    cJSON *root = cJSON_CreateObject();
    char text_index[33];
    char buf[MAX_TEXT_LEN + 1] = {0};
    unsigned int buf_addr = 0;
    for (int i = 0; i < 6; ++i)
    {
        snprintf(text_index, sizeof(text_index), "switchBindReport%d", i);
        if (len > MAX_TEXT_LEN)
        {
            strncpy(buf, &report[buf_addr], MAX_TEXT_LEN);
            buf_addr += MAX_TEXT_LEN;
        }
        else if (len == buf_addr)
        {
            logDebug("%s is null", text_index);
            cJSON_AddStringToObject(root, text_index, "");
            continue;
        }
        else
        {
            strcpy(buf, &report[buf_addr]);
            buf_addr = len;
        }
        cJSON_AddStringToObject(root, text_index, buf);
    }
    //------------------------------
    cJSON_AddStringToObject(root, "switchBindReport", resp_report);

    //-------------------------------
    char *json = cJSON_PrintUnformatted(root);
    logInfo("more_control_report send json:%s\n", json);
    cJSON_Delete(root);

    user_post_event("SwitchBindReport", json);
    cJSON_free(json);
    return 0;
}
static int group_select_cb(const char *group_id, const char *product_key, const char *device_id, const char *switch_loop, void *arg)
{
    cJSON *ListData = (cJSON *)arg;
    cJSON *data = cJSON_CreateObject();
    cJSON_AddItemToArray(ListData, data);
    cJSON_AddStringToObject(data, "productKey", product_key);
    cJSON_AddStringToObject(data, "deviceName", device_id);
    cJSON_AddStringToObject(data, "SwitchLoop", switch_loop);
    return 0;
}
static int group_name_select_cb(const char *group_id, const char *group_name, void *arg)
{
    cJSON *QueryBindSwitch = (cJSON *)arg;
    logInfo("group_name_select_cb:%s,%s", group_id, group_name);

    cJSON *group = cJSON_CreateObject();
    cJSON_AddItemToArray(QueryBindSwitch, group);
    cJSON_AddStringToObject(group, "GroupId", group_id);
    cJSON_AddStringToObject(group, "Enable", "1");
    cJSON_AddStringToObject(group, "GroupName", group_name);
    cJSON *ListData = cJSON_AddArrayToObject(group, "ListData");
    table1_select(group_id, ListData, group_select_cb);
    return 0;
}
static int response_control(char *group_id, const char *bindSwitch)
{
    if (group_id == NULL)
        return -1;

    cJSON *root = cJSON_CreateObject();
    cJSON_AddNumberToObject(root, "code", 200);
    cJSON_AddStringToObject(root, "message", "ok");
    cJSON *BindSwitch = cJSON_AddArrayToObject(root, bindSwitch);

    if (strcmp("AddBindSwitch", bindSwitch) == 0)
    {
        table2_select(group_id, BindSwitch, group_name_select_cb);
    }
    else if (strcmp("ModifyBindSwitch", bindSwitch) == 0)
    {
        table2_select(group_id, BindSwitch, group_name_select_cb);
    }
    else if (strcmp("QueryBindSwitch", bindSwitch) == 0)
    {
        if (strlen(group_id) == 0)
        {
            table2_select(NULL, BindSwitch, group_name_select_cb);
        }
        else
        {
            table2_select(group_id, BindSwitch, group_name_select_cb);
        }
    }
    else if (strcmp("DeleteBindSwitch", bindSwitch) == 0)
    {
        cJSON_AddStringToObject(BindSwitch, "GroupId", group_id);
    }

    char *json = cJSON_PrintUnformatted(root);
    unsigned int json_len = strlen(json);
    logInfo("report_query json:%s\n", json);

    if (strcmp("QueryBindSwitch", bindSwitch) == 0)
    {
        void *encodeOut = malloc(BASE64_ENCODE_OUT_SIZE(json_len));
        int encodeOutlen = base64_encode((const unsigned char *)json, json_len, encodeOut);
        logDebug("base64_encode encodeOut:%s,%d,%d", encodeOut, encodeOutlen, strlen(encodeOut));
        cJSON_free(json);
        json = encodeOut;
        json_len = encodeOutlen;
    }

    cJSON *resp = cJSON_CreateObject();
    cJSON_AddNumberToObject(resp, "code", 200);
    cJSON_AddStringToObject(resp, "message", "ok");
    cJSON_AddStringToObject(resp, bindSwitch, "");
    char *resp_json = cJSON_PrintUnformatted(resp);
    cJSON_Delete(resp);

    more_control_report(json, json_len, resp_json);

    cJSON_free(resp_json);
    cJSON_free(json);
    cJSON_Delete(root);
    return 0;
}

int add_mod_more_control(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len)
{
    logInfo("add_more_control");
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        logError("JSON Parse Error");
        return -1;
    }
    //Type字段
    cJSON *GroupId = cJSON_GetObjectItem(root, "GroupId");
    if (GroupId == NULL)
    {
        logError("GroupId is NULL\n");
        goto fail;
    }

    table1_delete(GroupId->valuestring);
    table2_delete(GroupId->valuestring);
    delete_single_scene(GroupId->valuestring);

    cJSON *GroupName = cJSON_GetObjectItem(root, "GroupName");
    if (GroupName == NULL)
    {
        logError("GroupName is NULL\n");
        goto fail;
    }
    cJSON *ListData = cJSON_GetObjectItem(root, "ListData");
    if (ListData == NULL)
    {
        logError("ListData is NULL\n");
        goto fail;
    }
    int array_size = cJSON_GetArraySize(ListData);
    for (int i = 0; i < array_size; ++i)
    {
        cJSON *array_sub = cJSON_GetArrayItem(ListData, i);
        if (array_sub == NULL)
            continue;
        cJSON *productKey = cJSON_GetObjectItem(array_sub, "productKey");
        cJSON *deviceName = cJSON_GetObjectItem(array_sub, "deviceName");
        cJSON *SwitchLoop = cJSON_GetObjectItem(array_sub, "SwitchLoop");
        table1_insert(GroupId->valuestring, productKey->valuestring, deviceName->valuestring, SwitchLoop->valuestring);
    }
    table2_insert(GroupId->valuestring, GroupName->valuestring);
    more_control_add_triggers(GroupId->valuestring, ListData);

    char bindSwitch[33] = {0};
    strncpy(bindSwitch, serviceid, serviceid_len);
    int ret = response_control(GroupId->valuestring, bindSwitch);
    cJSON_Delete(root);
    return ret;
fail:
    cJSON_Delete(root);
    return -1;
}

int query_more_control(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len)
{
    logInfo("query_more_control");
    /* Parse Root */
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        logError("JSON Parse Error");
        return -1;
    }
    //Type字段
    cJSON *GroupId = cJSON_GetObjectItem(root, "GroupId");
    if (GroupId == NULL)
    {
        logError("GroupId is NULL\n");
        goto fail;
    }
    int ret = response_control(GroupId->valuestring, "QueryBindSwitch");
    cJSON_Delete(root);
    return ret;
fail:
    cJSON_Delete(root);
    return -1;
}

int delete_more_control(const int devid, const char *serviceid, const int serviceid_len, const char *request, char **response, int *response_len)
{
    logInfo("delete_more_control");
    /* Parse Root */
    cJSON *root = cJSON_Parse(request);
    if (root == NULL || !cJSON_IsObject(root))
    {
        logError("JSON Parse Error");
        return -1;
    }
    //Type字段
    cJSON *GroupId = cJSON_GetObjectItem(root, "GroupId");
    if (GroupId == NULL)
    {
        logError("GroupId is NULL\n");
        goto fail;
    }
    table1_delete(GroupId->valuestring);
    table2_delete(GroupId->valuestring);
    int ret = response_control(GroupId->valuestring, "DeleteBindSwitch");
    return ret;
fail:
    cJSON_Delete(root);
    return -1;
}