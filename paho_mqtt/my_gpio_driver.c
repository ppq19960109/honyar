#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "my_gpio_driver.h"

#ifdef BOARD_WHITE_BOX_GATEWAY

#endif
#ifdef BOARD_NEW_WHITE_BOX_GATEWAY

#endif
#ifdef BOARD_U86_GATEWAY
extern int init_gpio_dev(void)
{
	printf("[Debug] #### init_gpio_dev\n");
	return 0;
}


extern int mt76x8_gpio_set_mode_led(int pin)
{
	FILE *fp;
	char acPatch[128] = {0};
	char acData[2] = "0";
	switch(pin)
	{
		case 40:/*system*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:system/brightness");
			break;
		case 43:/*zled*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:zled/brightness");
			break;
		case 44:/*wlan*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:wled/brightness");
			break;
	}
	
	fp = fopen(acPatch, "w");
	if(NULL == fp)
	{
		return -1;
	}
	fwrite(acData, 2, 1, fp);
	
	fclose(fp);
	
	return 0;
}



extern int mt76x8_gpio_get_pin(int pin)
{
	FILE *fp;
	char acPatch[128] = {0};
	char acData[2] = {0};
	switch(pin)
	{
		case 40:/*system*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:system/brightness");
			break;
		case 43:/*zled*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:zled/brightness");
			break;
		case 44:/*wlan*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:wled/brightness");
			break;
	}
	
	fp = fopen(acPatch, "r");
	if(NULL == fp)
	{
		return -1;
	}
	fread(acData, 2, 1, fp);
	
	fclose(fp);
	
	return atoi(acData);
}

extern int mt76x8_gpio_set_pin_value(int pin, int value)
{
	FILE *fp;
	char acPatch[128] = {0};
	char acData[2] = {0};
	switch(pin)
	{
		case 40:/*system*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:system/brightness");
			break;
		case 43:/*zled*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:zled/brightness");
			break;
		case 44:/*wlan*/
			snprintf(acPatch, 128, "/sys/class/leds/wrtnode2r:red:wled/brightness");
			break;
	}
	
	snprintf(acData, 2, "%d", value);
	fp = fopen(acPatch, "w");
	if(NULL == fp)
	{
		return -1;
	}
	fwrite(acData, 2, 1, fp);
	
	fclose(fp);
	
	return 0;
}

extern void close_gpio_dev(void)
{

	mt76x8_gpio_set_pin_value(40, 0);
	mt76x8_gpio_set_pin_value(43, 0);
	
	return;
}
#endif







