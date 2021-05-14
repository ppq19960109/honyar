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

/* 线程任务链表 */  
typedef struct _thread_worker_t  
{  
    void *(*process)(void *arg);  /* 线程处理的任务 */  
    void *arg;                    /* 任务接口参数 */  
	int arg_len;				/*参数长度*/
    struct _thread_worker_t *next;/* 下一个节点 */  
}thread_worker_t;  
  
/* 线程池对象 */  
typedef struct  
{  
    pthread_mutex_t queue_lock;   /* 队列互斥锁 */  
    pthread_cond_t queue_ready;   /* 队列条件锁 */  
  
    thread_worker_t *head;        /* 任务队列头指针 */  
    int isdestroy;    /* 是否已销毁线程 */  
    pthread_t *threadid;          /* 线程ID数组 —动态分配空间 */  
    int reqnum;                   /* 请求创建的线程个数 */  
    int num;                      /* 实际创建的线程个数 */  
    int queue_size;               /* 工作队列当前大小 */  
}thread_pool_t;  
  
/* 函数声明 */  
extern int thread_pool_init(thread_pool_t **pool, int num);  
extern int thread_pool_add_worker(thread_pool_t *pool, void *(*process)(void *arg), void *arg, int arg_len);  
extern int thread_pool_keepalive(thread_pool_t *pool);  
extern int thread_pool_destroy(thread_pool_t *pool); 

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

void *mutex_lock_create(void);

void mutex_lock_destroy(void *mutex);

void mutex_lock(void *mutex);

void mutex_unlock( void *mutex);


#endif /*PTHREAD_TOOL_H*/

