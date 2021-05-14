#ifndef _MISC_H_
#define _MISC_H_

char *md5_encode(const char *sIn, char *sOut);
char *md5_decode(const char *sIn, char *sOut);
char *base64_encode(const unsigned char * bindata, int binlength, char * base64);
int base64_decode( const char * base64, unsigned char * bindata );

#endif	/* _MISC_H_ */
