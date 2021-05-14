/***********************************************************
*文件名     : log.c
*版   本   : v1.0.0.0
*日   期   : 2018.05.03
*说   明   : 日志系统相关接口
*修改记录: 
************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h> 
#include <dirent.h>
#include <stdarg.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

#include "log.h"
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

int g_iFlieNum = 10;
int g_iFileSize = 1024 * 1024;
int g_iCount = 0;
log_level_t g_log_level;
char g_acLogFilePath[PATCH_NAME_MAX_LEN];
FILE *g_log_fp;

static char *loglevels[] = {
 "", "FATAL", "ERROR", "WARN ", "INFO ", "DEBUG", "TRACE"
};

int webPrintFlag=0;
int logSocket=0;
struct sockaddr_in broadcastAddr;
#define BROADCOAT_INTERFACE_NAME "br-lan"
#define BROADCOAT_PORT  (10001)
/*************************************************************
*函数:	CreatUdpLogClint
*描述:	创建UDP 广播客户端，用于将日志广播到局域网
*参数:	无
*结果:	成功0,失败-1;
*************************************************************/
static int CreatUdpLogClint()
{
	int lenAddr=sizeof(broadcastAddr);
	bzero(&broadcastAddr,lenAddr);
	broadcastAddr.sin_family=AF_INET;
	broadcastAddr.sin_addr.s_addr=inet_addr("255.255.255.255");
	broadcastAddr.sin_port=htons(BROADCOAT_PORT);
	bzero(&(broadcastAddr.sin_zero),8);


	struct sockaddr_in clientAddr;
	clientAddr.sin_port=htons(BROADCOAT_PORT);
	clientAddr.sin_addr.s_addr=htonl(INADDR_ANY);
	bzero(&(clientAddr.sin_zero),8);
	if (logSocket>=0)
		close(logSocket);
	logSocket=socket(AF_INET,SOCK_DGRAM,0);
	if (logSocket<0)
	{
		return -1;
	}

	int optival=1;
	setsockopt(logSocket,SOL_SOCKET,SO_BROADCAST,&optival,sizeof(int));

	struct ifreq if_eth0;
	strncpy(if_eth0.ifr_ifrn.ifrn_name,BROADCOAT_INTERFACE_NAME,strlen(BROADCOAT_INTERFACE_NAME));
	setsockopt(logSocket,SOL_SOCKET,SO_BINDTODEVICE,(char*)&if_eth0,sizeof(if_eth0));
	return 0;
}

/*************************************************************
*函数:	sendUdpBroadcastLog
*描述:	将日志广播到局域网
*参数:	@char *inputBuff:输入的日志;
*结果:	成功0,失败-1;
*************************************************************/
static int sendUdpBroadcastLog(char *inputBuff)
{
	int res=0;
	if (webPrintFlag==1)
	{
		res=sendto(logSocket,inputBuff,strlen(inputBuff),0,(struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr));
		if (res>0)
		{
			res= 0;
		}
		else
		{
			res= -1;
		}

	}
	return res;
}
/*************************************************************
*函数:	setUdpBroadcastFalg
*描述:	设置日志广播到局域网标志位;
*参数:	@int open_close:1 允许广播日志; 0 禁止广播日志;
*结果:	成功0,失败-1;
*************************************************************/
extern void SetUdpBroadcastFalg(int open_close)
{
	webPrintFlag=open_close;
}


/*************************************************************
*函数:	CreateDir
*参数:	pcPathName :目录
*描述:	创建多级目录
*************************************************************/
static int CreateDir(const char *pcPathName)
{
	int i = 0;
	int iLen = 0;
	char acPatchName[PATCH_NAME_MAX_LEN] = {0};
	char *pch = NULL;
	strncpy(acPatchName, pcPathName, PATCH_NAME_MAX_LEN);

	/*找到最后一个‘/’,并在'/'后截断字符串*/
	if(NULL != (pch = strrchr(acPatchName, '/')))
	{
		*(pch + 1) = '\0';
	}
	else
	{
		return -1;
	}

	iLen = strlen(acPatchName);
	for(i = 1; i < iLen; ++i)
	{
		if('/' == acPatchName[i])
		{
			acPatchName[i] = 0;
			if(access(acPatchName, R_OK)!=0)
			{
				if(mkdir(acPatchName, 0755)==-1)
				{
					return -1;
				}
			}
		acPatchName[i] = '/';
		}
	}
	return 0;
} 

/*************************************************************
*函数:	InitLog
*参数:	log_level :日志输出等级
*描述:	初始化日志输出
*************************************************************/
extern void InitLog(log_level_t enLog_Level, const char* pcLogFilePath)
{
	g_log_level = enLog_Level;
	if(NULL == pcLogFilePath)
	{
		g_log_fp = LOG_STDOUT_FD;
	}
	else
	{
		/*创建路径*/
		strncpy(g_acLogFilePath, pcLogFilePath, PATCH_NAME_MAX_LEN);
		CreateDir(g_acLogFilePath);
		/*创建文件*/
		g_log_fp = fopen(g_acLogFilePath, "a+");
		if(NULL == g_log_fp)
		{
			g_log_fp = LOG_STDOUT_FD;
		}
	}
	CreatUdpLogClint();
}
/*************************************************************
*函数:	InitLogFileNum
*参数:	iFileNum :日志文件个数
*描述:	初始化日志文件个数，默认2个
*************************************************************/
extern void InitLogFileNum(int iFileNum)
{
	g_iFlieNum = iFileNum;
}
/*************************************************************
*函数:	InitLogFileSize
*参数:	iFileSize :日志文件大小（单位字节）
*描述:	初始化日志文件大小，默认1024*1024（1M）
*************************************************************/
extern void InitLogFileSize(int iFileSize)
{
	g_iFileSize = iFileSize;
}

