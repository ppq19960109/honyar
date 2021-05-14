/***********************************************************
*文件名     : scene_sync.h
*版   本   : v1.0.0.0
*日   期   : 2021.04.03
*说   明   : 本地场景之状态同步
*修改记录: 
************************************************************/


#ifndef SCENE_SYNC_H
#define SCENE_SYNC_H



/*添加状态同步控制指令*/
int 
scene_sync_event_req_add(
	char *pcDevId, 
	char *pcKey, 
	char *pcValue
);

/*同步控制范围内的属性上报事件，判断其为主动上报还是控制回应*/
/*
*1、返回值为0, 代表该上报事件为首次上报. 
*	此时后续需要触发相关状态同步处理
*2、返回值为1, 代表该上报事件为请求事件的回应.
*	此时无需触发后续处理。
*/
int 
scene_sync_event(
	char *pcDevId, 
	char *pcKey, 
	char *pcValue
);

/*同步控制范围内的属性上报事件，判断其为主动上报还是控制回应*/
int 
scene_sync_event_req_exec(void);



#endif /* SCENE_SYNC_H */
