/*
 * signature.h
 *
 *  Created on: Nov 9, 2018
 *      Author: jerry
 */

#ifndef SRC_SIGNATURE_H_
#define SRC_SIGNATURE_H_
#include "hyprotocol.h"
/*************************
 *函数名：handleSetGatewaySignaturCommand
 *描述： 添加网关签名信息指令;
 *参数：@ KeyValue *command
 *返回：0成功，-1失败;
 */
int handleSetGatewaySignaturCommand(KeyValue *command);

/*************************
 *函数名：handleGetGatewaySignaturCommand
 *描述： 获取网关签名信息指令;
 *参数：@ RecvCommand *command
 *返回：0成功，-1失败;
 ************************/
int handleGetGatewaySignaturCommand();


/*************************
 *函数名：unpackAndReportSetSignaturReport
 *描述： 解析设置网关签名信息结果指令，并打包向平台上报的信息;
 *参数：@ cJSON *input：输入cJOSN;
 *参数：@ char *outBuff: 输出打包的信息
 *返回：0成功，-1失败;
 ************************/
int unpackAndReportSetSignaturReport(cJSON *input,char *outBuff);

/*********************
 *函数名：packGatewaySignatureInfoReport
 *描述： 解析设置网关签名信息结果指令，并打包向平台上报的信息;
 *参数：@ cJSON *input：输入cJOSN;
 *参数：@ char *outBuff: 输出打包的信息
 *返回：0成功，-1失败;
 */
int packGatewaySignatureInfoReport(char *input,char *outbuff);

#endif /* SRC_SIGNATURE_H_ */
