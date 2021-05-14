#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <ctype.h>
#include <sys/ioctl.h>
#include <time.h>

#include "misc.h"

#define   MD5_KEY            "cc2ef1eb6f2242c587b2c056f71611cb"


typedef unsigned char *MD5_POINTER;   
typedef unsigned long int MD5_UINT4;   

typedef struct     
{   
	MD5_UINT4   state[4];   
	MD5_UINT4   count[2];   
	unsigned   char   buffer[64];   
}MD5_CTX;

#define   MD5_S11   7   
#define   MD5_S12   12   
#define   MD5_S13   17   
#define   MD5_S14   22   
#define   MD5_S21   5   
#define   MD5_S22   9   
#define   MD5_S23   14   
#define   MD5_S24   20   
#define   MD5_S31   4   
#define   MD5_S32   11   
#define   MD5_S33   16   
#define   MD5_S34   23   
#define   MD5_S41   6   
#define   MD5_S42   10   
#define   MD5_S43   15   
#define   MD5_S44   21   

#define   MD5_F(x,   y,   z)   (((x)   &   (y))   |   ((~x)   &   (z)))   
#define   MD5_G(x,   y,   z)   (((x)   &   (z))   |   ((y)   &   (~z)))   
#define   MD5_H(x,   y,   z)   ((x)   ^   (y)   ^   (z))   
#define   MD5_I(x,   y,   z)   ((y)   ^   ((x)   |   (~z)))   

#define   MD5_ROTATE_LEFT(x,   n)   (((x)   <<   (n))   |   ((x)   >>   (32-(n))))   

#define   MD5_FF(a,   b,   c,   d,   x,   s,   ac)   {     (a)   +=   MD5_F   ((b),   (c),   (d))   +   (x)   +   (MD5_UINT4)(ac);     (a)   =   MD5_ROTATE_LEFT   ((a),   (s));     (a)   +=   (b);       }   
#define   MD5_GG(a,   b,   c,   d,   x,   s,   ac)   {     (a)   +=   MD5_G   ((b),   (c),   (d))   +   (x)   +   (MD5_UINT4)(ac);     (a)   =   MD5_ROTATE_LEFT   ((a),   (s));     (a)   +=   (b);       }   
#define   MD5_HH(a,   b,   c,   d,   x,   s,   ac)   {     (a)   +=   MD5_H   ((b),   (c),   (d))   +   (x)   +   (MD5_UINT4)(ac);     (a)   =   MD5_ROTATE_LEFT   ((a),   (s));     (a)   +=   (b);       }   
#define   MD5_II(a,   b,   c,   d,   x,   s,   ac)   {     (a)   +=   MD5_I   ((b),   (c),   (d))   +   (x)   +   (MD5_UINT4)(ac);     (a)   =   MD5_ROTATE_LEFT   ((a),   (s));     (a)   +=   (b);   }   



static unsigned char MD5_PADDING[64] = 
{   
	0x80,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   
	0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0   
};  

const char * base64char = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";


void   MD5Encode(unsigned   char   *output,   MD5_UINT4   *input,   unsigned   int   len)   
{   
	unsigned   int   i,   j;   

	for   (i   =   0,   j   =   0;   j   <   len;   i++,   j   +=   4)   {   
		output[j]   =   (unsigned   char)(input[i]   &   0xff);   
		output[j+1]   =   (unsigned   char)((input[i]   >>   8)   &   0xff);   
		output[j+2]   =   (unsigned   char)((input[i]   >>   16)   &   0xff);   
		output[j+3]   =   (unsigned   char)((input[i]   >>   24)   &   0xff);   
	}   
}   

void   MD5Decode(MD5_UINT4   *output,   unsigned   char   *input,   unsigned   int   len)   
{   
	unsigned   int   i,   j;   

	for   (i   =   0,   j   =   0;   j   <   len;   i++,   j   +=   4)   
		output[i]   =   ((MD5_UINT4)input[j])   |   (((MD5_UINT4)input[j+1])   <<   8)   |   
			(((MD5_UINT4)input[j+2])   <<   16)   |   (((MD5_UINT4)input[j+3])   <<   24);   
}   

