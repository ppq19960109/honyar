/***********************************************************
*文件名     : error_no.h
*版   本   : v1.0.0.0
*日   期   : 2018.05.31
*说   明   : 错误号
*修改记录: 
************************************************************/
#ifndef ERROR_NO_H
#define ERROR_NO_H

#include <errno.h>

#define	ERROR_INFO_MAX_LEN		1024

/*错误号定义*/
	/**success*/
#define	ERROR_SUCCESS			0
	/** bad input parameters */
#define	ERROR_PARAM				-1
    /** not enough available space */
#define	ERROR_NO_SPACE			-2
    /** item / data not found */
#define	ERROR_NOT_FOUND			-3
	/** system error, see errno for more details */
#define	ERROR_SYSTEM			-4
    /** general error */
#define	ERROR_GEN				-5
	/** time out*/
#define	ERROR_TIMEOUT			-6
	/** send or recv error*/
#define	ERROR_TXRX				-7
	/** be not online */
#define	ERROR_NOT_ONLINE		-8
	/** read lock */
#define	ERROR_READ_LOCK			-9
	/** write lock */
#define	ERROR_WRITE_LOCK		-10
	/** json format error */
#define	ERROR_JSON_FORMAT		-11
	/** parse json error : avalid value */
#define	ERROR_JSON_VALUE		-12
	/** Add error */
#define	ERROR_ADD				-13
	/** Del error */
#define	ERROR_DEL				-14
	/** Set error */
#define	ERROR_SET				-15
	/** Get error */
#define	ERROR_GET				-16
	/** Open database failed*/
#define	ERROR_OPEN_DB			-17
	/** Open file failed*/
#define	ERROR_OPEN_FILE			-18
	/** Great socket failed*/
#define	ERROR_OPEN_SOCKET		-19



extern char g_acErrorInfo[ERROR_INFO_MAX_LEN];

/*************************************************************
*函数:	strerrorinfo
*参数:	iErrorNo :错误号
*返回值:返回错误信息
*描述:	获取错误信息
*************************************************************/
extern char * 
strerrorinfo(int iErrorNo);



#endif /* ERROR_NO_H */

