/***********************************************************
*文件名    : scene.c
*版   本   : v1.0.0.0
*日   期   : 2018.07.24
*说   明   : 本地化场景协议接口
*
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "param_check.h"
#include "error_no.h"
#include "scene.h"
#include "cJSON.h"
#include "scene_list.h"
#include "condition_list.h"
#include "action_list.h"
#include "len.h"
#include "json_key.h"
#include "db_api.h"
#include "deviceList.h"

/*场景类指针*/
scene_list_class_t* g_pstSceneList = NULL;
int g_pstSceneEventDrivenType;

int 
hyScene_Add(
	cJSON *pstData
)
{
	int iRet = 0;
	int iValueType = 0;
	scene_info_t stScene;
	memset(&stScene, 0x0, sizeof(scene_info_t));
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		stScene.acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	scene_info_t *pstSeneGet =  
		g_pstSceneList->scene_get(g_pstSceneList, stScene.acSceneId);
	
	if(NULL != pstSeneGet)
	{
		DEBUG("hyScene_Set.\n");
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_NAME, 
			stScene.acSceneName, 
			NAME_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)	
		{
			strncpy(stScene.acSceneName, pstSeneGet->acSceneName, NAME_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_ENABLE, 
			stScene.acSceneEnable, 
			STATE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acSceneEnable, pstSeneGet->acSceneEnable, STATE_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_EXEC_DELAYED, 
			stScene.acExecDelayed, 
			DELAYED_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acExecDelayed, pstSeneGet->acExecDelayed, DELAYED_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_UPTIME, 
			stScene.acUpdateTime, 
			TIME_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acUpdateTime, pstSeneGet->acUpdateTime, TIME_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_NOTE, 
			stScene.acNote, 
			NOTE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acNote, pstSeneGet->acNote, NOTE_MAX_LEN);
		}
		iRet = g_pstSceneList->scene_set(
			g_pstSceneList,
			&stScene
		);
		if(ERROR_SUCCESS != iRet)
		{
			ERROR("Scene Info set failed.\n");
			iRet = ERROR_SET;
			goto hyscene_add_end;
		}

	}
	else
	{
		DEBUG("hyScene_Add\n");
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_NAME, 
			stScene.acSceneName, 
			NAME_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acSceneName, "0", STATE_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_ENABLE, 
			stScene.acSceneEnable, 
			STATE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acSceneEnable, "1", STATE_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_EXEC_DELAYED, 
			stScene.acExecDelayed, 
			DELAYED_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acExecDelayed, "0", DELAYED_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_UPTIME, 
			stScene.acUpdateTime, 
			TIME_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acUpdateTime, "0", TIME_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_SCENE_NOTE, 
			stScene.acNote, 
			NOTE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stScene.acNote, "0", NOTE_MAX_LEN);
		}
		scene_info_t *pstScene = 
			g_pstSceneList->scene_add(
				g_pstSceneList,
				&stScene
			);
		if(NULL == pstScene)
		{
			iRet = ERROR_GEN;
			goto hyscene_add_end;
		}
	}
hyscene_add_end:

	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	
	return iRet;
}
int 
hyScene_Del(
	cJSON *pstData
)
{
	DEBUG("hyScene_Del\n");
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	if(!strncmp(acSceneId, JSON_VALUE_ALL, INDEX_MAX_LEN))
	{
		/*清空场景列表*/
		iRet = g_pstSceneList->scene_clear(
			g_pstSceneList,
			1
		);
	}
	else
	{
		/*删除特定场景*/
		iRet = g_pstSceneList->scene_del(
			g_pstSceneList,
			acSceneId 
		);
	}
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	return iRet;
}
int 
hyScene_Get(
	cJSON *pstData, cJSON *pstDataArryOut
)
{
	DEBUG("hyScene_Get\n");
	int i = 0;
	int iCount = SCENE_MAX_NUM;
	scene_info_t *astScene = (scene_info_t *)calloc(SCENE_MAX_NUM, sizeof(scene_info_t));
	if(NULL == astScene)
	{
		ERROR("Calloc error.\n");
		return ERROR_NO_SPACE;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	
	g_pstSceneList->scene_get_list(
		g_pstSceneList, 
		astScene,
		&iCount
	);
	
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	DEBUG("iCount = %d\n", iCount);

	if(0 == iCount)
	{
		cJSON *pstData = cJSON_CreateObject();
		if(NULL == pstData)
		{
			ERROR("New JSON faild.");
			if(astScene)
			{
				free(astScene);
				astScene = NULL;
			}
			return ERROR_GEN;
		}
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_OPERATION, 
			JSON_VALUE_OPERATION_GET_SCENE
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_ID, 
			"none"
		);

		cJSON_AddItemToArray(pstDataArryOut, pstData);
		if(astScene)
		{
			free(astScene);
			astScene = NULL;
		}
		return ERROR_SUCCESS;
	}
	for(i = 0; i < iCount; ++i)
	{
		cJSON *pstData = cJSON_CreateObject();
		if(NULL == pstData)
		{
			ERROR("New JSON faild.");
			if(astScene)
			{
				free(astScene);
				astScene = NULL;
			}
			return ERROR_GEN;
		}
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_OPERATION, 
			JSON_VALUE_OPERATION_GET_SCENE
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_ID, 
			astScene[i].acSceneId
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_NAME, 
			astScene[i].acSceneName
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_ENABLE, 
			astScene[i].acSceneEnable
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_EXEC_DELAYED, 
			astScene[i].acExecDelayed
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_UPTIME, 
			astScene[i].acUpdateTime
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_NOTE, 
			astScene[i].acNote
		);
		cJSON_AddItemToArray(pstDataArryOut, pstData);
	}
	if(astScene)
	{
		free(astScene);
		astScene = NULL;
	}
	return ERROR_SUCCESS;
}
int 
hyScene_Exec(
	cJSON *pstData
)
{
	DEBUG("hyScene_Exec\n");
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	char acSceneEnable[STATE_MAX_LEN] = {0};
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}

	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		iRet = ERROR_NOT_FOUND;
		goto hyscene_exec_end;
	}
	
	if(ERROR_SUCCESS == JSON_value_get(
		JSON_KEY_PANEL_ENABLE, 
		acSceneEnable, 
		STATE_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		/*更新场景使能状态*/
		if(strncmp(pstScene->acSceneEnable, acSceneEnable, STATE_MAX_LEN))
		{
			/*使能发生改变*/
			scene_info_t stScene;
			memset(&stScene, 0x0, sizeof(scene_info_t));

			strncpy(stScene.acSceneId, acSceneId, INDEX_MAX_LEN);
			strncpy(stScene.acSceneName, pstScene->acSceneName, INDEX_MAX_LEN);
			strncpy(stScene.acSceneEnable, acSceneEnable, STATE_MAX_LEN);
			strncpy(stScene.acExecDelayed, pstScene->acExecDelayed, STATE_MAX_LEN);
			strncpy(stScene.acUpdateTime, pstScene->acUpdateTime, TIME_MAX_LEN);
			strncpy(stScene.acNote, pstScene->acNote, NOTE_MAX_LEN);
			iRet = 
				g_pstSceneList->scene_set(
					g_pstSceneList,
					&stScene
				);
			if(ERROR_SUCCESS != iRet)
			{
				iRet = ERROR_GEN;
				goto hyscene_exec_end;
			}
		}
	}
	else
	{
		strncpy(acSceneEnable, pstScene->acSceneEnable, STATE_MAX_LEN);
	}
	
	if(!strncmp(acSceneEnable, "1", STATE_MAX_LEN))
	{
		iRet = g_pstSceneList->scene_active(
						g_pstSceneList, 
						pstScene
					);

	}
	else
	{
		INFO("The scene is disable\n");
		iRet = ERROR_SUCCESS;
		goto hyscene_exec_end;
	}
hyscene_exec_end:

	return iRet;
}

