/***********************************************************
*文件名     : scene_sync.c
*版   本   : v1.0.0.0
*日   期   : 2021.04.03
*说   明   : 本地场景之状态同步
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <sys/time.h>   

#include "log.h"
#include "len.h"
#include "cJSON.h"
#include "json_key.h"

#include "link_list.h"
#include "scene_sync.h"
#include "hy_device_manage.h"

link_list_class_t *g_pstSceneSyncEventList = NULL;

/*同设备同键值最大事件数*/
#define EVENT_MAX_NUM						32


/*事件失效事件, 单位s*/
#define SYNC_EVENT_LOSE_TIME						30

/*事件状态同步处理安全时间,单位s*/
#define SYNC_EVENT_SAFE_TIME						2
/*保护机制下，事件下发延迟事件,单位s*/
#define SYNC_EVENT_DELAY_TIME						1
/*保护机制, 触发事件数*/
#define SYNC_EVENT_SAFE_NUM							4


/*事件*/
typedef struct scene_sync_node_s
{
	link_list_piece_t stLinkPiece;
	/*设备id*/
	char acDevId[DEV_ID_MAX_LEN];
	/*事件对象*/
	char acKey[KEY_MAX_LEN];
	
	/*事件值队列,事件*/
	int iValueTotal;
	int aiValue[EVENT_MAX_NUM];
	/*事件下发标志*/
	unsigned int uiSendFlag;
	
	/*触发保护机制*/
	int iProtect;
	/*在保护机制检测周期内事件累计*/
	int iCount;
	
	
	
	/*活跃时间戳*/
	struct timeval stActiveTime;
	
}scene_sync_node_t;

/*事件发送标志位操作接口*/
static int _scene_sync_event_send_flag_get(
		unsigned int uiSendFlag,
		int iEventId)
{
	return (uiSendFlag >> iEventId) & 0x1;
}
static void _scene_sync_event_send_flag_set(
		unsigned int *puiSendFlag,
		int iEventId)
{
	*puiSendFlag = *puiSendFlag | (0x1 << iEventId);
}
static void _scene_sync_event_send_flag_clean(
		unsigned int *puiSendFlag,
		int iEventId)
{
	*puiSendFlag = *puiSendFlag & (!(0x1 << iEventId));
}

/*添加状态同步控制指令*/
/*
*1、下发指令。
*2、最后一次下发事件后，事件失效时间内，未收到新请求，
*	则该事件失效，失效的事件将被删除。
*3、安全时间时间内，事件请求多次，则触发保护机制.
*	保护机制内,所有指令将被延迟一定时间发送,
*	在延迟时间内,收到新的请求将替换已有请求。
*	防止重复处理不必要的请求。
*
*/

