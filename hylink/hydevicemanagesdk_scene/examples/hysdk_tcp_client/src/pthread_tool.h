/***********************************************************
*文件名     : pthread_tool.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.23
*说   明   : 线程相关接口
*修改记录: 
************************************************************/
#ifndef PTHREAD_TOOL_H
#define PTHREAD_TOOL_H

#include <pthread.h>

typedef void* (*PthreadFunc)(void*);


/*************************************************************
*函数:	pthread_detached_create
*参数:	pid:线程ID
*		pthread_handle:线程执行函数
*		arg:线程执行函数参数
*返回值:0表示成功，非0便是失败
*描述:	创建分离线程
*************************************************************/
extern int
pthread_detached_create(pthread_t *pid,
                              PthreadFunc pthread_handle,
                              void *arg);



#endif /*PTHREAD_TOOL_H*/