int 
hyScene_Bind(
	cJSON *pstData
)
{
	DEBUG("hyScene_Bind\n");
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	scene_panel_t stPanelInfo;
	memset(&stPanelInfo, 0x0, sizeof(scene_panel_t));

	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		iRet = ERROR_NOT_FOUND;
		goto hyscene_bind_end;
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_PANEL_DEV_ID, 
		stPanelInfo.acDevId, 
		DEV_ID_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_bind_end;
	}
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_PANEL_KEY, 
		stPanelInfo.acKey, 
		KEY_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_bind_end;
	}

	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_PANEL_VALUE, 
		stPanelInfo.acValue, 
		VALUE_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_bind_end;
	}

	//if(ERROR_SUCCESS != JSON_value_get(
	//	JSON_KEY_PANEL_ENABLE, 
	//	stPanelInfo.acEnable, 
	//	STATE_MAX_LEN, 
	//	NULL,
	//	&iValueType,
	//	pstData)
	//)
	//{
	//	ERROR("The json format error.\n");
	//	return ERROR_JSON_FORMAT;
	//}
	strncpy(stPanelInfo.acEnable, "1", STATE_MAX_LEN);
	
	iRet = g_pstSceneList->scene_bind(
				g_pstSceneList, 
				pstScene,
				&stPanelInfo
			);
hyscene_bind_end:
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	return iRet;
}

int 
hyScene_Unbind(
	cJSON *pstData
)
{
	DEBUG("hyScene_Unbind\n");
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	scene_panel_t stPanelInfo;
	memset(&stPanelInfo, 0x0, sizeof(scene_panel_t));

	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		iRet = ERROR_NOT_FOUND;
		goto hyscene_unbind_end;
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_PANEL_DEV_ID, 
		stPanelInfo.acDevId, 
		DEV_ID_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_unbind_end;
	}

	if(!strncmp(stPanelInfo.acDevId, JSON_VALUE_ALL, DEV_ID_MAX_LEN))
	{
		/*解除该场景下的所有绑定*/
		iRet = g_pstSceneList->scene_bind_clear(
					g_pstSceneList, 
					pstScene,
					1
				);
	}
	else
	{
		/*解除某一个场景面板的绑定*/
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_PANEL_KEY, 
			stPanelInfo.acKey, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_unbind_end;
		}

		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_PANEL_VALUE, 
			stPanelInfo.acValue, 
			VALUE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
		}
		
		iRet = g_pstSceneList->scene_unbind(
					g_pstSceneList, 
					pstScene,
					&stPanelInfo
				);
	}
hyscene_unbind_end:
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	return iRet;
}

int 
hyScene_Get_Bind(
	cJSON *pstData, cJSON *pstDataArryOut
)
{
	DEBUG("hyScene_Get_Bind\n");

	int i = 0;
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	int iCount = SCENE_BIND_MAX_NUM;
	scene_panel_t *astPanel = (scene_panel_t *)calloc(SCENE_BIND_MAX_NUM, sizeof(action_info_t));
	if(NULL == astPanel)
	{
		ERROR("Calloc error.\n");
		return ERROR_NO_SPACE;
	}	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		if(astPanel)
		{
			free(astPanel);
			astPanel = NULL;
		}
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		if(astPanel)
		{
			free(astPanel);
			astPanel = NULL;
		}

		/*解锁场景*/
		g_pstSceneList->scene_unlock(g_pstSceneList);
		return ERROR_NOT_FOUND;
	}
	
	
	iRet =
		g_pstSceneList->scene_bind_get_list(
			g_pstSceneList, 
			pstScene, 
			astPanel,
			&iCount
		);
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	if(ERROR_SUCCESS != iRet)
	{
		ERROR("Scene panel get list error\n");
		if(astPanel)
		{
			free(astPanel);
			astPanel = NULL;
		}
		return iRet;
	}

	if(0 == iCount)
	{
		cJSON *pstData = cJSON_CreateObject();
		if(NULL == pstData)
		{
			ERROR("New JSON faild.");
			if(astPanel)
			{
				free(astPanel);
				astPanel = NULL;
			}
			return ERROR_GEN;
		}
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_OPERATION, 
			JSON_VALUE_OPERATION_GETBIND_SCENE
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_ID, 
			pstScene->acSceneId
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_PANEL_DEV_ID, 
			"none"
		);
		cJSON_AddItemToArray(pstDataArryOut, pstData);
		if(astPanel)
		{
			free(astPanel);
			astPanel = NULL;
		}
		return ERROR_SUCCESS;
	}
	for(i = 0; i < iCount; ++i)
	{
		cJSON *pstData = cJSON_CreateObject();
		if(NULL == pstData)
		{
			ERROR("New JSON faild.");
			if(astPanel)
			{
				free(astPanel);
				astPanel = NULL;
			}
			return ERROR_GEN;
		}
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_OPERATION, 
			JSON_VALUE_OPERATION_GETBIND_SCENE
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_ID, 
			pstScene->acSceneId
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_PANEL_DEV_ID, 
			astPanel[i].acDevId
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_PANEL_KEY, 
			astPanel[i].acKey
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_PANEL_VALUE, 
			astPanel[i].acValue
		);
		//cJSON_AddStringToObject(pstData, 
		//	JSON_KEY_PANEL_ENABLE, 
		//	astPanel[i].acEnable
		//);
		cJSON_AddItemToArray(pstDataArryOut, pstData);
	}
	if(astPanel)
	{
		free(astPanel);
		astPanel = NULL;
	}
	return iRet;
}

