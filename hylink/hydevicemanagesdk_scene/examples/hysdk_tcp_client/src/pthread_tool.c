/***********************************************************
*文件名     : pthread_tool.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.23
*说   明   : 线程相关接口
*修改记录: 
************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "pthread_tool.h"
#include "error_no.h"
#include "log.h"

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
                              void *arg)
{
	int iRet = 0;
	pthread_attr_t attr;
    iRet = pthread_attr_init(&attr);
    if(0 != iRet)
    {
    	ERROR("pthread_attr_init Error.\n");
        return -1;
    }
    iRet = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
    if(0 != iRet)
    {
    	pthread_attr_destroy(&attr);
        ERROR("pthread_attr_setdetachstate Error.\n");
        return -1;
    }
    iRet = pthread_create(pid, &attr, pthread_handle, arg);
    if(0 != iRet)
    {
        pthread_attr_destroy(&attr);
        ERROR("pthread_create Error.\n");
        return -1;
    }
    pthread_attr_destroy(&attr);
	return 0;
}


void *mutex_lock_create(void)
{
	int iRet = 0;
	pthread_mutex_t *mutex = (pthread_mutex_t *)malloc(sizeof(pthread_mutex_t));
	if (NULL == mutex) {
		return NULL;
	}

	if (0 != (iRet = pthread_mutex_init(mutex, NULL)))
	{
		ERROR("create mutex failed");
		free(mutex);
		return NULL;
	}

	return mutex;
}

void mutex_lock_destroy(void *mutex)
{
	int iRet = 0;

	if (!mutex)
	{
		WARN("mutex want to destroy is NULL!");
		return;
	}
	if (0 != (iRet = pthread_mutex_destroy((pthread_mutex_t *)mutex)))
	{
		ERROR("destroy mutex failed:  '%s' (%d)", strerror(iRet), iRet);
	}

	free(mutex);

	return;
}

void mutex_lock(void *mutex)
{
	int iRet = 0;

	if (0 != (iRet = pthread_mutex_lock((pthread_mutex_t *)mutex))) 
	{
		ERROR("lock mutex failed: '%s' (%d)", strerror(iRet), iRet);
	}
}

void mutex_unlock( void *mutex)
{
	int iRet = 0;

	if (0 != (iRet = pthread_mutex_unlock((pthread_mutex_t *)mutex)))
	{
		ERROR("unlock mutex failed- '%s' (%d)", strerror(iRet), iRet);
	}
}

