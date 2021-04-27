/*
 * led.c
 *
 *  Created on: Jan 22, 2018
 *      Author: jerry
 */
#include "callback.h"
#include "hyprotocol.h"
#include "log.h"
#include "led.h"
#include "my_gpio_driver.h"
#include "base_api.h"


#include <unistd.h>
#include <stdio.h>


static int cooLedFlag=0;
static int ledFlag=0;


void sys_start_run_led(int time)
{
	int i;
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_pin_value(40,0);
#endif
	int count;
	if (time==255)
	{
		count=2000;
	}
	else
	{
		count=(float)time/1.2;
	}

	INFO("[ZIGBEE] Flowing water light begin: total time %d\n\n",time);
    for(i=0;i<count;++i)
    {
#ifdef BOARD_WHITE_BOX_GATEWAY	
        //wan(39)  lan(40) wifi (44)
        mt76x8_gpio_set_pin_value(39,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(40,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(44,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(39,0);
        usleep(200000);
        mt76x8_gpio_set_pin_value(40,0);
        usleep(200000);
        mt76x8_gpio_set_pin_value(44,0);
        usleep(200000);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
        //wan(39)  lan(40) wifi (44)
        mt76x8_gpio_set_pin_value(39,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(40,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(44,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(39,0);
        usleep(200000);
        mt76x8_gpio_set_pin_value(40,0);
        usleep(200000);
        mt76x8_gpio_set_pin_value(44,0);
        usleep(200000);
#endif
#ifdef BOARD_U86_GATEWAY
		mt76x8_gpio_set_pin_value(40,1);
        usleep(1200000);
#endif
        if (ledFlag==0)
        {
            break;
        }
    }
    ledFlag=0;
    //结束
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_mode_led(40);
#endif	
    

	char outBuff[400];
	ReportAttrEventOnline permitJoin;
	sprintf(permitJoin.deviceId, "%s", "0000000000000000");
	sprintf(permitJoin.modelId, "%s", "000000");
	sprintf(permitJoin.key, "%s", "PermitJoining");
	sprintf(permitJoin.value, "%s", "0");
	permitJoin.next = NULL;
	packageReportMessageToJson(REPORT_TYPE_ATTRIBUTE, (void*)&permitJoin, outBuff);

	/*此处插入上报函数*/
	messageReportFun(outBuff);

}

int init_gpio_app(void* time)
{
	int period;
	period= *(int *)time;
    if (0 != init_gpio_dev())
    {
        ERROR("[ZIGBEE] inital gpio_fun failed\n\n");
        return -1;
    }
    else
    {
    	ledFlag=1;
        sys_start_run_led(period);
    }

    close_gpio_dev();
    return 0;
}


void set_led_ctrl_flag(int num)
{
	ledFlag=num;
}

int get_led_ctrl_flag()
{
	return ledFlag;
}

void sys_coo_run_led(void)
{
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_pin_value(40,0);
#endif
	
    while(1)
    {
#ifdef BOARD_WHITE_BOX_GATEWAY	
		//wan(39)  lan(40) wifi (44)
        mt76x8_gpio_set_pin_value(39,1);
        //usleep(200000);
        mt76x8_gpio_set_pin_value(40,1);
        //usleep(200000);
        mt76x8_gpio_set_pin_value(44,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(39,0);
        //usleep(200000);
        mt76x8_gpio_set_pin_value(40,0);
        //usleep(200000);
        mt76x8_gpio_set_pin_value(44,0);
        usleep(200000);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
		//wan(39)  lan(40) wifi (44)
        mt76x8_gpio_set_pin_value(39,1);
        //usleep(200000);
        mt76x8_gpio_set_pin_value(40,1);
        //usleep(200000);
        mt76x8_gpio_set_pin_value(44,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(39,0);
        //usleep(200000);
        mt76x8_gpio_set_pin_value(40,0);
        //usleep(200000);
        mt76x8_gpio_set_pin_value(44,0);
        usleep(200000);
#endif
#ifdef BOARD_U86_GATEWAY
		 mt76x8_gpio_set_pin_value(40,1);
        usleep(200000);
        mt76x8_gpio_set_pin_value(40,0);
        usleep(200000);
#endif
        
        if (cooLedFlag==0)
        {
        	break;
        }
    }
    //结束
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_mode_led(40);
#endif
	close_gpio_dev();
}


void init_gpio_coo_update(void)
{
    if (0 != init_gpio_dev())
    {
        ERROR("[ZIGBEE] inital gpio_fun failed\n\n");
        return ;
    }
    else
    {
    	cooLedFlag=1;
    	sys_coo_run_led();
    }
}

void closeCooLed()
{

	cooLedFlag=0;
}


void sys_refactory_run_led()
{
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_pin_value(40,0);
#endif

	int i=0;
	for(i=0;i<10;i++)
	{
#ifdef BOARD_WHITE_BOX_GATEWAY	
		mt76x8_gpio_set_pin_value(39,1);
        mt76x8_gpio_set_pin_value(40,1);
        mt76x8_gpio_set_pin_value(44,1);
        usleep(100000);
        mt76x8_gpio_set_pin_value(39,0);
        mt76x8_gpio_set_pin_value(40,0);
        mt76x8_gpio_set_pin_value(44,0);
        usleep(100000);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
		mt76x8_gpio_set_pin_value(39,1);
        mt76x8_gpio_set_pin_value(40,1);
        mt76x8_gpio_set_pin_value(44,1);
        usleep(100000);
        mt76x8_gpio_set_pin_value(39,0);
        mt76x8_gpio_set_pin_value(40,0);
        mt76x8_gpio_set_pin_value(44,0);
        usleep(100000);
#endif
#ifdef BOARD_U86_GATEWAY
		mt76x8_gpio_set_pin_value(40,1);
        usleep(100000);
        mt76x8_gpio_set_pin_value(40,0);
        usleep(100000);
#endif	
        

	 }
	    //结束
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_mode_led(40);
#endif
	close_gpio_dev();
}

void init_gpio_refactory(void)
{
    if (0 != init_gpio_dev())
    {
        ERROR("[ZIGBEE] inital gpio_fun failed\n\n");
        return ;
    }
    else
    {
    	sys_refactory_run_led();
    }
}


void resetLed()
{
	init_gpio_dev();
	close_gpio_dev();
#ifdef BOARD_U86_GATEWAY
	init_gpio_run();
#endif
}


void sys_8s_butten_run_led()
{
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_pin_value(40,0);
#endif

	int i=0;
	for(i=0;i<2;i++)
	{
#ifdef BOARD_WHITE_BOX_GATEWAY	
		mt76x8_gpio_set_pin_value(39,1);
        mt76x8_gpio_set_pin_value(40,1);
        mt76x8_gpio_set_pin_value(44,1);
        usleep(100000);
        mt76x8_gpio_set_pin_value(39,0);
        mt76x8_gpio_set_pin_value(40,0);
        mt76x8_gpio_set_pin_value(44,0);
        usleep(100000);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
		mt76x8_gpio_set_pin_value(39,1);
        mt76x8_gpio_set_pin_value(40,1);
        mt76x8_gpio_set_pin_value(44,1);
        usleep(100000);
        mt76x8_gpio_set_pin_value(39,0);
        mt76x8_gpio_set_pin_value(40,0);
        mt76x8_gpio_set_pin_value(44,0);
        usleep(100000);
#endif
#ifdef BOARD_U86_GATEWAY
		mt76x8_gpio_set_pin_value(40,1);
        usleep(100000);
        mt76x8_gpio_set_pin_value(40,0);
        usleep(100000);
#endif
        

	 }
    //结束
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_mode_led(40);
#endif

	close_gpio_dev();
}

void init_gpio_8s_butten(void)
{
    if (0 != init_gpio_dev())
    {
    	ERROR("[ZIGBEE] inital gpio_fun failed\n\n");
        return ;
    }
    else
    {
    	sys_8s_butten_run_led();
    }
}

static int wan_lan;
void sys_wan_lan_run_led()
{
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_pin_value(39,0);
	mt76x8_gpio_set_pin_value(40,0);
	mt76x8_gpio_set_pin_value(44,0);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_pin_value(40,0);
#endif
	
	while(1)
	{
#ifdef BOARD_WHITE_BOX_GATEWAY	
		mt76x8_gpio_set_pin_value(39,1);
		mt76x8_gpio_set_pin_value(40,1);
		mt76x8_gpio_set_pin_value(44,1);
		usleep(200000);
		mt76x8_gpio_set_pin_value(39,0);
		mt76x8_gpio_set_pin_value(40,0);
		mt76x8_gpio_set_pin_value(44,0);
		usleep(100000);
		mt76x8_gpio_set_pin_value(39,1);
		mt76x8_gpio_set_pin_value(40,1);
		mt76x8_gpio_set_pin_value(44,1);
		usleep(200000);
		mt76x8_gpio_set_pin_value(39,0);
		mt76x8_gpio_set_pin_value(40,0);
		mt76x8_gpio_set_pin_value(44,0);
		usleep(500000);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
		mt76x8_gpio_set_pin_value(39,1);
		mt76x8_gpio_set_pin_value(40,1);
		mt76x8_gpio_set_pin_value(44,1);
		usleep(200000);
		mt76x8_gpio_set_pin_value(39,0);
		mt76x8_gpio_set_pin_value(40,0);
		mt76x8_gpio_set_pin_value(44,0);
		usleep(100000);
		mt76x8_gpio_set_pin_value(39,1);
		mt76x8_gpio_set_pin_value(40,1);
		mt76x8_gpio_set_pin_value(44,1);
		usleep(200000);
		mt76x8_gpio_set_pin_value(39,0);
		mt76x8_gpio_set_pin_value(40,0);
		mt76x8_gpio_set_pin_value(44,0);
		usleep(500000);
#endif
#ifdef BOARD_U86_GATEWAY
		mt76x8_gpio_set_pin_value(40,1);
		usleep(200000);
		mt76x8_gpio_set_pin_value(40,0);
		usleep(100000);
		mt76x8_gpio_set_pin_value(40,1);
		usleep(200000);
		mt76x8_gpio_set_pin_value(40,0);
		usleep(500000);
#endif
		if( wan_lan==0)
			break;
	 }
	//结束
#ifdef BOARD_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY	
	mt76x8_gpio_set_mode_led(39);
    mt76x8_gpio_set_mode_led(40);
    mt76x8_gpio_set_mode_led(44);
#endif
#ifdef BOARD_U86_GATEWAY
	mt76x8_gpio_set_mode_led(40);
#endif
	close_gpio_dev();
}

void init_gpio_wan_wifi_lan(void)
{
	 if (0 != init_gpio_dev())
	 {
		  ERROR("[ZIGBEE] inital gpio_fun failed\n\n");
	      return ;
	 }
	 else
	 {
		 wan_lan=1;
		 sys_wan_lan_run_led();
	 }
}

void set_wan_lan_wifi_flag(int num)
{
	wan_lan=num;
}

#ifdef BOARD_U86_GATEWAY

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <poll.h>

#include "pthread_tool.h"

extern int resetButtenProcess(char *event);

pthread_t pid_gpio_key_exec = 0;
pthread_t pid_gpio_led_exec = 0;


static int zigbeeLedFlag=0;


static void*
gpio_key_exec(void *arg)
{
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
			if('1' == value)
			{
				/*抬起*/
				resetButtenProcess("{\"GatewayKey\":\"0\"}");

			}
			else if('0' == value)
			{
				/*按下*/
				resetButtenProcess("{\"GatewayKey\":\"1\"}");
			}
		}
		else if(pfd.revents & POLLERR) 
		{
			printf("poll error\n");
		}
		
	}

	close(fd);

	return NULL;
}
static void*
gpio_led_exec(void *arg)
{
	while(1)
	{
		if(0 == zigbeeLedFlag)
		{
			mt76x8_gpio_set_pin_value(43,0);
			usleep(1000000);
		}
		else if(1 == zigbeeLedFlag)
		{
			mt76x8_gpio_set_pin_value(43,1);
			usleep(1000000);
		}
		else if(2 == zigbeeLedFlag)
		{
			mt76x8_gpio_set_pin_value(43,1);
			usleep(1000000);
			mt76x8_gpio_set_pin_value(43,0);
			usleep(1000000);
		}
	}
	return NULL;
}

void set_zigbee_led_flag(int num)
{
	zigbeeLedFlag=num;
}

/*gpio控制*/
void init_gpio_run(void)
{
    if (0 != init_gpio_dev())
    {
        ERROR("[ZIGBEE] inital gpio_fun failed\n\n");
        return ;
    }
	
	/*初始化按键gpio 42*/
	base_system("echo \"42\" > /sys/class/gpio/export", NULL, 0);
	base_system("echo \"in\" > /sys/class/gpio/gpio42/direction", NULL, 0);
	base_system("echo \"both\" > /sys/class/gpio/gpio42/edge", NULL, 0);
	
	/*创建线程*/
	if(0 != pthread_detached_create(
		&pid_gpio_key_exec, 
		gpio_key_exec, 
		NULL)
	)
	{
		printf("pthread_attr_init Error.\n");
        return;
	}
	/*创建线程*/
	if(0 != pthread_detached_create(
		&pid_gpio_led_exec, 
		gpio_led_exec, 
		NULL)
	)
	{
		printf("pthread_attr_init Error.\n");
        return;
	}
}

#endif