int 
hyScene_Add_Cond(
	cJSON *pstData
)
{
	DEBUG("hyScene_Add_Cond\n");
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	char acLogic[LOGIC_MAX_LEN] = {0};
	char acCondType[TYPE_MAX_LEN] = {0};
	time_cond_t stTimeCond;
	event_cond_t stEventCond;
	memset(&stTimeCond, 0x0, sizeof(time_cond_t));
	memset(&stEventCond, 0x0, sizeof(event_cond_t));
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		iRet = ERROR_NOT_FOUND;
		goto hyscene_add_cond_end;
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_COND_LOGIC, 
		acLogic, 
		LOGIC_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_add_cond_end;
	}
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_COND_TYPE, 
		acCondType, 
		TYPE_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_add_cond_end;
	}
	if(!strncmp(acCondType, 
			JSON_VALUE_COND_TYPE_TIME,
			TYPE_MAX_LEN)
	)
	{
		/*时间条件*/
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_TIME_KEY, 
			stTimeCond.acTimeKey, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_TRIGGER_TYPE, 
			stTimeCond.acTriggerType, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stTimeCond.acTriggerType, 
				JSON_VALUE_COND_TRIGGER_TYPE_INSTANT_ONCE, 
				TRIGGER_TYPE_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_TRIGGER_INTERVAL, 
			stTimeCond.acTriggerInterval, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stTimeCond.acTriggerInterval, 
				"1", TRIGGER_INTERVAL_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_TIME_KEY, 
			stTimeCond.acTimeKey, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_START_HOUR, 
			stTimeCond.acStartHour, 
			HOUR_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_START_MINU, 
			stTimeCond.acStartMinu, 
			MINU_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_END_HOUR, 
			stTimeCond.acEndHour, 
			HOUR_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_END_MINU, 
			stTimeCond.acEndMinu, 
			MINU_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_WEEK, 
			stTimeCond.acWeek, 
			WEEK_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_REPEAT, 
			stTimeCond.acRepeat, 
			STATE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}

		strncpy(stTimeCond.acLose, "0", STATE_MAX_LEN);

		if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_OR,
			LOGIC_MAX_LEN))
		{
			/*时间或条件*/
			iRet =
				g_pstSceneList->time_or_cond_add(
					g_pstSceneList, 
					pstScene, 
					&stTimeCond
				);
		}
		else if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_AND,
			LOGIC_MAX_LEN))
		{
			/*时间与条件*/
			iRet = 
				g_pstSceneList->time_and_cond_add(
					g_pstSceneList, 
					pstScene,
					&stTimeCond
				);
		}
	}
	else if(!strncmp(acCondType, 
			JSON_VALUE_COND_TYPE_EVENT,
			TYPE_MAX_LEN)
	)
	{
		/*事件条件*/
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_DEV_ID, 
			stEventCond.acDevId, 
			DEV_ID_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_KEY, 
			stEventCond.acKey, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet =  ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_OR,
			LOGIC_MAX_LEN))
		{
			/*获取事件条件*/
			iRet = 
				g_pstSceneList->event_or_cond_get(
					g_pstSceneList,
					pstScene, 
					&stEventCond
				);
		}
		else
		{
			/*获取事件条件*/
			iRet = 
				g_pstSceneList->event_and_cond_get(
					g_pstSceneList,
					pstScene, 
					&stEventCond
				);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_VALUE, 
			stEventCond.acValue, 
			VALUE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_TRIGGER_TYPE, 
			stEventCond.acTriggerType, 
			TRIGGER_TYPE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stEventCond.acTriggerType, 
				JSON_VALUE_COND_TRIGGER_TYPE_INSTANT_ONCE, 
				TRIGGER_TYPE_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_CONTINUE_TIME, 
			stEventCond.acContinueTime, 
			CONTINUE_TIME_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			strncpy(stEventCond.acContinueTime, 
				"0", 
				CONTINUE_TIME_MAX_LEN);
		}
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_ACTIVE, 
			stEventCond.acActive, 
			ACTIVE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_add_cond_end;
		}

		if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_OR,
			LOGIC_MAX_LEN))
		{
			/*事件或条件*/
			iRet = 
				g_pstSceneList->event_or_cond_add(
					g_pstSceneList,
					pstScene, 
					&stEventCond
				);
		}
		else if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_AND,
			LOGIC_MAX_LEN))
		{
			/*事件与条件*/
			iRet = 
				g_pstSceneList->event_and_cond_add(
					g_pstSceneList, 
					pstScene, 
					&stEventCond
				);
		}
	}
hyscene_add_cond_end:
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	return iRet;
}
int 
hyScene_Del_Cond(
	cJSON *pstData
)
{
	DEBUG("hyScene_Del_Cond\n");
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	char acLogic[LOGIC_MAX_LEN] = {0};
	char acCondType[TYPE_MAX_LEN] = {0};
	time_cond_t stTimeCond;
	event_cond_t stEventCond;
	memset(&stTimeCond, 0x0, sizeof(time_cond_t));
	memset(&stEventCond, 0x0, sizeof(event_cond_t));
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		iRet = ERROR_NOT_FOUND;
		goto hyscene_del_cond_end;
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_COND_LOGIC, 
		acLogic, 
		LOGIC_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_del_cond_end;
	}
	if(!strncmp(acLogic, JSON_VALUE_ALL, LOGIC_MAX_LEN))
	{
		/*清空所有与条件以及或条件*/
		iRet = 
			g_pstSceneList->time_or_cond_clear(
				g_pstSceneList,
				pstScene,
				1
			);
		iRet += 
			g_pstSceneList->event_or_cond_clear(
				g_pstSceneList,
				pstScene,
				1
			);
		iRet += 
			g_pstSceneList->time_and_cond_clear(
				g_pstSceneList,
				pstScene,
				1
			);
		iRet += 
			g_pstSceneList->event_and_cond_clear(
				g_pstSceneList,
				pstScene,
				1
			);
	}
	else
	{
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_COND_TYPE, 
			acCondType, 
			TYPE_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_del_cond_end;
		}
		
		if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_OR,
			LOGIC_MAX_LEN))
		{
			/*或条件集合*/
			
			if(!strncmp(acCondType, 
					JSON_VALUE_COND_TYPE_TIME,
					TYPE_MAX_LEN)
			)
			{
				/*时间条件*/
				if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_COND_TIME_KEY, 
					stTimeCond.acTimeKey, 
					KEY_MAX_LEN, 
					NULL,
					&iValueType,
					pstData)
				)
				{
					ERROR("The json format error.\n");
					iRet = ERROR_JSON_FORMAT;
					goto hyscene_del_cond_end;
				}
				if(!strncmp(stTimeCond.acTimeKey, 
					JSON_VALUE_ALL,
					TYPE_MAX_LEN)
				)
				{
					/*删除所有时间条件*/
					iRet = 
						g_pstSceneList->time_or_cond_clear(
							g_pstSceneList,
							pstScene,
							1
						);
				}
				else
				{
					/*删除某一时间条件*/
					iRet =
						g_pstSceneList->time_or_cond_del(
							g_pstSceneList, 
							pstScene, 
							&stTimeCond
						);
				}
				
			}
			else if(!strncmp(acCondType, 
					JSON_VALUE_COND_TYPE_EVENT,
					TYPE_MAX_LEN)
			)
			{
				/*事件条件*/
				if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_COND_DEV_ID, 
					stEventCond.acDevId, 
					DEV_ID_MAX_LEN, 
					NULL,
					&iValueType,
					pstData)
				)
				{
					ERROR("The json format error.\n");
					iRet = ERROR_JSON_FORMAT;
					goto hyscene_del_cond_end;
				}
				if(!strncmp(stEventCond.acDevId, 
					JSON_VALUE_ALL,
					DEV_ID_MAX_LEN)
				)
				{
					/*删除所有事件条件*/
					iRet = 
						g_pstSceneList->event_or_cond_clear(
							g_pstSceneList,
							pstScene,
							1
						);
				}
				else
				{
					/*删除某一事件条件*/
					iRet =
						g_pstSceneList->event_or_cond_del(
							g_pstSceneList, 
							pstScene, 
							&stEventCond
						);
				}
			}
			else if(!strncmp(acCondType, 
					JSON_VALUE_ALL,
					TYPE_MAX_LEN)
			)
			{
				/*删除所有或条件*/
				iRet = 
					g_pstSceneList->time_or_cond_clear(
						g_pstSceneList,
						pstScene,
						1
					);
				iRet += 
					g_pstSceneList->event_or_cond_clear(
						g_pstSceneList,
						pstScene,
						1
					);
			}
		}
		else if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_AND,
			LOGIC_MAX_LEN))
		{
			/*与条件集合*/
			if(!strncmp(acCondType, 
					JSON_VALUE_COND_TYPE_TIME,
					TYPE_MAX_LEN)
			)
			{
				/*时间条件*/
				if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_COND_TIME_KEY, 
					stTimeCond.acTimeKey, 
					KEY_MAX_LEN, 
					NULL,
					&iValueType,
					pstData)
				)
				{
					ERROR("The json format error.\n");
					iRet = ERROR_JSON_FORMAT;
					goto hyscene_del_cond_end;
				}
				if(!strncmp(stTimeCond.acTimeKey, 
					JSON_VALUE_ALL,
					TYPE_MAX_LEN)
				)
				{
					/*删除所有时间条件*/
					iRet = 
						g_pstSceneList->time_and_cond_clear(
							g_pstSceneList,
							pstScene,
							1
						);
				}
				else
				{
					/*删除某一时间条件*/
					iRet =
						g_pstSceneList->time_and_cond_del(
							g_pstSceneList, 
							pstScene, 
							&stTimeCond
						);
				}
				
			}
			else if(!strncmp(acCondType, 
					JSON_VALUE_COND_TYPE_EVENT,
					TYPE_MAX_LEN)
			)
			{
				/*事件条件*/
				if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_COND_DEV_ID, 
					stEventCond.acDevId, 
					DEV_ID_MAX_LEN, 
					NULL,
					&iValueType,
					pstData)
				)
				{
					ERROR("The json format error.\n");
					iRet = ERROR_JSON_FORMAT;
					goto hyscene_del_cond_end;
				}
				if(!strncmp(stEventCond.acDevId, 
					JSON_VALUE_ALL,
					DEV_ID_MAX_LEN)
				)
				{
					/*删除所有事件条件*/
					iRet = 
						g_pstSceneList->event_and_cond_clear(
							g_pstSceneList,
							pstScene,
							1
						);
				}
				else
				{
					/*删除某一事件条件*/
					iRet =
						g_pstSceneList->event_and_cond_del(
							g_pstSceneList, 
							pstScene, 
							&stEventCond
						);
				}
			}
			else if(!strncmp(acCondType, 
					JSON_VALUE_ALL,
					TYPE_MAX_LEN)
			)
			{
				/*删除所有或条件*/
				iRet = 
					g_pstSceneList->time_and_cond_clear(
						g_pstSceneList,
						pstScene,
						1
					);
				iRet += 
					g_pstSceneList->event_and_cond_clear(
						g_pstSceneList,
						pstScene,
						1
					);
			}
		}
	}
