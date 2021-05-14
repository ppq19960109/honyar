/*
 * shell_api.c
 *
 *  Created on: Jul 16, 2020
 *      Author: jerry
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
int hy_system(const char *pcCmd, char *pcResult, int iBuffSize)
{
        /*参数定义*/
        int i = 0;
        int iReadSize = 0;
        FILE *pp =NULL;
        char *pcData = NULL;
        char *pcStr = NULL;
        
        /*参数校验*/
        if(NULL == pcCmd)
        {
                return -1;
        }

        /*创建管道*/
        pp = popen(pcCmd, "r");
        if(!pp)
        {
                return -1;
        }

        /*判断是否关心命令的执行结果*/
        if(NULL == pcResult ||
                0 == iBuffSize)
        {
                /*关闭管道*/
                pclose(pp);
                return 0;
        }

        /*获取命令的执行结果*/
        pcData = (char*)malloc(4096);
        if(NULL == pcData)
        {
                /*关闭管道*/
                pclose(pp);
                return -1;
        }
        memset(pcData, 0x0, 4096);
        
        pcStr = pcData;
        do
        {
                i++;
                iReadSize = 0;
                iReadSize = fread (pcStr, 4096, 1, pp);

                if(1 != iReadSize)
                {
                        break;
                }

                pcData =
                        (char*)realloc(pcData,(i + 1) * 4096);
                if(NULL == pcData)
                {
                        /*关闭管道*/
                        pclose(pp);
                        return -1;
                }
                pcStr = pcData + (i * 4096);
                memset(pcStr, 0x0, 4096);        
        }while(1);

        /*返回命令执行结果*/
        strncpy(pcResult, pcData, iBuffSize);

        /*关闭管道*/
        pclose(pp);
        free(pcData);

        return 0;
}
