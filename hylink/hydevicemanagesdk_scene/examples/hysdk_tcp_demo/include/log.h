/***********************************************************
*文件名     : log.h
*版   本   : v1.0.0.0
*日   期   : 2018.05.03
*说   明   : 日志系统相关接口
*修改记录: 
************************************************************/


#ifndef LOG_H_
#define LOG_H_

#define LOG_STDOUT_FD	stdout
#define PATCH_NAME_MAX_LEN	1024
#define FILE_NAME_MAX_LEN	128


#define DIGITS_HEX "0123456789ABCDEF"

typedef enum {
	LOG_NONE = 0,	/*关闭日志*/
	LOG_FATAL,		/*重大错误信息*/
	LOG_ERROR,		/*错误信息*/
	LOG_WARN,		/*警告信息*/
	LOG_INFO,		/*情报信息*/
	LOG_DEBUG,		/*调试信息*/
	LOG_TRACE		/*跟踪信息*/
} log_level_t;

#define FATAL(fmt, ...) LOG(LOG_FATAL,"[%s:%s:%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define ERROR(fmt, ...) LOG(LOG_ERROR,"[%s:%s:%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define WARN(fmt, ...)  LOG(LOG_WARN, "[%s:%s:%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define INFO(fmt, ...)  LOG(LOG_INFO, "[%s:%s:%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define DEBUG(fmt, ...) LOG(LOG_DEBUG,"[%s:%s:%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)
#define TRACE(fmt, ...) LOG(LOG_TRACE,"[%s:%s:%d] "fmt, __FILE__, __FUNCTION__, __LINE__, ##__VA_ARGS__)


/*函数声明*/
/*************************************************************
*函数:	InitLog
*参数:	log_level :日志输出等级
*		pcLogFilePath : 日志文件
*描述:	初始化日志输出
*************************************************************/
extern void InitLog(log_level_t enLog_Level, const char* pcLogFilePath);
/*************************************************************
*函数:	InitLogFileNum
*参数:	iFileNum :日志文件个数
*描述:	初始化日志文件个数，默认2个
*************************************************************/
extern void InitLogFileNum(int iFileNum);
/*************************************************************
*函数:	InitLogFileSize
*参数:	iFileSize :日志文件大小（单位字节）
*描述:	初始化日志文件大小，默认1024*1024（1M）
*************************************************************/
extern void InitLogFileSize(int iFileSize);

/*************************************************************
*函数:	InitLog
*参数:	log_level :日志输出等级
*描述:	初始化日志输出
*************************************************************/
extern void LOG(log_level_t enLog_Level, const char *pcFormat, ...);
/*************************************************************
*函数:	LOG_HEX
*参数:	pMemory :内存指针
*		iOffset: 起始下标
*		iExtent: 打印长度
*描述:	二进制日志输出
*************************************************************/
extern void LOG_HEX (log_level_t enLog_Level, const void * pMemory, int iOffset, int iExtent);

/*************************************************************
*函数:	setUdpBroadcastFalg
*描述:	设置日志广播到局域网标志位;
*参数:	@int open_close:1 允许广播日志; 0 禁止广播日志;
*结果:	成功0,失败-1;
*************************************************************/
extern void SetUdpBroadcastFalg(int open_close);
#endif /* LOG_H_ */