hyscene_del_cond_end:

	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);

	return iRet;
}
int 
hyScene_Get_Cond(
	cJSON *pstData, cJSON *pstDataArryOut
)
{
	DEBUG("hyScene_Get_Cond\n");
	int i = 0;
	int iRet = 0;
	int iCount = COND_MAX_NUM;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	char acLogic[LOGIC_MAX_LEN] = {0};
	char acCondType[TYPE_MAX_LEN] = {0};
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		iRet = ERROR_NOT_FOUND;
		goto hyscene_get_cond_end;
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_COND_LOGIC, 
		acLogic, 
		LOGIC_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_get_cond_end;
	}
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_COND_TYPE, 
		acCondType, 
		TYPE_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_get_cond_end;
	}
	if(!strncmp(acCondType, 
			JSON_VALUE_COND_TYPE_TIME,
			TYPE_MAX_LEN)
	)
	{
		time_cond_t *astTimeCond = (time_cond_t *)calloc(COND_MAX_NUM, sizeof(time_cond_t));
		if(NULL == astTimeCond)
		{
			ERROR("Calloc error.\n");
			iRet = ERROR_NO_SPACE;
			goto hyscene_get_cond_end;
		}
		if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_OR,
			LOGIC_MAX_LEN))
		{
			/*时间或条件*/
			iRet =
				g_pstSceneList->time_or_cond_get_list(
					g_pstSceneList, 
					pstScene, 
					astTimeCond,
					&iCount
				);
		}
		else if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_AND,
			LOGIC_MAX_LEN))
		{
			/*时间与条件*/
			iRet = 
				g_pstSceneList->time_and_cond_get_list(
					g_pstSceneList, 
					pstScene,
					astTimeCond,
					&iCount
				);
		}
		if(0 == iCount)
		{
			cJSON *pstData = cJSON_CreateObject();
			if(NULL == pstData)
			{
				ERROR("New JSON faild.");
				if(astTimeCond)
				{
					free(astTimeCond);
					astTimeCond = NULL;
				}
				iRet = ERROR_GEN;
				goto hyscene_get_cond_end;
			}
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_OPERATION, 
				JSON_VALUE_OPERATION_GET_SCENE
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_SCENE_ID, 
				acSceneId
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_LOGIC, 
				acLogic
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TYPE, 
				acCondType
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TIME_KEY, 
				"none"
			);
			cJSON_AddItemToArray(pstDataArryOut, pstData);
			if(astTimeCond)
			{
				free(astTimeCond);
				astTimeCond = NULL;
			}
			iRet = ERROR_SUCCESS;
			goto hyscene_get_cond_end;
		}
		for(i = 0; i < iCount; ++i)
		{
			cJSON *pstData = cJSON_CreateObject();
			if(NULL == pstData)
			{
				ERROR("New JSON faild.");
				iRet = ERROR_GEN;
				goto hyscene_get_cond_end;
			}
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_OPERATION, 
				JSON_VALUE_OPERATION_GET_SCENE
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_SCENE_ID, 
				acSceneId
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_LOGIC, 
				acLogic
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TYPE, 
				acCondType
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TIME_KEY, 
				astTimeCond[i].acTimeKey
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TRIGGER_TYPE, 
				astTimeCond[i].acTriggerType
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TRIGGER_INTERVAL, 
				astTimeCond[i].acTriggerInterval
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_START_HOUR, 
				astTimeCond[i].acStartHour
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_START_MINU, 
				astTimeCond[i].acStartMinu
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_END_HOUR, 
				astTimeCond[i].acEndHour
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_END_MINU, 
				astTimeCond[i].acEndMinu
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_WEEK, 
				astTimeCond[i].acWeek
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_REPEAT, 
				astTimeCond[i].acRepeat
			);
			
			cJSON_AddItemToArray(pstDataArryOut, pstData);
		}
		if(astTimeCond)
		{
			free(astTimeCond);
			astTimeCond = NULL;
		}
	}
	else if(!strncmp(acCondType, 
			JSON_VALUE_COND_TYPE_EVENT,
			TYPE_MAX_LEN)
	)
	{
		event_cond_t *astEventCond = (event_cond_t *)calloc(COND_MAX_NUM, sizeof(event_cond_t));
		if(NULL == astEventCond)
		{
			ERROR("Calloc error.\n");
			iRet = ERROR_NO_SPACE;
			goto hyscene_get_cond_end;
		}
		if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_OR,
			LOGIC_MAX_LEN))
		{
			/*事件或条件*/
			iRet = 
				g_pstSceneList->event_or_cond_get_list(
					g_pstSceneList,
					pstScene, 
					astEventCond,
					&iCount
				);
		}
		else if(!strncmp(acLogic, 
			JSON_VALUE_COND_LOGIC_AND,
			LOGIC_MAX_LEN))
		{
			/*事件与条件*/
			iRet = 
				g_pstSceneList->event_and_cond_get_list(
					g_pstSceneList, 
					pstScene, 
					astEventCond,
					&iCount
				);
		}

		/*对手动触发条件做特殊化处理*/
		if(1 == pstScene->cManualTriggerFlag)
		{
			strncpy(astEventCond[iCount].acTriggerType, 
				JSON_VALUE_COND_TRIGGER_TYPE_INSTANT_ONCE, TRIGGER_TYPE_MAX_LEN);
			strncpy(astEventCond[iCount].acContinueTime, "0", CONTINUE_TIME_MAX_LEN);
			strncpy(astEventCond[iCount].acDevId, "0000000000000000", DEV_ID_MAX_LEN);
			strncpy(astEventCond[iCount].acKey, JSON_VALUE_KEY_MANUAL, KEY_MAX_LEN);
			strncpy(astEventCond[iCount].acValue, "0", VALUE_MAX_LEN);
			strncpy(astEventCond[iCount].acActive, "Equ", DEV_ID_MAX_LEN);
			iCount ++;
		}
		
		if(0 == iCount)
		{
			cJSON *pstData = cJSON_CreateObject();
			if(NULL == pstData)
			{
				ERROR("New JSON faild.");
				if(astEventCond)
				{
					free(astEventCond);
					astEventCond = NULL;
				}
				iRet = ERROR_GEN;
				goto hyscene_get_cond_end;
			}
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_OPERATION, 
				JSON_VALUE_OPERATION_GET_SCENE
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_SCENE_ID, 
				acSceneId
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_LOGIC, 
				acLogic
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TYPE, 
				acCondType
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_DEV_ID, 
				"none"
			);
			cJSON_AddItemToArray(pstDataArryOut, pstData);
			if(astEventCond)
			{
				free(astEventCond);
				astEventCond = NULL;
			}
			iRet = ERROR_SUCCESS;
			goto hyscene_get_cond_end;
		}
		for(i = 0; i < iCount; ++i)
		{
			cJSON *pstData = cJSON_CreateObject();
			if(NULL == pstData)
			{
				ERROR("New JSON faild.");
				iRet = ERROR_GEN;
				goto hyscene_get_cond_end;
			}
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_OPERATION, 
				JSON_VALUE_OPERATION_GET_SCENE
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_SCENE_ID, 
				acSceneId
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_LOGIC, 
				acLogic
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TYPE, 
				acCondType
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_TRIGGER_TYPE, 
				astEventCond[i].acTriggerType
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_CONTINUE_TIME, 
				astEventCond[i].acContinueTime
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_DEV_ID, 
				astEventCond[i].acDevId
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_KEY, 
				astEventCond[i].acKey
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_VALUE, 
				astEventCond[i].acValue
			);
			cJSON_AddStringToObject(pstData, 
				JSON_KEY_COND_ACTIVE, 
				astEventCond[i].acActive
			);
			
			cJSON_AddItemToArray(pstDataArryOut, pstData);
		}
		if(astEventCond)
		{
			free(astEventCond);
			astEventCond = NULL;
		}
	}
	else
	{
		ERROR("Unknown Cond Type.\n");
		iRet = ERROR_JSON_VALUE;
	}