/*************************************************************
*函数:	InitLog
*参数:	log_level :日志输出等级
*描述:	初始化日志输出
*************************************************************/
void LOG(log_level_t enLog_Level, const char *pcFormat, ...)
{
	/*判断日志等级*/
	if(g_log_level < enLog_Level)
	{
		return;
	}
	/*判断文件大小*/
	if(LOG_STDOUT_FD != g_log_fp)
	{
		if(g_iFileSize < ftell(g_log_fp))
		{
			fclose(g_log_fp);
			g_log_fp = NULL;
			g_iCount ++;
			char acPatchName[PATCH_NAME_MAX_LEN] = {0};
			char acPatchNameNew[PATCH_NAME_MAX_LEN] = {0};
			snprintf(acPatchName, PATCH_NAME_MAX_LEN, "%s", g_acLogFilePath);
			snprintf(acPatchNameNew, PATCH_NAME_MAX_LEN, "%s.%d", g_acLogFilePath, g_iCount);
			rename(acPatchName, acPatchNameNew);
			if(g_iCount == g_iFlieNum - 1)
			{
				g_iCount = 0;
			}
			/*重新打开文件*/
			g_log_fp = fopen(g_acLogFilePath, "w");
			if(NULL == g_log_fp)
			{
				g_log_fp = LOG_STDOUT_FD;
			}
		}
	}

	/*日志输出*/
	va_list ap;
	time_t t = time(0);
	char acTimeBuf[32];
	strftime(acTimeBuf, sizeof(acTimeBuf), "%Y-%m-%d %H:%M:%S", localtime(&t));
	fprintf(g_log_fp, "%s [%s] ", acTimeBuf, loglevels[enLog_Level]);
	va_start(ap, pcFormat);
	vfprintf(g_log_fp, pcFormat, ap);
	va_end(ap);
	fflush(g_log_fp);

	if (webPrintFlag==1)
	{
		char tempBuff[1024]={0};
		char lastBuff[1024]={0};
		sprintf(lastBuff, "%s [%s] ", acTimeBuf, loglevels[enLog_Level]);
		va_start(ap, pcFormat);
		vsprintf(tempBuff, pcFormat, ap);
		va_end(ap);
		strcat(lastBuff,tempBuff);

		sendUdpBroadcastLog(lastBuff);
	}


}

/*************************************************************
*函数:	LOG_HEX
*参数:	pMemory :内存指针
*		iOffset: 起始下标
*		iExtent: 打印长度
*描述:	二进制日志输出
*************************************************************/
extern void LOG_HEX (log_level_t enLog_Level, const void * pMemory, int iOffset, int iExtent)
{
	/*判断日志等级*/
	if(g_log_level < enLog_Level)
	{
		return;
	}
	/*判断文件大小*/
	if(LOG_STDOUT_FD != g_log_fp)
	{
		if(g_iFileSize < ftell(g_log_fp))
		{
			fclose(g_log_fp);
			g_log_fp = NULL;
			g_iCount ++;
			char acPatchName[PATCH_NAME_MAX_LEN] = {0};
			char acPatchNameNew[PATCH_NAME_MAX_LEN] = {0};
			snprintf(acPatchName, PATCH_NAME_MAX_LEN, "%s", g_acLogFilePath);
			snprintf(acPatchNameNew, PATCH_NAME_MAX_LEN, "%s.%d", g_acLogFilePath, g_iCount);
			rename(acPatchName, acPatchNameNew);
			if(g_iCount == g_iFlieNum - 1)
			{
				g_iCount = 0;
			}
			/*重新打开文件*/
			g_log_fp = fopen(g_acLogFilePath, "w");
			if(NULL == g_log_fp)
			{
				g_log_fp = LOG_STDOUT_FD;
			}
		}
	}

	char * origin = (char *)(pMemory);
	unsigned field = sizeof (iExtent) + sizeof (iExtent);
	unsigned block = 0x10;
	int lower = block * (iOffset / block);
	int upper = block + lower;
	int index = 0;
	char buffer [sizeof (iExtent) + sizeof (iExtent) + 72];
	char * output;
	while (lower < iExtent) 
	{
		output = buffer + field;
		for (index = lower; output-- > buffer; index >>= 4) 
		{
			*output = DIGITS_HEX [index & 0x0F];
		}
		output = buffer + field;
		*output++ = ' ';
		for (index = lower; index < upper; index++) 
		{
			if (index < iOffset) 
			{
				*output++ = ' ';
				*output++ = ' ';
			}
			else if (index < iExtent) 
			{
				*output++ = DIGITS_HEX [(origin [index] >> 4) & 0x0F];
				*output++ = DIGITS_HEX [(origin [index] >> 0) & 0x0F];
			}
			else 
			{
				*output++ = ' ';
				*output++ = ' ';
			}
			*output++ = ' ';
		}
		for (index = lower; index < upper; index++) 
		{
			if (index < iOffset) 
			{
				*output++ = ' ';
			}
			else if (index < iExtent) 
			{
				unsigned c = origin [index];
				*output++ = isprint (c)? c: '.';
			}
			else 
			{
				*output++ = ' ';
			}
		}
		*output++ = '\n';
		*output++ = '\0';
		fputs (buffer, g_log_fp);
		lower += block;
		upper += block;
	}

	output = buffer;
	*output++ = '\n';
	*output++ = '\0';
	fputs (buffer, g_log_fp);

	return;

}


