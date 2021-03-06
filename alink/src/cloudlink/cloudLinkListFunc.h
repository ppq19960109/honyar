#ifndef _CLOUDLINKLISTFUNC_H_
#define _CLOUDLINKLISTFUNC_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "main.h"

#include "khash.h"
    khint_t cloudLink_kh_begin();
    khint_t cloudLink_kh_end();
    void *cloudLink_kh_exist(int k);
#define cloudLink_kh_foreach_value(vvar) \
    for (khint_t __i = cloudLink_kh_begin(); (vvar = cloudLink_kh_exist(__i)) || __i != cloudLink_kh_end(); ++__i)

    typedef struct
    {
        char hyKey[33];
        char hyType[8];
        char cloudKey[33];
        char *value;
        char valueType;
    } CloudLinkDevAttr;

    typedef struct
    {
        char eventId[33];
        char hyKey[33];
        char key[33];
        char value;
    } CloudLinkEventAttr;

    typedef struct
    {
        char serverId[33];
        char hyKey[33];
        char key[33];
        char value[33];
    } CloudLinkServerAttr;

    typedef struct
    {
        char mac[33];
        char modelId[33];
        //alink attr
        int id;
        iotx_linkkit_dev_meta_info_t alinkInfo;
        CloudLinkDevAttr *attr;
        char attrLen;
        CloudLinkServerAttr *serverAttr;
        char serverAttrLen;
        CloudLinkEventAttr *eventAttr;
        char eventAttrLen;
    } CloudLinkDev;

    void *cloudLinkParseJson(const char *devId, const char *str);

    void cloudLinkListInit(void);
    void cloudLinkListDestroy(void);
    void cloudLinkListAdd(void *node);

    void cloudLinkDevFree(CloudLinkDev *ptr);
    void cloudLinkListDelDev(CloudLinkDev *ptr);
    void cloudLinkListEmpty(void);
    CloudLinkDev *cloudLinkListGetById(const char *id);
    CloudLinkDev *cloudLinkListGetBySn(const int sn);
    void cloudLinkListPrintf(void);

#ifdef __cplusplus
}
#endif
#endif