void   MD5Transform   (MD5_UINT4   state[4],   unsigned   char   block[64])   
{   
	MD5_UINT4   a   =   state[0],   b   =   state[1],   c   =   state[2],   d   =   state[3],   x[16];   
	MD5Decode   (x,   block,   64);   
	MD5_FF   (a,   b,   c,   d,   x[ 0],   MD5_S11,   0xd76aa478);   /*   1   */   
	MD5_FF   (d,   a,   b,   c,   x[ 1],   MD5_S12,   0xe8c7b756);   /*   2   */   
	MD5_FF   (c,   d,   a,   b,   x[ 2],   MD5_S13,   0x242070db);   /*   3   */   
	MD5_FF   (b,   c,   d,   a,   x[ 3],   MD5_S14,   0xc1bdceee);   /*   4   */   
	MD5_FF   (a,   b,   c,   d,   x[ 4],   MD5_S11,   0xf57c0faf);   /*   5   */   
	MD5_FF   (d,   a,   b,   c,   x[ 5],   MD5_S12,   0x4787c62a);   /*   6   */   
	MD5_FF   (c,   d,   a,   b,   x[ 6],   MD5_S13,   0xa8304613);   /*   7   */   
	MD5_FF   (b,   c,   d,   a,   x[ 7],   MD5_S14,   0xfd469501);   /*   8   */   
	MD5_FF   (a,   b,   c,   d,   x[ 8],   MD5_S11,   0x698098d8);   /*   9   */   
	MD5_FF   (d,   a,   b,   c,   x[ 9],   MD5_S12,   0x8b44f7af);   /*   10   */   
	MD5_FF   (c,   d,   a,   b,   x[10],   MD5_S13,   0xffff5bb1);   /*   11   */   
	MD5_FF   (b,   c,   d,   a,   x[11],   MD5_S14,   0x895cd7be);   /*   12   */   
	MD5_FF   (a,   b,   c,   d,   x[12],   MD5_S11,   0x6b901122);   /*   13   */   
	MD5_FF   (d,   a,   b,   c,   x[13],   MD5_S12,   0xfd987193);   /*   14   */   
	MD5_FF   (c,   d,   a,   b,   x[14],   MD5_S13,   0xa679438e);   /*   15   */   
	MD5_FF   (b,   c,   d,   a,   x[15],   MD5_S14,   0x49b40821);   /*   16   */   
	MD5_GG   (a,   b,   c,   d,   x[ 1],   MD5_S21,   0xf61e2562);   /*   17   */   
	MD5_GG   (d,   a,   b,   c,   x[ 6],   MD5_S22,   0xc040b340);   /*   18   */   
	MD5_GG   (c,   d,   a,   b,   x[11],   MD5_S23,   0x265e5a51);   /*   19   */   
	MD5_GG   (b,   c,   d,   a,   x[ 0],   MD5_S24,   0xe9b6c7aa);   /*   20   */   
	MD5_GG   (a,   b,   c,   d,   x[ 5],   MD5_S21,   0xd62f105d);   /*   21   */   
	MD5_GG   (d,   a,   b,   c,   x[10],   MD5_S22,   0x02441453);   /*   22   */   
	MD5_GG   (c,   d,   a,   b,   x[15],   MD5_S23,   0xd8a1e681);   /*   23   */   
	MD5_GG   (b,   c,   d,   a,   x[ 4],   MD5_S24,   0xe7d3fbc8);   /*   24   */   
	MD5_GG   (a,   b,   c,   d,   x[ 9],   MD5_S21,   0x21e1cde6);   /*   25   */   
	MD5_GG   (d,   a,   b,   c,   x[14],   MD5_S22,   0xc33707d6);   /*   26   */   
	MD5_GG   (c,   d,   a,   b,   x[ 3],   MD5_S23,   0xf4d50d87);   /*   27   */   
	MD5_GG   (b,   c,   d,   a,   x[ 8],   MD5_S24,   0x455a14ed);   /*   28   */   
	MD5_GG   (a,   b,   c,   d,   x[13],   MD5_S21,   0xa9e3e905);   /*   29   */   
	MD5_GG   (d,   a,   b,   c,   x[ 2],   MD5_S22,   0xfcefa3f8);   /*   30   */   
	MD5_GG   (c,   d,   a,   b,   x[ 7],   MD5_S23,   0x676f02d9);   /*   31   */   
	MD5_GG   (b,   c,   d,   a,   x[12],   MD5_S24,   0x8d2a4c8a);   /*   32   */   
	MD5_HH   (a,   b,   c,   d,   x[ 5],   MD5_S31,   0xfffa3942);   /*   33   */   
	MD5_HH   (d,   a,   b,   c,   x[ 8],   MD5_S32,   0x8771f681);   /*   34   */   
	MD5_HH   (c,   d,   a,   b,   x[11],   MD5_S33,   0x6d9d6122);   /*   35   */   
	MD5_HH   (b,   c,   d,   a,   x[14],   MD5_S34,   0xfde5380c);   /*   36   */   
	MD5_HH   (a,   b,   c,   d,   x[ 1],   MD5_S31,   0xa4beea44);   /*   37   */   
	MD5_HH   (d,   a,   b,   c,   x[ 4],   MD5_S32,   0x4bdecfa9);   /*   38   */   
	MD5_HH   (c,   d,   a,   b,   x[ 7],   MD5_S33,   0xf6bb4b60);   /*   39   */   
	MD5_HH   (b,   c,   d,   a,   x[10],   MD5_S34,   0xbebfbc70);   /*   40   */   
	MD5_HH   (a,   b,   c,   d,   x[13],   MD5_S31,   0x289b7ec6);   /*   41   */   
	MD5_HH   (d,   a,   b,   c,   x[ 0],   MD5_S32,   0xeaa127fa);   /*   42   */   
	MD5_HH   (c,   d,   a,   b,   x[ 3],   MD5_S33,   0xd4ef3085);   /*   43   */   
	MD5_HH   (b,   c,   d,   a,   x[ 6],   MD5_S34,   0x04881d05);   /*   44   */   
	MD5_HH   (a,   b,   c,   d,   x[ 9],   MD5_S31,   0xd9d4d039);   /*   45   */   
	MD5_HH   (d,   a,   b,   c,   x[12],   MD5_S32,   0xe6db99e5);   /*   46   */   
	MD5_HH   (c,   d,   a,   b,   x[15],   MD5_S33,   0x1fa27cf8);   /*   47   */   
	MD5_HH   (b,   c,   d,   a,   x[ 2],   MD5_S34,   0xc4ac5665);   /*   48   */   
	MD5_II   (a,   b,   c,   d,   x[ 0],   MD5_S41,   0xf4292244);   /*   49   */   
	MD5_II   (d,   a,   b,   c,   x[ 7],   MD5_S42,   0x432aff97);   /*   50   */   
	MD5_II   (c,   d,   a,   b,   x[14],   MD5_S43,   0xab9423a7);   /*   51   */   
	MD5_II   (b,   c,   d,   a,   x[ 5],   MD5_S44,   0xfc93a039);   /*   52   */   
	MD5_II   (a,   b,   c,   d,   x[12],   MD5_S41,   0x655b59c3);   /*   53   */   
	MD5_II   (d,   a,   b,   c,   x[ 3],   MD5_S42,   0x8f0ccc92);   /*   54   */   
	MD5_II   (c,   d,   a,   b,   x[10],   MD5_S43,   0xffeff47d);   /*   55   */   
	MD5_II   (b,   c,   d,   a,   x[ 1],   MD5_S44,   0x85845dd1);   /*   56   */   
	MD5_II   (a,   b,   c,   d,   x[ 8],   MD5_S41,   0x6fa87e4f);   /*   57   */   
	MD5_II   (d,   a,   b,   c,   x[15],   MD5_S42,   0xfe2ce6e0);   /*   58   */   
	MD5_II   (c,   d,   a,   b,   x[ 6],   MD5_S43,   0xa3014314);   /*   59   */   
	MD5_II   (b,   c,   d,   a,   x[13],   MD5_S44,   0x4e0811a1);   /*   60   */   
	MD5_II   (a,   b,   c,   d,   x[ 4],   MD5_S41,   0xf7537e82);   /*   61   */   
	MD5_II   (d,   a,   b,   c,   x[11],   MD5_S42,   0xbd3af235);   /*   62   */   
	MD5_II   (c,   d,   a,   b,   x[ 2],   MD5_S43,   0x2ad7d2bb);   /*   63   */   
	MD5_II   (b,   c,   d,   a,   x[ 9],   MD5_S44,   0xeb86d391);   /*   64   */   
	state[0]   +=   a;   
	state[1]   +=   b;   
	state[2]   +=   c;   
	state[3]   +=   d;   
	memset   ((MD5_POINTER)x,   0,   sizeof   (x));   
}   

