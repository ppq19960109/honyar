#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXBUFSIZE 1024

//通过链接文件名获取被目标文件绝对路径
//为了防止buffer不够大，包装一下readlink()
char* getFilePathByLink(const char *filename)
{
	int size = 100;
	char *buffer = NULL;

	while (1)
	{
		buffer = (char *)realloc(buffer, size);
		//readlink()返回的路径名并没有以'\0'结尾
		int nchars = readlink(filename, buffer, size - 1);
		if (nchars < 0)
		{
			free(buffer);
			return NULL;
		}
		if (nchars <= size - 1) {
			buffer[nchars] = '\0';        //让路径名以'\0'结尾
			return buffer;
		}
		size *= 2;    //buffer空间不足，扩大为2倍
	}
}

//通过链接文件名获取被目标文件所在目录
char* getFileDirByLink(const char *linkfile)
{
	char* pathbuf = NULL;
	pathbuf = getFilePathByLink(linkfile);
	if (NULL == pathbuf) {
		printf("getFilePathByLink() Failed!\n");
		return(NULL);
	}

	char* last_slash = NULL;
	//找到最后一个'/',last_slash保存其地址
	last_slash = strrchr(pathbuf, '/');
	//如果是空，则未找到'/'
	if (last_slash == NULL) {
		printf("can not find '/'!\n");
		return(NULL);
	}

	int dir_len = 0;
	char* dir = NULL;
	dir_len = last_slash - pathbuf + 1;        //包括末尾的'/'
	dir = (char*)malloc(dir_len + 1);

	strncpy(dir, pathbuf, dir_len);
	dir[dir_len] = '\0';

	return dir;
}

