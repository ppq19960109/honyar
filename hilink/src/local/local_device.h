#ifndef _LOCAL_DEVICE_H_
#define _LOCAL_DEVICE_H_

#ifdef __cplusplus
extern "C"
{
#endif

#include "local_receive.h"

    typedef struct
    {
        char *const *attr;
        unsigned short attrLen;
    } SAttrInfo;

    typedef struct
    {
        char PermitJoining;
        char FirmwareVersion[8];
        char SoftVersion[8];
    } DevGateway_t;

    typedef struct
    {
        char Switch;
        char LedEnable;
        char PowerOffProtection;
    } dev_HY0095_t; //U2/天际系列：单键智能开关（HY0095）

    typedef struct
    {
        char Switch[2];
        char LedEnable;
        char PowerOffProtection;
    } dev_HY0096_t; //U2/天际系列：双键智能开关（HY0096）
    typedef struct
    {
        char Switch[3];
        char LedEnable;
        char PowerOffProtection;
    } dev_HY0097_t; //U2/天际系列：三键智能开关（HY0097）

    typedef struct
    {
        int ColorTemperature; //色温
        char Luminance;       //亮度
        char Switch;
    } dev_09223f_t; //U2/天际系列：DLT液晶调光器（09223f，型号U86KTGS150-ZXP）

    typedef struct
    {
        char Switch;
        char LedEnable;
        char PowerOffProtection;
        char KeyMode;
    } dev_HY0121_t; //1路智能开关模块（HY0121，型号IHC1238）
    typedef struct
    {
        char Switch[2];
        char LedEnable;
        char PowerOffProtection;
        char KeyMode;
    } dev_HY0122_t; //2路智能开关模块（HY0122，型号IHC1239）
    typedef struct
    {
        char Switch[3];
        char LedEnable;
        char PowerOffProtection;
        char KeyMode;
    } dev_HY0107_t; //3路智能开关模块（HY0107，型号IHC1240）

    //----------------------------------
    typedef struct
    {
        char ContactAlarm;
        char BatteryPercentage;
        char LowBatteryAlarm;
        char TamperAlarm;
        char init_ContactAlarm;
        char door_event;
    } dev_HY0093_t; //门磁传感器（HY0093，型号IHG5201）

    typedef struct
    {
        char KeyFobValue;
        char SceName[12][18];
        char ScePhoto[12];
        char Enable[3];
        char Switch[3];
        char CurrentTemperature;
        char TargetTemperature[2];
        char WorkMode;
        char WindSpeed[2];

    } dev_HY0134_t; //U2/天际系列：智镜/全面屏/触控屏（HY0134）

    extern const SAttrInfo g_SLocalModel;
    extern const SAttrInfo g_SLocalAttr[];

    int local_attribute_update(dev_local_t *dev, cJSON *Data);

    void hyLinkDevStatus(dev_local_t *src, int status);
    void hyLinkStatus(int online, int status);

#define INT_REBOOT 0
#define INT_REFACTORY 1
#define INT_OFFLINE 2

    void hyLinkSystem(int index);
#ifdef __cplusplus
}
#endif
#endif