void   MD5Init(MD5_CTX   *context)   
{   
	context->count[0]   =   context->count[1]   =   0;   
	context->state[0]   =   0x67452301;   
	context->state[1]   =   0xefcdab89;   
	context->state[2]   =   0x98badcfe;   
	context->state[3]   =   0x10325476;   
}   

void   MD5Update(MD5_CTX   *context,   unsigned   char   *input,   unsigned   int   inputLen)   
{   
	unsigned   int   i,   index,   partLen;   

	index   =   (unsigned   int)((context->count[0]   >>   3)   &   0x3F);   
	if   ((context->count[0]   +=   ((MD5_UINT4)inputLen   <<   3))   
			<   ((MD5_UINT4)inputLen   <<   3))   
		context->count[1]++;   
	context->count[1]   +=   ((MD5_UINT4)inputLen   >>   29);   

	partLen   =   64   -   index;   

	if   (inputLen   >=   partLen)   {   
		memcpy((MD5_POINTER)&context->buffer[index],   (MD5_POINTER)input,   partLen);   
		MD5Transform(context->state,   context->buffer);   

		for   (i   =   partLen;   i   +   63   <   inputLen;   i   +=   64)   
			MD5Transform   (context->state,   &input[i]);   
		index   =   0;   
	}   
	else   
		i   =   0;   

	memcpy((MD5_POINTER)&context->buffer[index],   (MD5_POINTER)&input[i],   inputLen-i);   
}   

