
#ifndef TOOL_H
#define TOOL_H

#define CONFIG_KEY_CORE		"CORE"
#define CONFIG_KEY_VER		"VER"
#define CONFIG_KEY_MODEL    "MODEL"

extern int get_local_ip(char * ifname, char * ip);
extern int HYgetMac(char* Mac);

/*************************************************************
*函数:	read_file
*参数:	pcPath :文件路径
*		pcBuff :输出缓存
*		iBuffSize:缓存大小
*返回值:成功返回0,失败返回-1
*描述:	读取文件
*************************************************************/
extern int
read_file(const char *pcPath, char *pcBuff, int iBuffSize);

/*************************************************************
*函数:	stristr
*参数:	str1:被查找目标
*		str2:要查找对象
*返回值:成功返回str2在str1的首次出现的地址,失败返回NULL
*描述:	忽略大小写的strstr
*************************************************************/
extern char * stristr(const char * str1, const char * str2);
/*************************************************************
*函数:	is_int
*参数:	str:字符串
*返回值:1表示是整数，0表示非整数
*描述:	判断字符串是否是整数类型
*************************************************************/
extern int is_int(char * str);

/*************************************************************
*函数:	is_float
*参数:	str:字符串
*返回值:1表示是浮点数，0表示非浮点数
*描述:	判断字符串是否是浮点类型
*************************************************************/
extern int is_float(char * str);

/*************************************************************
*函数:	time_cmp
*参数:	iHour_1:iMinu_1 时间1的时分
*		iHour_2:iMinu_2 时间2的时分
*返回值:0表示时间相等，小于0，表示时间1先于时间2，大于0，表示时间1后于时间2
*描述:	判断时间先后(时间应为24小时制，且不跨天)
*************************************************************/
extern int time_cmp(int iHour_1, int iMinu_1, int iHour_2, int iMinu_2);
/*************************************************************
*函数:	procedure_core_get
*参数:	pcCore:程序代码
*返回:	ml_error_t
*描述:	获取程序代码
*************************************************************/
extern int
procedure_core_get(char* pcCore);

/*************************************************************
*函数:	procedure_ver_get
*参数:	pcVersion:程序版本
*返回:	ml_error_t
*描述:	获取程序版本号
*************************************************************/
extern int
procedure_ver_get(char* pcVersion);


/*************************************************************
*函数:	procedure_model_get
*参数:	pcModel:程序model
*返回:	ml_error_t
*描述:	获取网关model型号
*************************************************************/
extern int
procedure_model_get(char* pcModel);


#endif