int 
scene_sync_event_req_add(
	char *pcDevId, 
	char *pcKey, 
	char *pcValue
)
{
	if(NULL == g_pstSceneSyncEventList)
	{
		g_pstSceneSyncEventList = 
			new_link_list(sizeof(scene_sync_node_t));
		if(NULL == g_pstSceneSyncEventList)
		{
			return -1;
		}
	}

	int iRet = 0;
	struct timeval stNow;
	unsigned char ucFoundFlag = 0;

	/*获取当前时间*/
	gettimeofday(&stNow, NULL);

	/*加锁*/
	g_pstSceneSyncEventList->write_lock(g_pstSceneSyncEventList);
	
	/*遍历所有事件*/
	scene_sync_node_t *pstPtr = 
		(scene_sync_node_t *)(
			g_pstSceneSyncEventList->next(
				g_pstSceneSyncEventList,
				NULL
			)
		);
	if(NULL == pstPtr)
	{
		/*事件列表为空，直接添加事件*/
		DEBUG("The event list is empty.\n");
		iRet = 0;
		goto scene_sync_event_req_add_exec;
	}
	scene_sync_node_t *pstHead = 
		(scene_sync_node_t *)(
			g_pstSceneSyncEventList->head(
				g_pstSceneSyncEventList
			)
		);
	
	if(NULL == pstHead)
	{
		/*事件列表为空，直接添加事件*/
		DEBUG("The event list is empty.\n");
		iRet = 0;
		goto scene_sync_event_req_add_exec;
	}

	
	DEBUG("Count = %d\n", g_pstSceneSyncEventList->iNodeNum);
	scene_sync_node_t *pstDel = NULL;
	while(pstHead != pstPtr)
	{
		DEBUG("Now = %d, Active = %d\n", stNow.tv_sec, pstPtr->stActiveTime.tv_sec);
		
		/*查找事件*/
		if(!strncmp(pstPtr->acDevId, pcDevId, DEV_ID_MAX_LEN) &&
		!strncmp(pstPtr->acKey, pcKey, KEY_MAX_LEN))
		{
			/*查找到事件*/
			DEBUG("Find the event.\n");
			ucFoundFlag = 1;
			
			if(stNow.tv_sec - pstPtr->stActiveTime.tv_sec < 
				SYNC_EVENT_SAFE_TIME)
			{
				pstPtr->iCount ++;
				DEBUG("Protection iCount = %d.\n", pstPtr->iCount);
			}
			else
			{
				pstPtr->iCount = 0;
			}
			
			if(0 == pstPtr->iProtect && 
				pstPtr->iCount >= SYNC_EVENT_SAFE_NUM)
			{
				DEBUG("Protection mode on.\n");
				pstPtr->iProtect = 1;
			}
				
			if(1 == pstPtr->iProtect)
			{
				/*保护机制已触发*/
				/*删除所有未发送的指令*/
				int i = 0;
				int iValueTotalTmp = 0;
				int aiValueTmp[EVENT_MAX_NUM] = {0};
				unsigned int uiSendFlagTmp = 0;
				for(i = 0; i < pstPtr->iValueTotal; i ++)
				{
					if(1 == 
						_scene_sync_event_send_flag_get(
							pstPtr->uiSendFlag, i)
					)
					{
						/*已发送的指令将保留*/
						aiValueTmp[iValueTotalTmp] = 
						pstPtr->aiValue[i];
						_scene_sync_event_send_flag_set(
							&uiSendFlagTmp, iValueTotalTmp);
						iValueTotalTmp ++;
					}
				}
				pstPtr->iValueTotal = iValueTotalTmp;
				memcpy(pstPtr->aiValue, 
					aiValueTmp, sizeof(aiValueTmp));
				pstPtr->uiSendFlag = uiSendFlagTmp;
				
				/*添加当前指令*/
				pstPtr->aiValue[pstPtr->iValueTotal] = 
					atoi(pcValue);
				_scene_sync_event_send_flag_clean(&pstPtr->uiSendFlag, pstPtr->iValueTotal);
				pstPtr->iValueTotal ++;

				
				DEBUG("scene_sync_event_req_add: %s-%s: %d\n", 
					pstPtr->acDevId, pstPtr->acKey, pstPtr->aiValue[pstPtr->iValueTotal - 1]);
				DEBUG("event total: %d\n", pstPtr->iValueTotal);
				DEBUG("event uiSendFlag: %x\n", pstPtr->uiSendFlag);
			}
			else
			{
				/*保护机制未开启,添加当前指令*/
				pstPtr->aiValue[pstPtr->iValueTotal] =
					atoi(pcValue);
				_scene_sync_event_send_flag_clean(&pstPtr->uiSendFlag, pstPtr->iValueTotal);
				pstPtr->iValueTotal ++;

				
				DEBUG("scene_sync_event_req_add: %s-%s: %d\n", 
					pstPtr->acDevId, pstPtr->acKey, pstPtr->aiValue[pstPtr->iValueTotal - 1]);
				DEBUG("event total: %d\n", pstPtr->iValueTotal);
				DEBUG("event uiSendFlag: %x\n", pstPtr->uiSendFlag);
			}

			/*更新活跃时间*/
			memcpy(
				&pstPtr->stActiveTime,
				&stNow, 
				sizeof(struct timeval)
			);
		}
		pstPtr = 
			(scene_sync_node_t *)(
				g_pstSceneSyncEventList->next(
					g_pstSceneSyncEventList, 
					(link_list_piece_t *)pstPtr
				)
			);
	}
scene_sync_event_req_add_exec:	
	/*未找到事件*/
	if(0 == ucFoundFlag)
	{
		/*记录事件*/
		pstPtr = 
			(scene_sync_node_t *)(
				g_pstSceneSyncEventList->new_node(
					g_pstSceneSyncEventList
				)
			);
		if(NULL == pstPtr)
		{
			iRet = -1;
			goto scene_sync_event_req_add_end;
		}
		strncpy(pstPtr->acDevId, pcDevId, DEV_ID_MAX_LEN);
		strncpy(pstPtr->acKey, pcKey, KEY_MAX_LEN);

		pstPtr->aiValue[0] = atoi(pcValue);
		pstPtr->iValueTotal = 1;
		
		pstPtr->iCount = 1;

		memcpy(
			&pstPtr->stActiveTime,
			&stNow, 
			sizeof(struct timeval)
		);
		DEBUG("scene_sync_event_req_add: %s-%s: %d\n", 
			pstPtr->acDevId, pstPtr->acKey, pstPtr->aiValue[0]);
		DEBUG("event total: %d\n", pstPtr->iValueTotal);
		iRet = 
			g_pstSceneSyncEventList->inst_tail(
				g_pstSceneSyncEventList,
				(link_list_piece_t *)pstPtr);
		if(0 != iRet)
		{
			ERROR("Sync List inst failed.\n");
			g_pstSceneSyncEventList->destroy_node(
				g_pstSceneSyncEventList, 
				(link_list_piece_t *)pstPtr);
			iRet = -1;
			goto scene_sync_event_req_add_end;
		}
	}
scene_sync_event_req_add_end:

	/*解锁*/
	g_pstSceneSyncEventList->write_unlock(g_pstSceneSyncEventList);

	return 0;
}
/*状态同步指令下发*/
/*
*
*
*
*
*/
static int 
_scene_sync_ctrl_dispatch(
	char *pcDevId, 
	char *pcKey, 
	char *pcValue
)
{
	if(NULL == g_pstSceneSyncEventList)
	{
		g_pstSceneSyncEventList = 
			new_link_list(sizeof(scene_sync_node_t));
		if(NULL == g_pstSceneSyncEventList)
		{
			return -1;
		}
	}

	/*获取*/
	
	int iRet = 0;
	char *pstr = NULL;
	cJSON *pJson = NULL;
	cJSON *pData = NULL;
	cJSON *pDataItem = NULL;
	char acBuff[BUFF_MAX_LEN] = {0};
	
	/*发送指令*/
	/*
	*{
		"Command":"Dispatch",
		"FrameNumber":"00",
		"Type":"Ctrl",
		"Data":[
			{
				"DeviceId":"123456787654310",
				"Key":"Switch",
				"Value":"1"
			}
		]
	}
	*/
	pJson = cJSON_CreateObject();
	cJSON_AddStringToObject(pJson, JSON_KEY_COMMAND, JSON_VALUE_COMMAND_DISPATCH);
	cJSON_AddStringToObject(pJson, JSON_KEY_FRAMENUMBER, JSON_VALUE_FRAMENUMBER_00);
	cJSON_AddStringToObject(pJson, JSON_KEY_TYPE, JSON_VALUE_TYPE_CTRL);
	pData = cJSON_CreateArray();
	pDataItem = cJSON_CreateObject();
	cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_DAVICE_ID, pcDevId);
	cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_KEY, pcKey);
	cJSON_AddStringToObject(pDataItem, JSON_KEY_DATA_VALUE, pcValue);
	cJSON_AddItemToArray(pData, pDataItem);
	cJSON_AddItemToObject(pJson, JSON_KEY_DATA, pData);

	pstr = cJSON_Print(pJson);
	strncpy(acBuff, pstr, strlen(pstr));
	free(pstr);
	
	cJSON_Minify(acBuff);
	cJSON_Delete(pJson);
	DEBUG("Sync exec: %s\n", acBuff);
	iRet = hyMsgProcess_AsynQuery(acBuff,BUFF_MAX_LEN);


	return iRet;
}