hyscene_get_cond_end:
	
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	
	return iRet;
}

int 
hyScene_Add_Action(
	cJSON *pstData
)
{
	DEBUG("hyScene_Add_Action\n");
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	action_info_t stAction;
	memset(&stAction, 0x0, sizeof(action_info_t));
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		iRet = ERROR_NOT_FOUND;
		goto hyscene_add_action_end;
	}
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_ACTION_ID, 
		stAction.acActionId, 
		ACTION_ID_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		strncpy(stAction.acActionId, "0", ACTION_ID_MAX_LEN);
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_ACTION_DEV_ID, 
		stAction.acDevId, 
		DEV_ID_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_add_action_end;
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_ACTION_KEY, 
		stAction.acKey, 
		KEY_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_add_action_end;
	}

	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_ACTION_VALUECODING, 
		stAction.acValueCoding, 
		VALUE_CODING_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		strncpy(stAction.acValueCoding, JSON_VALUE_VALUECOGING_ORIGINAL, ACTION_ID_MAX_LEN);
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_ACTION_VALUE, 
		stAction.acValue, 
		VALUE_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_add_action_end;
	}
	
	iRet =
		g_pstSceneList->action_add(
			g_pstSceneList, 
			pstScene, 
			&stAction
		);
hyscene_add_action_end:	
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	return iRet;
}
int 
hyScene_Del_Action(
	cJSON *pstData
)
{
	DEBUG("hyScene_Del_Action\n");
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	action_info_t stAction;
	memset(&stAction, 0x0, sizeof(action_info_t));
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		iRet = ERROR_NOT_FOUND;
		goto hyscene_del_action_end;
	}

	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_ACTION_ID, 
		stAction.acActionId, 
		ACTION_ID_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		strncpy(stAction.acActionId, "0", ACTION_ID_MAX_LEN);
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_ACTION_DEV_ID, 
		stAction.acDevId, 
		DEV_ID_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		iRet = ERROR_JSON_FORMAT;
		goto hyscene_del_action_end;
	}
	if(!strncmp(stAction.acDevId, JSON_VALUE_ALL, DEV_ID_MAX_LEN))
	{
		/*删除该场景下的所有动作*/
		iRet =
			g_pstSceneList->action_clear(
				g_pstSceneList, 
				pstScene, 
				1
			);
	}
	else
	{
		/*删除某一个动作*/
		if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_ACTION_KEY, 
			stAction.acKey, 
			KEY_MAX_LEN, 
			NULL,
			&iValueType,
			pstData)
		)
		{
			ERROR("The json format error.\n");
			iRet = ERROR_JSON_FORMAT;
			goto hyscene_del_action_end;
		}
		
		iRet =
			g_pstSceneList->action_del(
				g_pstSceneList, 
				pstScene, 
				&stAction
			);
	}
hyscene_del_action_end:

	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);

	return iRet;
}
int 
hyScene_Get_Action(
	cJSON *pstData, cJSON *pstDataArryOut
)
{
	DEBUG("hyScene_Get_Action\n");

	int i = 0;
	int iRet = 0;
	int iValueType = 0;
	char acSceneId[INDEX_MAX_LEN] = {0};
	int iCount = ACTION_MAX_NUM;
	action_info_t *astAction = (action_info_t *)calloc(ACTION_MAX_NUM, sizeof(action_info_t));
	if(NULL == astAction)
	{
		ERROR("Calloc error.\n");
		return ERROR_NO_SPACE;
	}
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_SCENE_ID, 
		acSceneId, 
		INDEX_MAX_LEN, 
		NULL,
		&iValueType,
		pstData)
	)
	{
		ERROR("The json format error.\n");
		if(astAction)
		{
			free(astAction);
			astAction = NULL;
		}
		return ERROR_JSON_FORMAT;
	}
	/*锁住场景*/
	g_pstSceneList->scene_lock(g_pstSceneList);
	/*获取场景*/
	scene_info_t * pstScene = 
		g_pstSceneList->scene_get(
			g_pstSceneList, 
			acSceneId
		);
	if(NULL == pstScene)
	{
		ERROR("The Scene not Found.\n");
		if(astAction)
		{
			free(astAction);
			astAction = NULL;
		}
		/*解锁场景*/
		g_pstSceneList->scene_unlock(g_pstSceneList);
		return ERROR_NOT_FOUND;
	}
	
	
	iRet =
		g_pstSceneList->action_get_list(
			g_pstSceneList, 
			pstScene, 
			astAction,
			&iCount
		);
	/*解锁场景*/
	g_pstSceneList->scene_unlock(g_pstSceneList);
	if(ERROR_SUCCESS != iRet)
	{
		ERROR("Action get list error\n");
		if(astAction)
		{
			free(astAction);
			astAction = NULL;
		}
		return iRet;
	}

	if(0 == iCount)
	{
		cJSON *pstData = cJSON_CreateObject();
		if(NULL == pstData)
		{
			ERROR("New JSON faild.");
			if(astAction)
			{
				free(astAction);
				astAction = NULL;
			}
			return ERROR_GEN;
		}
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_OPERATION, 
			JSON_VALUE_OPERATION_GET_ACTION
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_ID, 
			pstScene->acSceneId
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_ACTION_DEV_ID, 
			"none"
		);
		cJSON_AddItemToArray(pstDataArryOut, pstData);
		if(astAction)
		{
			free(astAction);
			astAction = NULL;
		}
		return ERROR_SUCCESS;
	}
	for(i = 0; i < iCount; ++i)
	{
		cJSON *pstData = cJSON_CreateObject();
		if(NULL == pstData)
		{
			ERROR("New JSON faild.");
			if(astAction)
			{
				free(astAction);
				astAction = NULL;
			}
			return ERROR_GEN;
		}
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_OPERATION, 
			JSON_VALUE_OPERATION_GET_ACTION
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_SCENE_ID, 
			pstScene->acSceneId
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_ACTION_ID, 
			astAction[i].acActionId
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_ACTION_DEV_ID, 
			astAction[i].acDevId
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_ACTION_KEY, 
			astAction[i].acKey
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_ACTION_VALUECODING, 
			astAction[i].acValueCoding
		);
		cJSON_AddStringToObject(pstData, 
			JSON_KEY_ACTION_VALUE, 
			astAction[i].acValue
		);
		cJSON_AddItemToArray(pstDataArryOut, pstData);
	}

	if(astAction)
	{
		free(astAction);
		astAction = NULL;
	}
	return iRet;
}

