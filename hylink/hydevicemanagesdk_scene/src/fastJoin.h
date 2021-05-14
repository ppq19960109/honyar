


/********************************************************
函数名： handlFastJoinCommand
描  述：快速入网指令处理函数
参  数：@char *commandStr：快速入网指令字符串；

返  回：-1 失败 ，0成功.
*********************************************************/
int handleFastJoinCommand(char *commandStr);



/********************************************************
函数名： dealFastJoinResultReport
描  述：解析处理快速入网结果上报指令
参  数：@char *reportState：快速入网指令字符串；
返  回：-1 失败 ，0成功.
*********************************************************/
int dealFastJoinResultReport(char *reportState);


