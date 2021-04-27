#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include <pthread.h>
#include <time.h>

#include <poll.h>
#include "frameCb.h"
#include "base_api.h"

static time_t press_time = 0;
void resetButtenProcess(char value)
{
    printf("resetButten %c\n", value);
    if ('1' == value)
    {
        if (press_time == 0)
        {
            return;
        }
        /*抬起*/
        time_t press_lift_time = time(NULL) - press_time;
        printf("press lift time:%ld\n", press_lift_time);
        if (press_lift_time >= 10)
        {
            printf("press time over 10s,start reset\n");
            runSystemCb(SYSTEM_RESET);
        }
    }
    else if ('0' == value)
    {
        /*按下*/
        press_time = time(NULL);
    }
    else
    {
        printf("press button error\n");
        press_time = 0;
    }
}

static void *gpio_key_thread(void *arg)
{
    sigset_t mask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    pthread_sigmask(SIG_BLOCK, &mask, NULL);

    int fd, ret;
    char value;
    struct pollfd pfd;

    fd = open("/sys/class/gpio/gpio42/value", O_RDWR);
    if (fd < 0)
    {
        printf("open file fail\n");
        return NULL;
    }
    /* poll要监听的文件描述符号 */
    pfd.fd = fd;
    /* 监听的文件的事件类型,当配置为中断'edge‘的时候.events必须设置为POLLPRI
	　　  * 详情查看内核中的文档gpio.txt */
    pfd.events = POLLPRI | POLLERR;
    /* 由于value文件中本来就有数据需要读取，没有读取的话，会被当成一个中断而进行处理 */
    //到文件开头读取
    ret = lseek(fd, 0, SEEK_SET);
    if (ret == -1)
    {
        printf("lseek error\n");
        return NULL;
    }
    //读取1字节
    ret = read(fd, &value, 1);

    while (1)
    {
        /* 监听个数1， 等待时间无限 */
        ret = poll(&pfd, 1, -1);
        if (ret == -1)
        {
            printf("poll error\n");
            continue;
        }
        /* 监听的时间会保存在revents成员中 */
        if (pfd.revents & POLLPRI)
        {
            //文件指针只想文件的开头
            ret = lseek(fd, 0, SEEK_SET);
            if (ret == -1)
            {
                printf("lseek error\n");
                continue;
            }
            //读取，结果为字符'0'或者‘1’
            read(fd, &value, 1);
            resetButtenProcess(value);
        }
        else if (pfd.revents & POLLERR)
        {
            printf("poll error\n");
        }
    }

    close(fd);

    return NULL;
}

/*gpio控制*/
void init_gpio_run(void)
{

    /*初始化按键gpio 42*/
    base_system("echo \"42\" > /sys/class/gpio/export", NULL, 0);
    base_system("echo \"in\" > /sys/class/gpio/gpio42/direction", NULL, 0);
    base_system("echo \"both\" > /sys/class/gpio/gpio42/edge", NULL, 0);
}

void u86_diver(void)
{
    init_gpio_run();
    pthread_t tid;
    pthread_create(&tid, NULL, (void *)gpio_key_thread, NULL);
    //要将id分配出去。
    pthread_detach(tid);
}