void   MD5Final(unsigned   char   digest[16],   MD5_CTX   *context)   
{   
	unsigned   char   bits[8];   
	unsigned   int   index,   padLen;   

	MD5Encode   (bits,   context->count,   8);   
	index   =   (unsigned   int)((context->count[0]   >>   3)   &   0x3f);   
	padLen   =   (index   <   56)   ?   (56   -   index)   :   (120   -   index);   
	MD5Update   (context,   MD5_PADDING,   padLen);   
	MD5Update   (context,   bits,   8);   
	MD5Encode   (digest,   context->state,   16);   
	memset   ((MD5_POINTER)context,   0,   sizeof   (*context));   
}   

void   MD5Digest(char   *pszInput,   unsigned   long   nInputSize,   char   *pszOutPut)   
{   
	MD5_CTX   context;   
	unsigned   int   len   =   strlen   (pszInput);   

	MD5Init   (&context);   
	MD5Update   (&context,   (unsigned   char   *)pszInput,   len);   
	MD5Final   ((unsigned   char   *)pszOutPut,   &context);   
}   

static unsigned char _md5_toHex(const unsigned char x)
{
	return x > 9 ? x -10 + 'A': x + '0';
}
 
static unsigned char _md5_fromHex(const unsigned char x)
{
	return isdigit(x) ? x-'0' : x-'A'+10;
}

char *md5_encode(const char *sIn, char* sOut)
{
	size_t ix;
	unsigned char buf[4];
	char szDigest[16];  
	char encrypt_key[33] = {0};
	char rand_data[10] = {0};
	char encrypt_rand[33] = {0};
	unsigned char ch = 0;
	unsigned char ch2 = 0;
	unsigned char ch3 = 0;
	int len = 0;
	int i = 0;
	int j = 0;

	sprintf(encrypt_key, "%s", MD5_KEY);
	
	srand((int)time(0));
	sprintf(rand_data, "%d", rand()%100);

	MD5Digest(rand_data, strlen(rand_data), szDigest);  
	for (i=0;i<16;i++)
	{
		sprintf(encrypt_rand+i*2, "%02x", (unsigned char)szDigest[i]);
	}

	i = 0;
	len = strlen(sIn)*2;
	for(ix = 0; ix < len; ix++ )
	{
		if (i == strlen(encrypt_rand)) 
		{
			i = 0;
		}
		
 		if(ix%2 == 0)
		{
			ch2 = encrypt_rand[i/2];
			ch = ch2 ^ encrypt_key[i];		
		}else
		{
			ch3 = sIn[ix/2] ^ encrypt_rand[i/2];
			ch = ch3 ^ encrypt_key[i];		
		}
	
		memset( buf, 0, 4 );
		if( isalnum( (unsigned char)ch) )
		{      
			buf[0] = ch;
			sprintf(sOut+j, "%s", buf);
			j++;
		}
		else
		{
			buf[0] = '%';
			buf[1] = _md5_toHex( (unsigned char)ch >> 4 );
			buf[2] = _md5_toHex( (unsigned char)ch % 16);
			sprintf(sOut+j, "%s", buf);		
			j += 3;
		}		
		i++;
	}

	sOut[j] = '\0';

	return sOut;
}
 
