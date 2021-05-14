#ifndef _PTHREADPOOL_H_
#define _PTHREADPOOL_H_

/*线程池初始化
@int max_thread_num:最大线程数
*/
extern void pool_init(int max_thread_num);


/*向线程池中加入任务*/
extern int pool_add_worker(void *(*process) (void *arg), void *arg);



#endif 