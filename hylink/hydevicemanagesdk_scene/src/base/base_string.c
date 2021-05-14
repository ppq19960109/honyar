#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "error_msg.h"
#include "base_api.h"

extern char *base_strcpy(char *destin, char *source)
{
	return strcpy(destin, source);
}
extern char *base_strncpy(char *dest, char *src, int n)
{
	return strncpy(dest, src, n);
}

extern char *base_strcat(char *destin, char *source)
{
	return strcat(destin, source);
}

extern char *base_strchr(char *str, char c)
{
	return strchr(str, c);
}

extern char *base_strrchr(char *str, int c)
{
	return strrchr(str, c);
}

extern int base_strcmp(char *str1, char *str2)
{
	return strcmp(str1, str2);
}

extern int base_strncmp(char *str1, char *str2, int n)
{
	return strncmp(str1, str2, n);
}

extern int base_strcasecmp(char *str1, char *str2)
{
	return strcasecmp(str1, str2);
}

extern int base_strlen(char *str)
{
	return strlen(str);
}

extern char *base_strtok_r(char *str, char *delim, char **ptrptr)
{
	return strtok_r(str, delim, ptrptr);
}


extern int base_isprint(char c)
{
	return isprint(c);
}

extern int base_atoi(char *str)
{
	return atoi(str);
}
extern long int base_atol(char *str)
{
	return atol(str);
}

extern double base_atof(char *str)
{
	return atof(str);
}

extern unsigned long int base_strtoul(char *nptr, char **endptr, int base)
{
	return strtoul(nptr, endptr, base);
}


