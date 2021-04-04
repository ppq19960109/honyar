/***********************************************************
*文件名    : scene.h
*版   本   : v1.0.0.0
*日   期   : 2018.07.24
*说   明   : 本地化场景协议接口
*
*修改记录: 
************************************************************/

#ifndef SCENE_H
#define SCENE_H

/**************************************************************
函数名：scene_init
描述：本地化场景初始化
参数：@char *dataBasePath：数据库路径
@int  eventDrivenType: 事件驱动类型，1表示外部事件驱动
	   0表示内部事件驱动，即由SDK内部提供事件传入
返回：0初始化成功，-1失败；
**************************************************************/
int scene_init(const char* dataBasePath, int  eventDrivenType);

/**************************************************************
函数名：scene_destroy
描述：销毁本地场景的初始化
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_destroy(void);

/**************************************************************
函数名：scene_clean
描述：清空本地场景配置
参数：
返回：0删除成功，非0失败；
**************************************************************/
int scene_clean(void);


/**************************************************************
函数名：hyScene_Query
描述：本地化场景命令处理函数，查询时同步返回
参数：@char *inputJsonString：控制命令Json字符串，参考鸿雁对外协议说明文档；
	  @int inputSize：输入缓存的大小,至少为实际控制命令Json字符串长度+1；
      @char *outBuff：查询结果输出缓存，不能为空；
	  @int outSize：输出缓存大小；
	  @int*outLen，实际输出字符串大小,由调用者创建；
返回：0命令接收或发送成功，-1失败；
**************************************************************/
int scene_query(char *inputBuff,int inputSize, char *outBuff,int outSize, int *outLen);


/**************************************************************
函数名：hySceneEvent
描述：本地场景事件发生
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_event(char* inputBuff);
/**************************************************************
函数名：scene_event2
描述：本地场景事件发生
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_event2(char* devId, char* key, char* value);

/**************************************************************
函数名：scene_report_reg
描述：本地场景上报接口注册
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_report_reg(void* funName);

/**************************************************************
函数名：scene_action_cb_reg
描述：场景动作回调函数注册
参数：
返回：0销毁成功，-1失败；
**************************************************************/
int scene_action_cb_reg(int iCbid, void *pFun, void *pUserData);


#endif /* SCENE_H */