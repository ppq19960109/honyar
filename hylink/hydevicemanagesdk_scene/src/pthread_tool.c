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
#include <unistd.h>  
#include <memory.h>   
#include <sys/types.h>  
#include <signal.h>

#include "pthread_tool.h"
#include "error_no.h"
#include "log.h"

static void *thread_routine(void *arg);
static int thread_create_detach(thread_pool_t *pool, int idx);

/************************************************************* 
 **功  能：线程池的初始化 
 **参  数： 
 **    pool：线程池对象 
 **    num ：线程池中线程个数 
 **返回值：0：成功 !0: 失败 
 *************************************************************/  
int thread_pool_init(thread_pool_t **pool, int num)  
{
	int ret = 0;
    int idx = 0;  
  
    /* 为线程池分配空间 */  
    *pool = (thread_pool_t*)calloc(1, sizeof(thread_pool_t));  
    if(NULL == *pool)  
    {  
        return -1;  
    }  
  
    /* 初始化线程池 */  
    pthread_mutex_init(&((*pool)->queue_lock), NULL);  
    pthread_cond_init(&((*pool)->queue_ready), NULL);  
    (*pool)->head = NULL;  
    (*pool)->reqnum = num;  
    (*pool)->queue_size = 0;  
    (*pool)->isdestroy = 0;  
    (*pool)->threadid = (pthread_t*)calloc(1, num*sizeof(pthread_t));  
    if(NULL == (*pool)->threadid)  
    {  
        free(*pool);  
        (*pool) = NULL;  
  
        return -1;  
    }  
  
    /* 依次创建线程 */  
    for(idx=0; idx<num; idx++)  
    {  
        ret = thread_create_detach(*pool, idx);  
        if(0 != ret)  
        {  
            return -1;  
        }  
        (*pool)->num++;  
    }  
  
    return 0;  
}

/************************************************************* 
 **功  能：将任务加入线程池处理队列 
 **参  数： 
 **    pool：线程池对象 
 **    process：需处理的任务 
 **    arg: process函数的参数 
 **返回值：0：成功 !0: 失败 
 *************************************************************/  
int thread_pool_add_worker(thread_pool_t *pool, void *(*process)(void *arg), void *arg, int arg_len)  
{  
    thread_worker_t *worker=NULL, *member=NULL;  
      
    worker = (thread_worker_t*)calloc(1, sizeof(thread_worker_t));  
    if(NULL == worker)  
    {  
        return -1;  
    }  
  
    worker->process = process;  
	if(arg)
	{
		void *pFunArgTmp = calloc(arg_len, 1);
		if(NULL == pFunArgTmp)
		{
			printf("Calloc error.\n");
			return -1;
		}
		memcpy(pFunArgTmp, arg, arg_len);
		worker->arg = pFunArgTmp;
		worker->arg_len = arg_len;
	}
	else
	{
		worker->arg = arg;
		worker->arg_len = 0;
	}
    worker->next = NULL;  
  
    pthread_mutex_lock(&(pool->queue_lock));  
  
    member = pool->head;  
    if(NULL != member)  
    {  
        while(NULL != member->next) member = member->next;  
        member->next = worker;  
    }  
    else  
    {  
        pool->head = worker;  
    }  
  
    pool->queue_size++;  
  
    pthread_mutex_unlock(&(pool->queue_lock));  
    pthread_cond_signal(&(pool->queue_ready));  
  
    return 0;  
}  
/****************************************************************************** 
 **函数名称: thread_pool_keepalive 
 **功    能: 线程保活 
 **输入参数:  
 **       pool: 线程池 
 **输出参数: NONE 
 **返    回: 0: success !0: failed 
 **实现过程: 
 **      1. 判断线程是否存在 
 **      2. 不存在，说明线程死亡，需重新创建 
 ******************************************************************************/  
int thread_pool_keepalive(thread_pool_t *pool)  
{  
    int idx=0, ret=0;  
  
    for(idx=0; idx<pool->num; idx++)  
    {  
        ret = pthread_kill(pool->threadid[idx], 0);  
        if(ESRCH == ret)  
        {  
            ret = thread_create_detach(pool, idx);  
            if(ret < 0)  
            {  
                return -1;  
            }  
        }  
    }  
  
    return 0;  
}  

/************************************************************* 
 **功  能：线程池的销毁 
 **参  数： 
 **    pool：线程池对象 
 **返回值：0：成功 !0: 失败 
 *************************************************************/  
int thread_pool_destroy(thread_pool_t *pool)  
{  
	int ret = 0;
    int idx = 0;  
    thread_worker_t *member = NULL;  
  
    if(0 != pool->isdestroy)  
    {  
        return -1;  
    }  
  
    pool->isdestroy = 1;  
  
    pthread_cond_broadcast(&(pool->queue_ready));  
    for(idx=0; idx<pool->num; idx++)  
    {  
        ret = pthread_kill(pool->threadid[idx], 0);  
        if(ESRCH == ret)  
        {  
            continue;  
        }  
        else  
        {  
            idx--;  
            sleep(1);  
        }  
    }  
  
    free(pool->threadid);  
    pool->threadid = NULL;  
  
    while(NULL != pool->head)  
    {  
        member = pool->head;  
        pool->head = member->next;  
        free(member);  
    }  
  
    pthread_mutex_destroy(&(pool->queue_lock));  
    pthread_cond_destroy(&(pool->queue_ready));  
    free(pool);  
      
    return 0;  
}  

/************************************************************* 
 **功  能：线程池各个线程入口函数 
 **参  数： 
 **    arg：线程池对象 
 **返回值：0：成功 !0: 失败 
 *************************************************************/  
static void *thread_routine(void *arg)
{  
    thread_worker_t *worker = NULL;  
    thread_pool_t *pool = (thread_pool_t*)arg;  
  
    while(1)  
    {  
        pthread_mutex_lock(&(pool->queue_lock));  
        while((0 == pool->isdestroy) && (0 == pool->queue_size))  
        {  
            pthread_cond_wait(&(pool->queue_ready), &(pool->queue_lock));  
        }  
  
        if(0 != pool->isdestroy)  
        {  
            pthread_mutex_unlock(&(pool->queue_lock));  
            pthread_exit(NULL);  
        }  
  
        pool->queue_size--;  
        worker = pool->head;  
        pool->head = worker->next;
        pthread_mutex_unlock(&(pool->queue_lock));
        /* 执行队列中的任务 */  
        (*(worker->process))(worker->arg);  

		if(worker->arg)
		{
			free(worker->arg);
		}
		
        free(worker);  
        worker = NULL;  
    }  
}  

/****************************************************************************** 
 **函数名称: thread_create_detach 
 **功    能: 创建分离线程 
 **输入参数:  
 **       pool: 线程池 
 **       idx: 线程索引号 
 **输出参数: NONE 
 **返    回: 0: success !0: failed 
 ******************************************************************************/  
static int thread_create_detach(thread_pool_t *pool, int idx)
{  
    int ret = 0;  
    pthread_attr_t attr;  
  
    do  
    {
        ret = pthread_attr_init(&attr);
        if(0 != ret)
        {
            return -1;
        }
        
        ret = pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);
        if(0 != ret)
        {
            return -1;
        }
        
        ret = pthread_create(&(pool->threadid[idx]), &attr, thread_routine, pool);
        if(0 != ret)
        {
            pthread_attr_destroy(&attr);
            
            if(EINTR == errno)
            {
                continue;
            }
            return -1;
        }
        pthread_attr_destroy(&attr);
    }while(0);

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