int 
hyScene_Dispatch(
	cJSON *pstDataArryIn, cJSON *pstDataArryOut
)
{
	DEBUG("hyScene_Dispatch\n");
	int iRet = 0;
	int i = 0;
	int iValueType = 0;
	cJSON *pstData = NULL;
	char acOp[OPERATION_MAX_LEN] = {0};
	int iCount = cJSON_GetArraySize(pstDataArryIn);
	/*如果有场景的增删改查，则先执行*/
	for(i = 0; i < iCount; ++i)
	{
		if(NULL != (pstData = cJSON_GetArrayItem(pstDataArryIn,i)))
		{
			if(ERROR_SUCCESS != JSON_value_get(
				JSON_KEY_OPERATION, 
				acOp, 
				OPERATION_MAX_LEN, 
				NULL,
				&iValueType,
				pstData)
			)
			{
				ERROR("The json format error.\n");
				iRet = ERROR_JSON_FORMAT;
				continue;
			}
			if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_ADD_SCENE, 
				OPERATION_MAX_LEN)
			)
			{
				/*增加或设置场景*/
				DEBUG("Start add scene\n");
				iRet += hyScene_Add(pstData);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--/*因为删除第i个元素，下次循环还应从第i个开始*/);
				iCount --;
				DEBUG("End add scene\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_DEL_SCENE, 
				OPERATION_MAX_LEN)
			)
			{
				/*删除场景*/
				DEBUG("Start del scene\n");
				iRet += hyScene_Del(pstData);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End del scene\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_GET_SCENE, 
				OPERATION_MAX_LEN)
			)
			{
				/*获取场景*/
				DEBUG("Start get scene\n");
				cJSON *pstDataArryGet = cJSON_CreateArray();
				if(NULL == pstDataArryGet)
				{
					iRet = ERROR_GEN;
					continue;
				}
				iRet += hyScene_Get(pstData, pstDataArryGet);
				
				/*将返回值中的元素添加到返回数组中*/
				int j = 0;
				cJSON *pstDataGet = NULL;
				cJSON *pstDataTmp = NULL;
				int iCountGet = cJSON_GetArraySize(pstDataArryGet);
				for(j = 0; j < iCountGet; ++j)
				{
					if(NULL != (pstDataGet = 
						cJSON_GetArrayItem(pstDataArryGet, j)))
					{
						pstDataTmp = cJSON_Duplicate(pstDataGet,1);
						cJSON_AddItemToArray(pstDataArryOut, pstDataTmp);
					}
				}
				cJSON_Delete(pstDataArryGet);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;

				DEBUG("End get scene\n");
			}
		}
	}
	iCount = cJSON_GetArraySize(pstDataArryIn);
	/*处理处执行场景以外的其他请求*/
	for(i = 0; i < iCount; ++i)
	{
		if(NULL != (pstData = cJSON_GetArrayItem(pstDataArryIn,i)))
		{
			if(ERROR_SUCCESS != JSON_value_get(
				JSON_KEY_OPERATION, 
				acOp, 
				OPERATION_MAX_LEN, 
				NULL,
				&iValueType,
				pstData)
			)
			{
				ERROR("The json format error.\n");
				continue;
			}
			if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_ADD_ACTION, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start add action\n");
				/*增加或设置动作*/
				iRet += hyScene_Add_Action(pstData);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End add action\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_DEL_ACTION, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start del action\n");
				/*删除动作*/
				iRet += hyScene_Del_Action(pstData);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End del action\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_GET_ACTION, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start get action\n");
				/*获取动作*/
				cJSON *pstDataArryGet = cJSON_CreateArray();
				if(NULL == pstDataArryGet)
				{
					iRet = ERROR_GEN;
					continue;
				}
				iRet += hyScene_Get_Action(pstData, pstDataArryGet);
				/*将返回值中的元素添加到返回数组中*/
				int j = 0;
				cJSON *pstDataGet = NULL;
				cJSON *pstDataTmp = NULL;
				int iCountGet = cJSON_GetArraySize(pstDataArryGet);
				for(j = 0; j < iCountGet; ++j)
				{
					if(NULL != (pstDataGet = 
						cJSON_GetArrayItem(pstDataArryGet, j)))
					{
						pstDataTmp = cJSON_Duplicate(pstDataGet,1);
						cJSON_AddItemToArray(pstDataArryOut, pstDataTmp);
					}
				}
				cJSON_Delete(pstDataArryGet);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End get action\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_ADD_COND, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start add cond\n");
				/*增加或设置条件*/
				iRet += hyScene_Add_Cond(pstData);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End add cond\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_DEL_COND, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start del cond\n");
				/*删除条件*/
				iRet += hyScene_Del_Cond(pstData);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End del cond\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_GET_COND, 
				OPERATION_MAX_LEN)
			)
			{
				/*获取条件*/
				DEBUG("Start get cond\n");
				cJSON *pstDataArryGet = cJSON_CreateArray();
				if(NULL == pstDataArryGet)
				{
					iRet = ERROR_GEN;
					continue;
				}
				iRet += hyScene_Get_Cond(pstData, pstDataArryGet);
				/*将返回值中的元素添加到返回数组中*/
				int j = 0;
				cJSON *pstDataGet = NULL;
				cJSON *pstDataTmp = NULL;
				int iCountGet = cJSON_GetArraySize(pstDataArryGet);
				for(j = 0; j < iCountGet; ++j)
				{
					if(NULL != (pstDataGet = 
						cJSON_GetArrayItem(pstDataArryGet, j)))
					{
						pstDataTmp = cJSON_Duplicate(pstDataGet,1);
						cJSON_AddItemToArray(pstDataArryOut, pstDataTmp);
					}
				}
				cJSON_Delete(pstDataArryGet);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End get cond\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_BIND_SCENE, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start bind scene\n");
				/*绑定场景面板*/
				iRet += hyScene_Bind(pstData);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End bind scene\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_UNBIND_SCENE, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start unbind scene\n");
				/*解绑场景面板*/
				iRet += hyScene_Unbind(pstData);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End unbind scene\n");
			}
			else if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_GETBIND_SCENE, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start get bind scene\n");
				/*获取绑定的场景面板信息*/
				cJSON *pstDataArryGet = cJSON_CreateArray();
				if(NULL == pstDataArryGet)
				{
					iRet = ERROR_GEN;
					continue;
				}
				iRet += hyScene_Get_Bind(pstData, pstDataArryGet);
				/*将返回值中的元素添加到返回数组中*/
				int j = 0;
				cJSON *pstDataGet = NULL;
				cJSON *pstDataTmp = NULL;
				int iCountGet = cJSON_GetArraySize(pstDataArryGet);
				for(j = 0; j < iCountGet; ++j)
				{
					if(NULL != (pstDataGet = 
						cJSON_GetArrayItem(pstDataArryGet, j)))
					{
						pstDataTmp = cJSON_Duplicate(pstDataGet,1);
						cJSON_AddItemToArray(pstDataArryOut, pstDataTmp);
					}
				}
				cJSON_Delete(pstDataArryGet);
				/*处理完成后删除该json数组元素*/
				cJSON_DeleteItemFromArray(pstDataArryIn, i--);
				iCount --;
				DEBUG("End get bind scene\n");
			}
		}
	}

	iCount = cJSON_GetArraySize(pstDataArryIn);
	/*处理执行场景请求*/
	for(i = 0; i < iCount; ++i)
	{
		if(NULL != (pstData = cJSON_GetArrayItem(pstDataArryIn,i)))
		{
			if(ERROR_SUCCESS != JSON_value_get(
				JSON_KEY_OPERATION, 
				acOp, 
				OPERATION_MAX_LEN, 
				NULL,
				&iValueType,
				pstData)
			)
			{
				ERROR("The json format error.\n");
				continue;
			}
			if(!strncmp(acOp, 
				JSON_VALUE_OPERATION_EXEC_SCENE, 
				OPERATION_MAX_LEN)
			)
			{
				DEBUG("Start exec scene\n");
				/*执行场景*/
				iRet += hyScene_Exec(pstData);
				DEBUG("End exec scene\n");
			}
		}
	}
	return iRet;
}
int 
hyScene_Report(
	cJSON *pstDataArryIn, cJSON *pstJsonOut
)
{
	DEBUG("hyScene_Report\n");

	char acCooMac[IEEE_MAX_LEN] = {0};
	
	cJSON_AddStringToObject(pstJsonOut, 
		JSON_KEY_COMMAND, 
		JSON_VALUE_COMMAND_REPORT
	);
	cJSON_AddStringToObject(pstJsonOut, 
		JSON_KEY_FRAMENUMBER, 
		JSON_VALUE_FRAMENUMBER_00
	);
	
	getCooMac(acCooMac);
	cJSON_AddStringToObject(pstJsonOut, 
		JSON_KEY_GATEWAY_ID, 
		acCooMac
	);
	
	cJSON_AddStringToObject(pstJsonOut, 
		JSON_KEY_TYPE, 
		JSON_VALUE_TYPE_SCENE
	);
	cJSON_AddItemToObject(pstJsonOut,
		JSON_KEY_DATA,
		pstDataArryIn
	);
	return ERROR_SUCCESS;
}
 

