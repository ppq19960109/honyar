#ifndef _ZIGBEELISTFUNC_H_
#define _ZIGBEELISTFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "list.h"

#include "khash.h"
    khint_t zigbee_kh_begin();
    khint_t zigbee_kh_end();
    void *zigbee_kh_exist(int k);

#define zigbee_kh_foreach_value(vvar) for (khint_t __i = zigbee_kh_begin(); (vvar = zigbee_kh_exist(__i)) || __i != zigbee_kh_end(); ++__i)

    typedef struct
    {
        /*状态、属性鸿雁协议名称*/
        char hyKey[33];
        char hyKeyPrivate;
        /*属性endpoint，zigbee设备使用*/
        char srcEndpoint;
        char dstEndpoint;
        unsigned short AttributeId;
        unsigned short ClusterId;
        char z3CmdType;
        char z3CmdId;
        unsigned char dataType;
    } ZigbeeAttr;

    typedef struct
    {
        char key[33];
        char (*value)[33];
        char valueLen;
    } SceneAttr;

    typedef struct
    {
        char modelId[33];
        char report_modelId[33];
        char manuName[33];
    #define HEART_AUTO_REPORT 1
        char heart_type; //0-default 1-auto report
        long heartbeatTime;
        ZigbeeAttr *attr;
        char attrLen;
        SceneAttr *sceneAttr;
        char sceneAttrLen;
    } zigbeeDev;

    void zigbeeListInit(void);
    void zigbeeListDestroy(void);
    void zigbeeListAdd(void *node);
    void zigbeeListDel(const char *key);
    void *zigbeeListGet(const char *key);
    void zigbeeListEmpty(void);
    void zigbeeListPrintf(void);
#ifdef __cplusplus
}
#endif
#endif