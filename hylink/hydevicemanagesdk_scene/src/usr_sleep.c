/*
 * usr_sleep.c
 *
 *  Created on: Jun 11, 2020
 *      Author: jerry
 */
#include <sys/time.h>
#include <unistd.h>
#include <errno.h>

void select_sleep(int i)
{
	struct timeval timeout;
	int err;
	timeout.tv_sec=i;
	timeout.tv_usec=0;

	do
	{
		err=select(0,NULL,NULL,NULL,&timeout);
	}while(err<0 && errno==EINTR);


}

void select_mssleep(int i)
{
	struct timeval timeout;
	int err;
	timeout.tv_sec=0;
	timeout.tv_usec=i*1000;

	do
	{
		err=select(0,NULL,NULL,NULL,&timeout);
	}while(err<0 && errno==EINTR);


}