/*状态同步指令执行*/
/*
*1、通过该接口获取的指令，内部默认自动标记为已发送，
*	所以获取出的指令，必须调用scene_sync_ctrl_dispatch接口发送。
*
*
*
*/

int 
scene_sync_event_req_exec(void)
{
	if(NULL == g_pstSceneSyncEventList)
	{
		g_pstSceneSyncEventList = 
			new_link_list(sizeof(scene_sync_node_t));
		if(NULL == g_pstSceneSyncEventList)
		{
			return -1;
		}
		else
		{
			return 0;
		}
	}
	
	int iRet = 0;
	struct timeval stNow;
	
	/*加锁*/
	g_pstSceneSyncEventList->write_lock(g_pstSceneSyncEventList);
	
	/*遍历同步事件*/
	scene_sync_node_t *pstPtr = 
		(scene_sync_node_t *)(
			g_pstSceneSyncEventList->next(
				g_pstSceneSyncEventList,
				NULL
			)
		);
	if(NULL == pstPtr)
	{
		iRet = 0;
		goto scene_sync_event_req_exec_end;
	}
	scene_sync_node_t *pstHead = 
		(scene_sync_node_t *)(
			g_pstSceneSyncEventList->head(
				g_pstSceneSyncEventList
			)
		);
	
	if(NULL == pstHead)
	{
		iRet = 0;
		goto scene_sync_event_req_exec_end;
	}
	
	/*获取当前时间*/
	gettimeofday(&stNow, NULL);
	
	DEBUG("Count = %d\n", g_pstSceneSyncEventList->iNodeNum);
	scene_sync_node_t *pstDel = NULL;
	while(pstHead != pstPtr)
	{
		DEBUG("iValueTotal = %d, uiSendFlag = %x\n", pstPtr->iValueTotal, pstPtr->uiSendFlag);
		if(NULL != pstDel)
		{
			/*删除*/
			g_pstSceneSyncEventList->del(
				g_pstSceneSyncEventList, 
				(link_list_piece_t *)pstDel
			);
			pstDel = NULL;
		}
		/*判断事件数是否为空*/
		if(0 == pstPtr->iValueTotal)
		{
			/*删除*/
			pstDel = pstPtr;

			pstPtr = 
			(scene_sync_node_t *)(
				g_pstSceneSyncEventList->next(
					g_pstSceneSyncEventList, 
					(link_list_piece_t *)pstPtr
				)
			);

			continue;
		}
		
		/*判断事件是否失效*/
		if(stNow.tv_sec - pstPtr->stActiveTime.tv_sec >= 
			SYNC_EVENT_LOSE_TIME)
		{
			/*删除*/
			pstDel = pstPtr;

			pstPtr = 
			(scene_sync_node_t *)(
				g_pstSceneSyncEventList->next(
					g_pstSceneSyncEventList, 
					(link_list_piece_t *)pstPtr
				)
			);

			continue;
		}
		
		if(1 == pstPtr->iProtect)
		{
			/*事件已经有一段时间不活跃了，则解除保护机制*/
			if(stNow.tv_sec - pstPtr->stActiveTime.tv_sec >= 
				SYNC_EVENT_SAFE_TIME)
			{
				pstPtr->iProtect = 0;
				pstPtr->iCount = 0;
			}
			/*处于保护机制下，延迟一定时间再下发*/
			if(stNow.tv_sec - pstPtr->stActiveTime.tv_sec < 
				SYNC_EVENT_DELAY_TIME)
			{
				DEBUG("Delay send. Now = %d, Active = %d\n", stNow.tv_sec, pstPtr->stActiveTime.tv_sec);
				
				pstPtr = 
					(scene_sync_node_t *)(
						g_pstSceneSyncEventList->next(
							g_pstSceneSyncEventList, 
							(link_list_piece_t *)pstPtr
						)
					);
				
				continue;
			}
		}
		
		/*遍历*/
		int i = 0;
		for(i = 0; i < pstPtr->iValueTotal; i ++)
		{
			if(0 == 
				_scene_sync_event_send_flag_get(pstPtr->uiSendFlag, i)
			)
			{
				char acValue[VALUE_MAX_LEN] = {0};
				snprintf(acValue,
					VALUE_MAX_LEN,
					"%d",
					pstPtr->aiValue[i]);
				iRet = _scene_sync_ctrl_dispatch(
					pstPtr->acDevId, 
					pstPtr->acKey, 
					acValue
				);
				if(0 == iRet)
				{
					_scene_sync_event_send_flag_set(
						&pstPtr->uiSendFlag,
						i);
				}
			}
		}

		pstPtr = 
			(scene_sync_node_t *)(
				g_pstSceneSyncEventList->next(
					g_pstSceneSyncEventList, 
					(link_list_piece_t *)pstPtr
				)
			);
	}

	if(NULL != pstDel)
	{
		/*删除*/
		g_pstSceneSyncEventList->del(
			g_pstSceneSyncEventList, 
			(link_list_piece_t *)pstDel
		);
		pstDel = NULL;
	}
scene_sync_event_req_exec_end:

	/*解锁*/
	g_pstSceneSyncEventList->write_unlock(g_pstSceneSyncEventList);

	return iRet;
}
/*同步控制范围内的属性上报事件，判断其为主动上报还是控制回应*/
/*
*1、返回值为0, 代表该上报事件为首次上报. 
*	此时后续需要触发相关状态同步处理
*2、返回值为1, 代表该上报事件为请求事件的回应.
*	此时无需触发后续处理。
*
*
*/

