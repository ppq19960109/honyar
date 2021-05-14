/*******************************************************************************************************
**                                           File description
** File Name:           rex_export_gateway.h
** Description:         定义SDK接口函数
** Creator:             xieshaobing
** Creation Date:       2017年4月7日
** Modify Log:          none
** Last Modified Date:  2017年4月7日
*******************************************************************************************************/

#ifndef __INC_REX_EXPORT_GATEWAY_H
#define __INC_REX_EXPORT_GATEWAY_H

/* If this is a C++ compiler, use C linkage */
#if defined(__cplusplus)
extern "C"
{
#endif

/**
 * rex_init_process
 *     初始化处理
 * log_level[in]: 记录日志水平, 参见REX_LOG_LEVEL枚举
 * log_max_szie[in]: 日志最大字节数
 * track_level[in]: 跟踪显示水平, 参见REX_TRACK_LEVEL枚举
 * return: 0成功,否则-1
 * note: none
 */
extern int rex_init_process(char log_level, int log_max_szie, char track_level);

/**
 * rex_set_serial_port
 *     设置串口
 * name[in]: 串口名称
 * baud_rate[in]: 串口波特率
 * return: 0成功,否则-1
 * note: none
 */
extern int rex_set_serial_port(char *name, unsigned int baud_rate);

/**
 * rex_set_ota_image_file_path
 *    设置OTA固件文件路径
 * path[in]: 路径，采用绝对路径
 * return: 0成功,否则-1
 */
extern int rex_set_ota_image_file_path(char *path);

/**
 * rex_set_callback
 *     设置回调函数
 * cb_type[in]:回调函数种类
 * cb_function[in]: 回调函数
 * return: 0成功,否则-1
 * note: none
 */
extern int rex_set_callback(unsigned char cb_type, void *cb_function);

/**
 * rex_start
 *     开始
 * return: 0成功,否则-1
 * note: none
 */
extern int rex_start();

/**
 * rex_configure_attribute
 *     配置属性
 * cluster_id[in]: 簇标识号
 * attribute_id[in]: 属性标识号
 * attribute_name[in]: 属性名称
 * attribute_type[in]: 属性类型码
 * return: 0成功,1属性名称重名,2簇标识号与属性标识号重复,3其它错误
 */
extern int rex_configure_attribute(unsigned short cluster_id, unsigned short attribute_id, char *attribute_name, unsigned char attribute_type);

/**
 * rex_send_command_data
 *     发送命令数据
 * address[in]: 设备地址
 * endpoint_id[in]: 设备端ID号,默认为1,多路设备从1依次递增
 * command_type[in]: 命令类型
 * command[in]: 命令数据,为JSON字符串
 * return: 0成功,否则-1
 * note: 设备地址采用16个16进制字符表示，即”0000000000000000”~”FFFFFFFFFFFFFFFF”
 *       函数返回成功只代表命令发送成功，不代表命令执行成功，命令执行是否成功取决于响应数据的上报
 */
extern int rex_send_command_data(char *address, unsigned char endpoint_id, unsigned short command_type, char *command);

/**
 * rex_send_group_command_data
 *     发送组命令数据
 * group_id[in]: 组号
 * endpoint_id[in]: 设备端ID号,预设为1,多路设备从1依次递增
 * command_type[in]: 命令类型
 * command[in]: 命令数据,为JSON字符串
 * return: 0成功,否则-1
 * note: 组号取值1~65527
 *       函数返回成功只代表命令发送成功，不代表命令执行成功，命令执行是否成功取决于响应数据的上报
 *
 */
extern int rex_send_group_command_data(unsigned short group_id, unsigned char endpoint_id, unsigned short command_type, char *command);

/**
 * rex_send_self_defining_data
 *     发送自定义数据
 * address[in]: 设备地址
 * group_id[in]: 组号
 * endpoint_id[in]: 设备端ID号,预设为1,多路设备从1依次递增
 * data[in]: 自定义数据,为字节流
 * length[in]: 自定义数据长度
 * return: 0成功,否则-1
 * note: 组号取值1~65527
 *       函数返回成功只代表命令发送成功，不代表命令执行成功，命令执行是否成功取决于响应数据的上报
 */
extern int rex_send_self_defining_data(char *address, unsigned short group_id, unsigned char endpoint_id, char *data, unsigned short length);

/**
 * rex_end
 *     结束
 * return: 0成功,否则-1
 * note: none
 */
extern int rex_end();

/* If this is a C++ compiler, use C linkage */
#if defined(__cplusplus)
}
#endif

#endif
/*******************************************************************************************************
**                                           End of file
*******************************************************************************************************/