int scene_init(const char* dataBasePath, int  eventDrivenType)
{
	DEBUG("Local scenario initialization.\n");
	DB *pDB = NULL;

	/*打开数据服务*/
	pDB = db_server_start(dataBasePath);
	if(NULL == pDB)
	{
		ERROR("Open DB(%s) failed.\n", dataBasePath);
		return ERROR_OPEN_DB;
	}
	
	g_pstSceneEventDrivenType = eventDrivenType;
	g_pstSceneList = new_scene_list(pDB);
	
	if(NULL == g_pstSceneList)
	{
		ERROR("New Scene List Error.\n");
		return ERROR_GEN;
	}

	
	/*数据库兼容性处理*/
	/*数据库兼容性处理*/
	db_compatibility(pDB, 0);
	db_compatibility(pDB, 1);
	db_compatibility(pDB, 2);
	db_compatibility(pDB, 3);
	
	
	/*加载数据库数据*/
	g_pstSceneList->scene_init(g_pstSceneList);

	return ERROR_SUCCESS;
}

/*删除所有本地场景配置*/
int scene_clean(void)
{
	if(NULL == g_pstSceneList)
	{
		return ERROR_GEN;
	}
	int iRet = 0;
	int i = 0;
	int iCount = SCENE_MAX_NUM;
	scene_info_t *astScene = (scene_info_t *)calloc(SCENE_MAX_NUM, sizeof(scene_info_t));
	if(NULL == astScene)
	{
		ERROR("Calloc error.\n");
		return ERROR_NO_SPACE;
	}
	g_pstSceneList->scene_get_list(
		g_pstSceneList, 
		astScene,
		&iCount
	);
	DEBUG("iCount = %d\n", iCount);

	for(i = 0; i < iCount; ++i)
	{
		iRet += g_pstSceneList->scene_del(
			g_pstSceneList,
			astScene[i].acSceneId 
		);
	}
	if(astScene)
	{
		free(astScene);
		astScene = NULL;
	}
	return iRet;
}

int scene_destroy(void)
{
	if(NULL == g_pstSceneList)
	{
		return ERROR_GEN;
	}
	DEBUG("hySceneDestroy\n");
	return destroy_scene_list(g_pstSceneList);
}



int 
scene_query(
	char *inputBuff, 
	int inputSize, 
	char *outBuff, 
	int outSize, 
	int *outLen
)
{
	if(NULL == g_pstSceneList)
	{
		return ERROR_GEN;
	}
	PARAM_CHECK_RETURN_ERRORNO_1(inputBuff);
	int iRet = 0;
	int iCount = 0;
	cJSON *pstJson = NULL;
	cJSON *pstDataArry = NULL;
	cJSON *pstDataArryOut = cJSON_CreateArray();
	int iValueType = 0;
	char acType[TYPE_MAX_LEN] = {0};
	char acCommand[COMMAND_MAX_LEN] = {0};
	
	//if(!JSON_check(inputBuff))
	//{
	//	ERROR("The json format error.\n");
	//	return ERROR_JSON_FORMAT;
	//}
	
	pstJson = cJSON_Parse(inputBuff);
	if(NULL == pstJson)
	{
		ERROR("The json format error.\n");
		cJSON_Delete(pstDataArryOut);
		return ERROR_JSON_FORMAT;
	}

	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_TYPE, 
		acType, 
		TYPE_MAX_LEN, 
		NULL,
		&iValueType,
		pstJson)
	)
	{
		ERROR("The json format error.\n");
		cJSON_Delete(pstJson);
		cJSON_Delete(pstDataArryOut);
		return ERROR_JSON_FORMAT;
	}
	if(strncmp(acType,
			JSON_VALUE_TYPE_SCENE, 
			TYPE_MAX_LEN)
	)
	{
		ERROR("The Type error.\n");
		cJSON_Delete(pstJson);
		cJSON_Delete(pstDataArryOut);
		return ERROR_JSON_FORMAT;
		
	}
	
	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_COMMAND, 
		acCommand, 
		COMMAND_MAX_LEN, 
		NULL,
		&iValueType,
		pstJson)
	)
	{
		ERROR("The json format error.\n");
		cJSON_Delete(pstJson);
		cJSON_Delete(pstDataArryOut);
		return ERROR_JSON_FORMAT;
	}
	

	if(ERROR_SUCCESS != JSON_value_get(
			JSON_KEY_DATA, 
			NULL, 
			0, 
			&pstDataArry, 
			&iValueType, 
			pstJson
		)
	)
	{
		ERROR("The json format error.\n");
		cJSON_Delete(pstJson);
		cJSON_Delete(pstDataArryOut);
		return ERROR_JSON_FORMAT;
	}
	else
	{
		if(cJSON_Array != iValueType)
		{
			ERROR("The json format error.\n");
			cJSON_Delete(pstJson);
			cJSON_Delete(pstDataArryOut);
			return ERROR_JSON_FORMAT;
		}
	}
	
	if(!strncmp(acCommand, 
		JSON_VALUE_COMMAND_DISPATCH,
		COMMAND_MAX_LEN)
	)
	{
		/*下发*/
		iRet = hyScene_Dispatch(
			pstDataArry, 
			pstDataArryOut
		);
	}
	else
	{
		ERROR("The Command not found.\n");
		cJSON_Delete(pstJson);
		cJSON_Delete(pstDataArryOut);
		return ERROR_JSON_FORMAT;
	}
	iCount = cJSON_GetArraySize(pstDataArryOut);
	if(ERROR_SUCCESS == iRet && 0 != iCount)
	{
		/*上报数据*/
		cJSON *pstJsonOut = cJSON_CreateObject();
		if(NULL != pstJsonOut)
		{
			iRet = hyScene_Report(
				pstDataArryOut, 
				pstJsonOut
			);
			if(ERROR_SUCCESS == iRet)
			{
				char *pstr = NULL;
				pstr = cJSON_Print(pstJsonOut);
				if(NULL != outBuff && NULL != outLen)
				{
					strncpy(outBuff, pstr, outSize);
					cJSON_Minify(outBuff);
					*outLen = strlen(pstr) > outSize ? outSize : strlen(pstr);
				}
				free(pstr);
				cJSON_Delete(pstJson);
				cJSON_Delete(pstJsonOut);
				return iRet;
			}
			else
			{
				cJSON_Delete(pstJson);
				cJSON_Delete(pstDataArryOut);
				cJSON_Delete(pstJsonOut);
				return iRet;
			}
		}
		else
		{
			cJSON_Delete(pstJson);
			cJSON_Delete(pstDataArryOut);
			return ERROR_GEN;
		}
	}
	
	cJSON_Delete(pstJson);
	cJSON_Delete(pstDataArryOut);
	return iRet;
}