int 
scene_sync_event(
	char *pcDevId, 
	char *pcKey, 
	char *pcValue
)
{
	if(NULL == g_pstSceneSyncEventList)
	{
		g_pstSceneSyncEventList = 
			new_link_list(sizeof(scene_sync_node_t));
		if(NULL == g_pstSceneSyncEventList)
		{
			return -1;
		}
	}

	int iRet = 0;
	struct timeval stNow;
	unsigned char ucFoundFlag = 0;

	/*获取当前时间*/
	gettimeofday(&stNow, NULL);

	/*加锁*/
	g_pstSceneSyncEventList->write_lock(g_pstSceneSyncEventList);
	
	/*遍历所有事件*/
	scene_sync_node_t *pstPtr = 
		(scene_sync_node_t *)(
			g_pstSceneSyncEventList->next(
				g_pstSceneSyncEventList,
				NULL
			)
		);
	if(NULL == pstPtr)
	{
		return -1;
	}
	scene_sync_node_t *pstHead = 
		(scene_sync_node_t *)(
			g_pstSceneSyncEventList->head(
				g_pstSceneSyncEventList
			)
		);
	
	if(NULL == pstHead)
	{
		return -1;
	}
	DEBUG("Count = %d\n", g_pstSceneSyncEventList->iNodeNum);
	scene_sync_node_t *pstDel = NULL;
	while(pstHead != pstPtr)
	{
		/*查找事件*/
		if(!strncmp(pstPtr->acDevId, pcDevId, DEV_ID_MAX_LEN) &&
		!strncmp(pstPtr->acKey, pcKey, KEY_MAX_LEN))
		{
			DEBUG("pstPtr->iValueTotal = %d\n", pstPtr->iValueTotal);
			/*更新活跃时间*/
			memcpy(
				&pstPtr->stActiveTime,
				&stNow, 
				sizeof(struct timeval)
			);
			/*遍历*/
			int i = 0;
			int iValueTotalTmp = 0;
			int aiValueTmp[EVENT_MAX_NUM] = {0};
			unsigned int uiSendFlagTmp = 0;
			for(i = 0; i < pstPtr->iValueTotal; i ++)
			{
				if(1 == _scene_sync_event_send_flag_get(pstPtr->uiSendFlag, i))
				{
					if(0 == ucFoundFlag && 
						pstPtr->aiValue[i] == atoi(pcValue))
					{
						/*匹配到发送事件*/
						ucFoundFlag = 1;
						DEBUG("Found\n");
					}
					else
					{
						DEBUG("Not Found\n");
						aiValueTmp[iValueTotalTmp] = pstPtr->aiValue[i];
						_scene_sync_event_send_flag_set(&uiSendFlagTmp, iValueTotalTmp);
						iValueTotalTmp ++;
					}
				}
				else
				{
					aiValueTmp[iValueTotalTmp] = pstPtr->aiValue[i];
					_scene_sync_event_send_flag_clean(&uiSendFlagTmp, iValueTotalTmp);
					iValueTotalTmp++;
				}
			}
			pstPtr->iValueTotal = iValueTotalTmp;
			DEBUG("pstPtr->iValueTotal = %d\n", pstPtr->iValueTotal);
			memcpy(pstPtr->aiValue, aiValueTmp, sizeof(aiValueTmp));
			pstPtr->uiSendFlag = uiSendFlagTmp;
		}	
		pstPtr = 
			(scene_sync_node_t *)(
				g_pstSceneSyncEventList->next(
					g_pstSceneSyncEventList, 
					(link_list_piece_t *)pstPtr
				)
			);
	}
	
	/*解锁*/
	g_pstSceneSyncEventList->write_unlock(g_pstSceneSyncEventList);
	
	return ucFoundFlag;
}

