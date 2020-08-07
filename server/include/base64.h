#ifndef BASE64_H
#define BASE64_H

//bindata编码成base64
//bindata：  源字符串
//binlength: 源字符串长度
//base64：目的字符串，base64字符串
//返回值：base64字符串
char * base64_encode( const unsigned char * bindata, int binlength, char * base64 );

//解码base64
//base64：源字符串
//bindata: 目的字符串
//返回值：目的字符串长度
int base64_decode( const char * base64, unsigned char * bindata );

#endif // BASE64_H

