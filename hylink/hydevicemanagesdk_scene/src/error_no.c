/***********************************************************
*文件名     : error_no.c
*版   本   : v1.0.0.0
*日   期   : 2018.05.31
*说   明   : 错误号
*修改记录: 
************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "error_no.h"

/*错误信息*/
char g_acErrorInfo[ERROR_INFO_MAX_LEN] = {0};

/*************************************************************
*函数:	strerrorinfo
*参数:	iErrorNo :错误号
*返回值:返回错误信息
*描述:	获取错误信息
*************************************************************/
extern char * 
strerrorinfo(int iErrorNo)
{
	memset(g_acErrorInfo, 0x0, ERROR_INFO_MAX_LEN);
	switch (iErrorNo)
	{
		case ERROR_SUCCESS:
			strncpy(g_acErrorInfo, "success", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_PARAM:
			strncpy(g_acErrorInfo, "bad input parameters", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_NO_SPACE:
			strncpy(g_acErrorInfo, "not enough available space", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_NOT_FOUND:
			strncpy(g_acErrorInfo, "item / data not found", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_SYSTEM:
			strncpy(g_acErrorInfo, strerror(errno), ERROR_INFO_MAX_LEN);
			break;
		case ERROR_GEN:
			strncpy(g_acErrorInfo, "general error", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_TIMEOUT:
			strncpy(g_acErrorInfo, "time out", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_TXRX:
			strncpy(g_acErrorInfo, "send or recv error", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_NOT_ONLINE:
			strncpy(g_acErrorInfo, "be not online", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_READ_LOCK:
			strncpy(g_acErrorInfo, "read lock", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_WRITE_LOCK:
			strncpy(g_acErrorInfo, "write lock", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_JSON_FORMAT:
			strncpy(g_acErrorInfo, "json format error", ERROR_INFO_MAX_LEN);
			break;
		case ERROR_JSON_VALUE:
			strncpy(g_acErrorInfo, "parse json error : avalid value", ERROR_INFO_MAX_LEN);
			break;
		default:
			strncpy(g_acErrorInfo, "invalid error no", ERROR_INFO_MAX_LEN);
			break;
	}
	return g_acErrorInfo;
}

