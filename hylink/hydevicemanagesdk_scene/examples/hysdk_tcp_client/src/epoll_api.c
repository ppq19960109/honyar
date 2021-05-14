/***********************************************************
*文件名     : epoll_api.c
*版   本   : v1.0.0.0
*日   期   : 2018.09.14
*说   明   : epoll事件监听
*修改记录: 
************************************************************/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "epoll_api.h"
#include "log.h"
#include "error_no.h"
#include "len.h"
#include "param_check.h"

/*初始化epoll监听事件(默认监听读事件)*/
int 
epoll_init_event(epoll_fd_t *_this, int iEventfd)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_ERRORNO_1(iEventfd);
	
	struct epoll_event stEpollEvent;
	memset(&stEpollEvent, 0x0, sizeof(struct epoll_event));
	stEpollEvent.events = EPOLLIN | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	
	if(epoll_ctl(_this->iEpollFd, EPOLL_CTL_ADD, iEventfd, &stEpollEvent) < 0)
	{
		ERROR("Init epoll event error, %s\n",  strerror(errno));
		return ERROR_SYSTEM;
	}
	return ERROR_SUCCESS;
}

/*epoll监听读事件*/
int 
epoll_read_event(epoll_fd_t *_this, int iEventfd)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_ERRORNO_1(iEventfd);
	
	struct epoll_event stEpollEvent;
	memset(&stEpollEvent, 0x0, sizeof(struct epoll_event));
	stEpollEvent.events = EPOLLIN | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	if(epoll_ctl(_this->iEpollFd, EPOLL_CTL_MOD, iEventfd, &stEpollEvent) < 0)
	{
		ERROR("Read epoll event error, %s\n",  strerror(errno));
		return ERROR_SYSTEM;
	}
	return ERROR_SUCCESS;
}
/*epoll监听写事件*/
int 
epoll_write_event(epoll_fd_t *_this, int iEventfd)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_ERRORNO_1(iEventfd);
	
	struct epoll_event stEpollEvent;
	memset(&stEpollEvent, 0x0, sizeof(struct epoll_event));
	stEpollEvent.events = EPOLLOUT | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	if(epoll_ctl(_this->iEpollFd, EPOLL_CTL_MOD, iEventfd, &stEpollEvent) < 0)
	{
		ERROR("Write epoll event error, %s\n",  strerror(errno));
		return ERROR_SYSTEM;
	}
	return ERROR_SUCCESS;
}
/*epoll监听读写事件*/
int 
epoll_read_write_event(epoll_fd_t *_this, int iEventfd)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_ERRORNO_1(iEventfd);
	
	struct epoll_event stEpollEvent;
	memset(&stEpollEvent, 0x0, sizeof(struct epoll_event));
	stEpollEvent.events = EPOLLIN | EPOLLOUT | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	if(epoll_ctl(_this->iEpollFd, EPOLL_CTL_MOD, iEventfd, &stEpollEvent) < 0)
	{
		ERROR("Read and write epoll event error, %s\n",  strerror(errno));
		return ERROR_SYSTEM;
	}
	return ERROR_SUCCESS;
}
/*删除epoll监听事件*/
int 
epoll_del_event(epoll_fd_t *_this, int iEventfd)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_ERRORNO_1(iEventfd);
	
	struct epoll_event stEpollEvent;
	memset(&stEpollEvent, 0x0, sizeof(struct epoll_event));
	stEpollEvent.events = EPOLLIN | EPOLLET;
	stEpollEvent.data.fd = iEventfd;
	if(epoll_ctl(_this->iEpollFd, EPOLL_CTL_DEL, iEventfd, &stEpollEvent) < 0)
	{
		ERROR("Del epoll event error, %s\n",  strerror(errno));
		return ERROR_SYSTEM;
	}
	return ERROR_SUCCESS;
}

int
epoll_event_happen(epoll_fd_t *_this, epoll_event_t *paEpollEvents, int *piCount, int iTimeOutMs)
{
	PARAM_CHECK_RETURN_ERRORNO_3(_this, paEpollEvents, piCount);
	
	int i = 0;
	int iEventsCount = 0;
	int iEventsCountMax = *piCount;
	iEventsCount = 
		epoll_wait(
			_this->iEpollFd, 
			_this->pastWaitevents,
			_this->iEpollEventMaxNum,
			iTimeOutMs
		);
	for(i = 0; i < iEventsCount || i < iEventsCountMax; ++i)
	{
		paEpollEvents[i].iFd = _this->pastWaitevents[i].data.fd;
		paEpollEvents[i].uiEvent = _this->pastWaitevents[i].events;
	}

	*piCount = iEventsCount > iEventsCountMax ? iEventsCountMax : iEventsCount;

	return *piCount;
}

/*构造函数*/
epoll_fd_t *new_epoll_fd(int iEpollEventMaxNum)
{	
	PARAM_CHECK_POSITIVE_NUMBER_RETURN_NULL_1(iEpollEventMaxNum);
	
	epoll_fd_t* pstEpollFd = 
		(epoll_fd_t *)calloc(1, sizeof(epoll_fd_t));
	if(NULL == pstEpollFd)
	{
		FATAL("Malloc error: %s\n", strerror(errno));
		return NULL;
	}

	pstEpollFd->iEpollEventMaxNum = iEpollEventMaxNum;
	pstEpollFd->pastWaitevents = 
		(struct epoll_event *)calloc(pstEpollFd->iEpollEventMaxNum, sizeof(struct epoll_event));
	if(NULL == pstEpollFd->pastWaitevents)
	{
		FATAL("Malloc error: %s\n", strerror(errno));
		return NULL;
	}
	
	if((pstEpollFd->iEpollFd = epoll_create(iEpollEventMaxNum)) < 0)
	{  
		FATAL("Epoll create error, %s.",strerror(errno));
		return NULL;  
    }
	
	pstEpollFd->init_event = epoll_init_event;
	pstEpollFd->read_event = epoll_read_event;
	pstEpollFd->write_event = epoll_write_event;
	pstEpollFd->read_write_event = epoll_read_write_event;
	pstEpollFd->destroy_event = epoll_del_event;
	pstEpollFd->happen_event = epoll_event_happen;
	
	return pstEpollFd;
}

/*析构函数*/
int destroy_epoll_fd(epoll_fd_t *_this)
{
	PARAM_CHECK_RETURN_ERRORNO_1(_this);
	if(_this->iEpollFd)
	{
		close(_this->iEpollFd);
	}

	if(_this->pastWaitevents)
	{
		free(_this->pastWaitevents);
	}

	free(_this);
	_this = NULL;

	return ERROR_SUCCESS;
}