char *md5_decode(const char *sIn, char *sOut)
{
	char encrypt_key[33] = {0};
	size_t ix;
	int i = 0;
	int j = 0;
	unsigned char ch = 0;
	unsigned char ch2 = 0;
	unsigned char ch3 = 0;
	int len = 0;

	sprintf(encrypt_key, "%s", MD5_KEY);
	
	i = 0;
	len = strlen(sIn);
	for(ix = 0; ix < len; ix++ )
	{
		if(sIn[ix]=='%')
		{
			ch = (_md5_fromHex(sIn[ix+1])<<4);
			ch |= _md5_fromHex(sIn[ix+2]);		
			ix += 2;
		}
		else if(sIn[ix] == '+')
		{
			ch = ' ';
		}
		else
		{
			ch = sIn[ix];
		}
		
		if (i == strlen(encrypt_key)) 
		{
			i = 0;
		}		
		ch2 = ch ^ encrypt_key[i];
	
		if(j%2 == 0)
		{
			ch3 = ch2;
		}
		
		if(j%2 != 0)
		{
			sOut[(j+1)/2-1] = (ch2 ^ ch3);		
		}

		j++;
		i++;
	}
	sOut[j/2] = '\0';
	
	return sOut;
}



char * base64_encode(const unsigned char * bindata, int binlength, char * base64)
{
    int i, j;
    unsigned char current;

    for ( i = 0, j = 0 ; i < binlength ; i += 3 )
    {
        current = (bindata[i] >> 2) ;
        current &= (unsigned char)0x3F;
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i] << 4 ) ) & ( (unsigned char)0x30 ) ;
        if ( i + 1 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+1] >> 4) ) & ( (unsigned char) 0x0F );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)(bindata[i+1] << 2) ) & ( (unsigned char)0x3C ) ;
        if ( i + 2 >= binlength )
        {
            base64[j++] = base64char[(int)current];
            base64[j++] = '=';
            break;
        }
        current |= ( (unsigned char)(bindata[i+2] >> 6) ) & ( (unsigned char) 0x03 );
        base64[j++] = base64char[(int)current];

        current = ( (unsigned char)bindata[i+2] ) & ( (unsigned char)0x3F ) ;
        base64[j++] = base64char[(int)current];
    }
    base64[j] = '\0';
    return base64;
}

int base64_decode( const char * base64, unsigned char * bindata )
{
    int i, j;
    unsigned char k;
    unsigned char temp[4];
    for ( i = 0, j = 0; base64[i] != '\0' ; i += 4 )
    {
        memset( temp, 0xFF, sizeof(temp) );
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i] )
                temp[0]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+1] )
                temp[1]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+2] )
                temp[2]= k;
        }
        for ( k = 0 ; k < 64 ; k ++ )
        {
            if ( base64char[k] == base64[i+3] )
                temp[3]= k;
        }

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[0] << 2))&0xFC)) |
                ((unsigned char)((unsigned char)(temp[1]>>4)&0x03));
        if ( base64[i+2] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[1] << 4))&0xF0)) |
                ((unsigned char)((unsigned char)(temp[2]>>2)&0x0F));
        if ( base64[i+3] == '=' )
            break;

        bindata[j++] = ((unsigned char)(((unsigned char)(temp[2] << 6))&0xF0)) |
                ((unsigned char)(temp[3]&0x3F));
    }
    return j;
}


