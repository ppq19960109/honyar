/*******************************************************************************************************
**                                           File description
** File Name:           rex_export_type.h
** Description:         定义SDK类型
** Creator:             xieshaobing
** Creation Date:       2017年4月7日
** Modify Log:          none
** Last Modified Date:  2017年4月7日
*******************************************************************************************************/

#ifndef __INC_REX_EXPORT_TYPE_H
#define __INC_REX_EXPORT_TYPE_H

/* If this is a C++ compiler, use C linkage */
#if defined(__cplusplus)
extern "C"
{
#endif

/** 记录日志水平 */
enum REX_LOG_LEVEL {
    //None：不记录日志
    REX_LL_NONE = 0,
    //Error：记录错误日志
    REX_LL_ERROR,
    //Debug：记录错误日志+函数调用跟踪日志
    REX_LL_DEBUG,
    //INFO：记录错误日志+函数调用跟踪日志+收发信息日志
    REX_LL_INFO
};

/** 跟踪显示水平 */
enum REX_TRACK_LEVEL {
    //None：不显示
    REX_TL_NONE = 0,
    //Error：显示错误
    REX_TL_ERROR,
    //Debug：显示错误+函数调用跟踪
    REX_TL_DEBUG,
    //INFO：显示错误+函数调用跟踪+收发信息
    REX_TL_INFO
};

/** 回调函数种类 */
enum REX_CALLBACK_TYPE {
    //设备入网回调函数
    REX_DEVICE_JOIN = 0,
    //设备离网回调函数
    REX_DEVICE_LEAVE,
    //上报状态数据回调函数
    REX_REPORT_STATE_DATA,
    //上报事件数据回调函数
    REX_REPORT_EVENT_DATA,
    //上报自定义数据回调函数
    REX_REPORT_SELF_DEFINING_DATA
};

/**
 * REX_DEVICE_JOIN_CB
 *     设备入网回调函数
 * address[in]: 设备地址
 * reporting[in]: 上报周期
 * type[in]: 设备类型
 * model_id[in]: Model Id
 * return: 0成功,否则-1
 * note: 设备地址采用16个16进制字符表示，即”0000000000000000”~”FFFFFFFFFFFFFFFF”
 *       model_id最长6个字节
 */
typedef int (*REX_DEVICE_JOIN_CB)(char *address, unsigned short reporting, char *type, char *model_id);

/**
 * REX_DEVICE_LEAVE_CB
 *     设备离网回调函数
 * address[in]: 设备地址
 * return: 0成功,否则-1
 * note: 设备地址采用16个16进制字符表示，即”0000000000000000”~”FFFFFFFFFFFFFFFF”
 */
typedef int (*REX_DEVICE_LEAVE_CB)(char *address);

/**
 * REX_REPORT_STATE_DATA_CB
 *     上报状态数据回调函数
 * address[in]: 设备地址
 * endpoint_id[in]: 设备端ID号,默认为1,多路设备从1依次递增
 * state_type[in]: 状态种类
 * state[in]: 状态数据,为JSON字符串
 * return: 0成功,否则-1
 * note: 设备地址采用16个16进制字符表示，即”0000000000000000”~”FFFFFFFFFFFFFFFF”
 */
typedef int (*REX_REPORT_STATE_DATA_CB)(char *address, unsigned char endpoint_id, unsigned short state_type, char *state);

/**
 * REX_REPORT_EVENT_DATA_CB
 *     上报事件数据回调函数
 * address[in]: 设备地址
 * endpoint_id[in]: 设备端ID号,默认为1,多路设备从1依次递增
 * event_type[in]: 事件种类
 * event[in]: 事件数据,为JSON字符串
 * return: 0成功,否则-1
 * note: 设备地址采用16个16进制字符表示，即”0000000000000000”~”FFFFFFFFFFFFFFFF”
 */
typedef int (*REX_REPORT_EVENT_DATA_CB)(char *address, unsigned char endpoint_id, unsigned short event_type, char *event);

/**
 * REX_REPORT_SELF_DEFINING_DATA_CB
 *     上报自定义数据回调函数
 * address[in]: 设备地址
 * endpoint_id[in]: 设备端ID号,默认为1,多路设备从1依次递增
 * data[in]: 自定义数据
 * length[in]: 自定义数据长度
 * return: 0成功,否则-1
 * note: none
 */
typedef int (*REX_REPORT_SELF_DEFINING_DATA_CB)(char *address, unsigned char endpoint_id, char *data, unsigned short length);

/* If this is a C++ compiler, use C linkage */
#if defined(__cplusplus)
}
#endif

#endif
/*******************************************************************************************************
**                                           End of file
*******************************************************************************************************/