/**************************************************************
函数名：scene_event
描述：本地场景事件发生
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_event(char* inputBuff)
{
	if(NULL == g_pstSceneList)
	{
		return ERROR_GEN;
	}

	cJSON *pstJson = NULL;
	cJSON *pstDataArry = NULL;
	cJSON *pstData = NULL;
	int iValueType = 0;
	char acType[TYPE_MAX_LEN] = {0};
	/*解析json*/
	
	if(!JSON_check(inputBuff))
	{
		ERROR("The json format error.\n");
		return ERROR_JSON_FORMAT;
	}

	pstJson = cJSON_Parse(inputBuff);

	if(ERROR_SUCCESS != JSON_value_get(
		JSON_KEY_TYPE, 
		acType, 
		TYPE_MAX_LEN, 
		NULL,
		&iValueType,
		pstJson)
	)
	{
		ERROR("The json format error.\n");
		cJSON_Delete(pstJson);
		return ERROR_JSON_FORMAT;
	}
	
	
	
	if(!strncmp(acType, JSON_VALUE_TYPE_ATTRIBUTE, TYPE_MAX_LEN) ||
		!strncmp(acType, JSON_VALUE_TYPE_EVENT, TYPE_MAX_LEN)
	)
	{
		/*如果事件来源为外部，则内部的事件不起作用*/
		//if(0 != g_pstSceneEventDrivenType)
		//{
		//	cJSON_Delete(pstJson);
		//	return ERROR_SUCCESS;
		//}
		
		/*属性上报或事件上报*/
		if(ERROR_SUCCESS != JSON_value_get(
				JSON_KEY_DATA, 
				NULL, 
				0, 
				&pstDataArry, 
				&iValueType, 
				pstJson
			)
		)
		{
			ERROR("The json format error.\n");
			cJSON_Delete(pstJson);
			return ERROR_JSON_FORMAT;
		}
		else
		{
			if(cJSON_Array != iValueType)
			{
				ERROR("The json format error.\n");
				cJSON_Delete(pstJson);
				return ERROR_JSON_FORMAT;
			}
		}
		
		int i = 0;
		int iCount = cJSON_GetArraySize(pstDataArry);
		for(i = 0; i < iCount; i++)
		{
			if(NULL != (pstData = cJSON_GetArrayItem(pstDataArry,i)))
			{
				event_t stEvent;
				memset(&stEvent, 0x0, sizeof(event_t));
				
				if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_DATA_DAVICE_ID, 
					stEvent.acDevId, 
					DEV_ID_MAX_LEN, 
					NULL,
					&iValueType,
					pstData)
				)
				{
					ERROR("The json format error.\n");
					continue;
				}

				if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_DATA_KEY, 
					stEvent.acKey, 
					KEY_MAX_LEN, 
					NULL,
					&iValueType,
					pstData)
				)
				{
					ERROR("The json format error.\n");
					continue;
				}

				if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_DATA_VALUE, 
					stEvent.acValue, 
					VALUE_MAX_LEN, 
					NULL,
					&iValueType,
					pstData)
				)
				{
					ERROR("The json format error.\n");
					continue;
				}
				g_pstSceneList->scene_event(g_pstSceneList, &stEvent);
			}
			
		}
	}
	else if(!strncmp(acType, JSON_VALUE_TYPE_UNREGISTER, TYPE_MAX_LEN))
	{
		/*接收到设备注销上报，将场景中所有与该设备相关的删除*/
		
		if(ERROR_SUCCESS != JSON_value_get(
				JSON_KEY_DATA, 
				NULL, 
				0, 
				&pstDataArry, 
				&iValueType, 
				pstJson
			)
		)
		{
			ERROR("The json format error.\n");
			cJSON_Delete(pstJson);
			return ERROR_JSON_FORMAT;
		}
		else
		{
			if(cJSON_Array != iValueType)
			{
				ERROR("The json format error.\n");
				cJSON_Delete(pstJson);
				return ERROR_JSON_FORMAT;
			}
		}
		
		int i = 0;
		int iCount = cJSON_GetArraySize(pstDataArry);
		for(i = 0; i < iCount; i++)
		{
			if(NULL != (pstData = cJSON_GetArrayItem(pstDataArry,i)))
			{
				char acDevId[DEV_ID_MAX_LEN] = {0};
				
				if(ERROR_SUCCESS != JSON_value_get(
					JSON_KEY_DATA_DAVICE_ID, 
					acDevId, 
					DEV_ID_MAX_LEN, 
					NULL,
					&iValueType,
					pstData)
				)
				{
					ERROR("The json format error.\n");
					continue;
				}

				g_pstSceneList->scene_dev_unregister(g_pstSceneList, acDevId);
			}
		}
		
	}
	else if(!strncmp(acType, JSON_VALUE_TYPE_REFACTORY, TYPE_MAX_LEN))
	{
		/*接收到出厂恢复上报，将场景中所有所有数据删除*/
		g_pstSceneList->scene_clear(g_pstSceneList, 1);
	}
	cJSON_Delete(pstJson);
	return ERROR_SUCCESS;
}

/**************************************************************
函数名：scene_event2
描述：本地场景事件发生
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_event2(char* devId, char* key, char* value)
{
	if(NULL == g_pstSceneList)
	{
		return ERROR_GEN;
	}
	
	//if(1 != g_pstSceneEventDrivenType)
	//{
	//	return ERROR_SUCCESS;
	//}
	
	event_t stEvent;
	memset(&stEvent, 0x0, sizeof(event_t));
	
	strncpy(stEvent.acDevId, devId, DEV_ID_MAX_LEN);
	strncpy(stEvent.acKey, key, KEY_MAX_LEN);
	strncpy(stEvent.acValue, value, VALUE_MAX_LEN);
	
	return g_pstSceneList->scene_event(g_pstSceneList, &stEvent);
}

/**************************************************************
函数名：scene_report_reg
描述：本地场景上报接口注册
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_report_reg(void* funName)
{
	DEBUG("Register local scene report callback function.\n");
	if(NULL == g_pstSceneList)
	{
		return ERROR_GEN;
	}
	return g_pstSceneList->scene_report_reg(
		g_pstSceneList, 
		(SceneReportFun)funName
	);
}
/**************************************************************
函数名：scene_action_cb_reg
描述：场景动作回调函数注册
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_action_cb_reg(int iCbid, void *pFun, void *pUserData)
{
	return action_callback_reg(iCbid, pFun, pUserData);
}

