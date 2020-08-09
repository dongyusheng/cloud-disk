/******************************************************
 *
 *  des.c
 *  common des......
 *
 ******************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "des.h"

/*********************************************************
  data type definition for Des;
**********************************************************/
#define EN0	0
#define DE1	1

#define DES_KEYBYTES	128
#define DES_KEYLONGS	32
#define DES_BLOCKLEN	8

typedef struct {
	unsigned char ek[DES_KEYBYTES];
	int	ekLen;
	unsigned char dk[DES_KEYBYTES];
	int	dkLen;
	unsigned char CbcCtx[DES_BLOCKLEN];
} DES_CTX;

typedef struct {
	unsigned char ek1[DES_KEYBYTES];
	int	ek1Len;
	unsigned char dk1[DES_KEYBYTES];
	int	dk1Len;
	unsigned char ek2[DES_KEYBYTES];
	int	ek2Len;
	unsigned char dk2[DES_KEYBYTES];
	int	dk2Len;
	unsigned char CbcCtx[DES_BLOCKLEN];
	//int	IsFirstBlock;
} DES3_CTX;


static unsigned char pc1[56] = {
	56, 48, 40, 32, 24, 16,  8,  0, 57, 49, 41, 33, 25, 17,
	 9,  1, 58, 50, 42, 34, 26, 18, 10,  2, 59, 51, 43, 35,
	62, 54, 46, 38, 30, 22, 14,  6, 61, 53, 45, 37, 29, 21,
	13,  5, 60, 52, 44, 36, 28, 20, 12,  4, 27, 19, 11,  3 };

static unsigned char pc2[48] = {
	13, 16, 10, 23,  0,  4,		 2, 27, 14,  5, 20,  9,
	22, 18, 11,  3, 25,  7, 	15,  6, 26, 19, 12,  1,
	40, 51, 30, 36, 46, 54,		29, 39, 50, 44, 32, 47,
	43, 48, 38, 55, 33, 52, 	45, 41, 49, 35, 28, 31 };

static unsigned short bytebit[8] = {0200,0100,040,020,010,04,02,01 };
static unsigned char totrot[16] = {1,2,4,6,8,10,12,14,15,17,19,21,23,25,27,28};
static unsigned long bigbyte[24] = {
	0x800000L,	0x400000L,	0x200000L,	0x100000L,
	0x80000L,	0x40000L,	0x20000L,	0x10000L,
	0x8000L,	0x4000L,	0x2000L,	0x1000L,
	0x800L,		0x400L,		0x200L,		0x100L,
	0x80L,		0x40L,		0x20L,		0x10L,
	0x8L,		0x4L,		0x2L,		0x1L	};

//insert digits
static unsigned long SP1[64] ={
       0x01010400l,0x00000000l,0x00010000l,0x01010404l,
       0x01010004l,0x00010404l,0x00000004l,0x00010000l,
       0x00000400l,0x01010400l,0x01010404l,0x00000400l,
       0x01000404l,0x01010004l,0x01000000l,0x00000004l,
       0x00000404l,0x01000400l,0x01000400l,0x00010400l,
       0x00010400l,0x01010000l,0x01010000l,0x01000404l,
       0x00010004l,0x01000004l,0x01000004l,0x00010004l,
       0x00000000l,0x00000404l,0x00010404l,0x01000000l,
       0x00010000l,0x01010404l,0x00000004l,0x01010000l,
       0x01010400l,0x01000000l,0x01000000l,0x00000400l,
       0x01010004l,0x00010000l,0x00010400l,0x01000004l,
       0x00000400l,0x00000004l,0x01000404l,0x00010404l,
       0x01010404l,0x00010004l,0x01010000l,0x01000404l,
       0x01000004l,0x00000404l,0x00010404l,0x01010400l,
       0x00000404l,0x01000400l,0x01000400l,0x00000000l,
       0x00010004l,0x00010400l,0x00000000l,0x01010004l };
       
       
static unsigned long SP2[64]={
       0x80108020l,0x80008000l,0x00008000l,0x00108020l,
       0x00100000l,0x00000020l,0x80100020l,0x80008020l,
       0x80000020l,0x80108020l,0x80108000l,0x80000000l,
       0x80008000l,0x00100000l,0x00000020l,0x80100020l,
       0x00108000l,0x00100020l,0x80008020l,0x00000000l,
       0x80000000l,0x00008000l,0x00108020l,0x80100000l,
       0x00100020l,0x80000020l,0x00000000l,0x00108000l,
       0x00008020l,0x80108000l,0x80100000l,0x00008020l,
       0x00000000l,0x00108020l,0x80100020l,0x00100000l,
       0x80008020l,0x80100000l,0x80108000l,0x00008000l,
       0x80100000l,0x80008000l,0x00000020l,0x80108020l,
       0x00108020l,0x00000020l,0x00008000l,0x80000000l,
       0x00008020l,0x80108000l,0x00100000l,0x80000020l,
       0x00100020l,0x80008020l,0x80000020l,0x00100020l,
       0x00108000l,0x00000000l,0x80008000l,0x00008020l,
       0x80000000l,0x80100020l,0x80108020l,0x00108000l };
       
       
static unsigned long SP3[64]={ 
       0x00000208l,0x08020200l,0x00000000l,0x08020008l,
       0x08000200l,0x00000000l,0x00020208l,0x08000200l,
       0x00020008l,0x08000008l,0x08000008l,0x00020000l,
       0x08020208l,0x00020008l,0x08020000l,0x00000208l,
       0x08000000l,0x00000008l,0x08020200l,0x00000200l,
       0x00020200l,0x08020000l,0x08020008l,0x00020208l,
       0x08000208l,0x00020200l,0x00020000l,0x08000208l,
       0x00000008l,0x08020208l,0x00000200l,0x08000000l,
       0x08020200l,0x08000000l,0x00020008l,0x00000208l,
       0x00020000l,0x08020200l,0x08000200l,0x00000000l,
       0x00000200l,0x00020008l,0x08020208l,0x08000200l,
       0x08000008l,0x00000200l,0x00000000l,0x08020008l,
       0x08000208l,0x00020000l,0x08000000l,0x08020208l,
       0x00000008l,0x00020208l,0x00020200l,0x08000008l,
       0x08020000l,0x08000208l,0x00000208l,0x08020000l,
       0x00020208l,0x00000008l,0x08020008l,0x00020200l };
       
       
static unsigned long SP4[64]={             
       0x00802001l,0x00002081l,0x00002081l,0x00000080l,
       0x00802080l,0x00800081l,0x00800001l,0x00002001l,
       0x00000000l,0x00802000l,0x00802000l,0x00802081l,
       0x00000081l,0x00000000l,0x00800080l,0x00800001l,
       0x00000001l,0x00002000l,0x00800000l,0x00802001l,
       0x00000080l,0x00800000l,0x00002001l,0x00002080l,
       0x00800081l,0x00000001l,0x00002080l,0x00800080l,
       0x00002000l,0x00802080l,0x00802081l,0x00000081l,
       0x00800080l,0x00800001l,0x00802000l,0x00802081l,
       0x00000081l,0x00000000l,0x00000000l,0x00802000l,
       0x00002080l,0x00800080l,0x00800081l,0x00000001l,
       0x00802001l,0x00002081l,0x00002081l,0x00000080l,
       0x00802081l,0x00000081l,0x00000001l,0x00002000l,
       0x00800001l,0x00002001l,0x00802080l,0x00800081l,
       0x00002001l,0x00002080l,0x00800000l,0x00802001l,
       0x00000080l,0x00800000l,0x00002000l,0x00802080l };
       
       
static unsigned long SP5[64]={   
       0x00000100l,0x02080100l,0x02080000l,0x42000100l,
       0x00080000l,0x00000100l,0x40000000l,0x02080000l,
       0x40080100l,0x00080000l,0x02000100l,0x40080100l,
       0x42000100l,0x42080000l,0x00080100l,0x40000000l,
       0x02000000l,0x40080000l,0x40080000l,0x00000000l,
       0x40000100l,0x42080100l,0x42080100l,0x02000100l,
       0x42080000l,0x40000100l,0x00000000l,0x42000000l,
       0x02080100l,0x02000000l,0x42000000l,0x00080100l,
       0x00080000l,0x42000100l,0x00000100l,0x02000000l,
       0x40000000l,0x02080000l,0x42000100l,0x40080100l,
       0x02000100l,0x40000000l,0x42080000l,0x02080100l,
       0x40080100l,0x00000100l,0x20000000l,0x42080000l,
       0x42080100l,0x00080100l,0x42000000l,0x42080100l,
       0x02080000l,0x02000100l,0x40000100l,0x00080000l,
       0x00080100l,0x02000100l,0x40000100l,0x00080000l,
       0x00000000l,0x40080000l,0x02080100l,0x40000100l };
       
       
static unsigned long SP6[64]={ 
       0x20000010l,0x20400000l,0x00004000l,0x20404010l,
       0x20400000l,0x00000010l,0x20404010l,0x00400000l,
       0x20004000l,0x00404010l,0x00400000l,0x20000010l,
       0x00400010l,0x20004000l,0x20000000l,0x00004010l,
       0x00000000l,0x00400010l,0x20004010l,0x00004000l,
       0x00404000l,0x20004010l,0x00000010l,0x20400010l,
       0x20400010l,0x00000000l,0x00404010l,0x20404000l,
       0x00004010l,0x00404000l,0x20404000l,0x20000000l,
       0x20004000l,0x00000010l,0x20400010l,0x00404000l,
       0x20404010l,0x00400000l,0x00004010l,0x20000010l,
       0x00400000l,0x20004000l,0x20000000l,0x00004010l,
       0x20000010l,0x20404010l,0x00404000l,0x20400000l,
       0x00404010l,0x20404000l,0x00000000l,0x20400010l,
       0x00000010l,0x00004000l,0x20400000l,0x00404010l,
       0x00004000l,0x00400010l,0x20004010l,0x00000000l,
       0x20404000l,0x20000000l,0x00400010l,0x20004010l };  
            
static unsigned long SP7[64] = {
	0x00200000L, 0x04200002L, 0x04000802L, 0x00000000L,
	0x00000800L, 0x04000802L, 0x00200802L, 0x04200800L,
	0x04200802L, 0x00200000L, 0x00000000L, 0x04000002L,
	0x00000002L, 0x04000000L, 0x04200002L, 0x00000802L,
	0x04000800L, 0x00200802L, 0x00200002L, 0x04000800L,
	0x04000002L, 0x04200000L, 0x04200800L, 0x00200002L,
	0x04200000L, 0x00000800L, 0x00000802L, 0x04200802L,
	0x00200800L, 0x00000002L, 0x04000000L, 0x00200800L,
	0x04000000L, 0x00200800L, 0x00200000L, 0x04000802L,
	0x04000802L, 0x04200002L, 0x04200002L, 0x00000002L,
	0x00200002L, 0x04000000L, 0x04000800L, 0x00200000L,
	0x04200800L, 0x00000802L, 0x00200802L, 0x04200800L,
	0x00000802L, 0x04000002L, 0x04200802L, 0x04200000L,
	0x00200800L, 0x00000000L, 0x00000002L, 0x04200802L,
	0x00000000L, 0x00200802L, 0x04200000L, 0x00000800L,
	0x04000002L, 0x04000800L, 0x00000800L, 0x00200002L };
	
static unsigned long SP8[64] = {
	0x10001040L, 0x00001000L, 0x00040000L, 0x10041040L,
	0x10000000L, 0x10001040L, 0x00000040L, 0x10000000L,
	0x00040040L, 0x10040000L, 0x10041040L, 0x00041000L,
	0x10041000L, 0x00041040L, 0x00001000L, 0x00000040L,
	0x10040000L, 0x10000040L, 0x10001000L, 0x00001040L,
	0x00041000L, 0x00040040L, 0x10040040L, 0x10041000L,
	0x00001040L, 0x00000000L, 0x00000000L, 0x10040040L,
	0x10000040L, 0x10001000L, 0x00041040L, 0x00040000L,
	0x00041040L, 0x00040000L, 0x10041000L, 0x00001000L,
	0x00000040L, 0x10040040L, 0x00001000L, 0x00041040L,
	0x10001000L, 0x00000040L, 0x10000040L, 0x10040000L,
	0x10040040L, 0x10000000L, 0x00040000L, 0x10001040L,
	0x00000000L, 0x10041040L, 0x00040040L, 0x10000040L,
	0x10040000L, 0x10001000L, 0x10001040L, 0x00000000L,
	0x10041040L, 0x00041000L, 0x00041000L, 0x00001040L,
	0x00001040L, 0x00040040L, 0x10000000L, 0x10041000L };

void deskey(unsigned char *key,short edf, unsigned long *kn);
void cookey(register unsigned long *raw1, unsigned long *dough);
//void cpkey(register unsigned long *into);
//void usekey(register unsigned long *from);
//void des(unsigned char *inblock,unsigned char *outblock);
void scrunch(register unsigned char *outof, register unsigned long *into);
void unscrun(register unsigned long *outof, register unsigned char *into);
void desfunc(register unsigned long *block,register unsigned long *keys);

/*****************  DES Function  *****************/
unsigned long OPENCOMM_DesExpandEncKey(
		unsigned char *pbDesKey,
		unsigned long  ulDesKeyLen,
		unsigned char *pbDesEncKey,
		unsigned long *ulDesEncKeyLen);

unsigned long OPENCOMM_DesExpandDecKey(
		unsigned char *pbDesKey,
		unsigned long  ulDesKeyLen,
		unsigned char *pbDesDecKey,
		unsigned long *ulDesDecKeyLen);

unsigned long OPENCOMM_DesEncRaw(
		unsigned char *pbDesEncKey,
		unsigned long  ulDesEncKeyLen,
		unsigned char *pbInData,
		unsigned long  ulInDataLen,
		unsigned char *pbOutData,
		unsigned long *ulOutDataLen);

unsigned long OPENCOMM_DesDecRaw(
		unsigned char *pbDesDecKey,
		unsigned long  ulDesDecKeyLen,
		unsigned char *pbInData,
		unsigned long  ulInDataLen,
		unsigned char *pbOutData,
		unsigned long *ulOutDataLen);


int myic_DESDecrypt(
		unsigned char *pDesKey,
		int            nDesKeyLen,
		unsigned char *pInData,
		int            nInDataLen,
		unsigned char *pOutData,
		int           *pOutDataLen);

int myic_DESEncrypt(
		unsigned char *pDesKey,
		int            nDesKeyLen,
		unsigned char *pInData,
		int            nInDataLen,
		unsigned char *pOutData,
		int           *pOutDataLen);


void deskey(unsigned char *key,short edf, unsigned long *kn)
{
	register int i, j, l, m, n;
	unsigned long pc1m[56],pcr[56];
	
	
	for ( j = 0; j < 56; j++ ) 
	{
		l = pc1[j];
		m = l & 07;
		pc1m[j] = (((unsigned long) key[l >> 3] & (unsigned long)bytebit[m] ) ? 1:0);
	}
	for ( i = 0;i < 16; i++)
	{
		if ( edf == DE1 )	m = (15 - i) << 1;
		else	m = i << 1;
		n = m + 1;
		kn[m] = kn[n] = 0L;
		for ( j = 0; j < 28; j++ )
		{
			l = j + totrot[i];
			if ( l < 28 )	pcr[j] = pc1m[l];
			else	pcr[j] = pc1m[l-28];
		}
		for (j = 28; j < 56; j++ ) 
		{
			l = j + totrot[i];
			if ( l < 56 )	pcr[j] = pc1m[l];
			else	pcr[j] = pc1m[l-28];
		} 
		for ( j = 0; j < 24; j++ ) 
		{
			if ( pcr[pc2[j]] )	kn[m] |= bigbyte[j];
			if ( pcr[pc2[j+24]] )	kn[n] |= bigbyte[j];
		}
	}
	return;
}

void cookey(register unsigned long *raw1, unsigned long *dough)
{
	register unsigned long *cook,*raw0;
	register int i;
	
	cook = dough;
	for ( i = 0; i < 16; i++, raw1++ ) {
		raw0 = raw1++;
		*cook	 = (*raw0 & 0x00fc0000L) << 6;
		*cook	|= (*raw0 & 0x00000fc0L) << 10;
		*cook	|= (*raw1 & 0x00fc0000L) >> 10;
		*cook++	|= (*raw1 & 0x00000fc0L) >> 6;
		*cook	 = (*raw0 & 0x0003f000L) << 12;
		*cook	|= (*raw0 & 0x0000003fL) << 16;
		*cook	|= (*raw1 & 0x0003f000L) >> 4;
		*cook++	|= (*raw1 & 0x0000003fL);
	}
	return;
}

void scrunch(register unsigned char *outof, register unsigned long *into)
{
	*into	 = (*outof++ & 0xffL) << 24;
	*into	|= (*outof++ & 0xffL) << 16;
	*into	|= (*outof++ & 0xffL) << 8;
	*into++	|= (*outof++ & 0xffL);
	*into	 = (*outof++ & 0xffL) << 24;
	*into	|= (*outof++ & 0xffL) << 16;
	*into	|= (*outof++ & 0xffL) << 8;
	*into++	|= (*outof   & 0xffL);
	return;
}

void unscrun(register unsigned long *outof, register unsigned char *into)
{
	*into++	 = (unsigned char)((*outof >> 24) & 0xffL);
	*into++	 = (unsigned char)((*outof >> 16) & 0xffL);
	*into++	 = (unsigned char)((*outof >>  8) & 0xffL);
	*into++	 = (unsigned char)( *outof++	  & 0xffL);
	*into++	 = (unsigned char)((*outof >> 24) & 0xffL);
	*into++	 = (unsigned char)((*outof >> 16) & 0xffL);
	*into++	 = (unsigned char)((*outof >>  8) & 0xffL);
	*into	 = (unsigned char)( *outof		  & 0xffL);
	return;
}

void desfunc(register unsigned long *block,register unsigned long *keys)
{
	register unsigned long fval, work, right, leftt;
	register int round;
	
	leftt = block[0];
	right = block[1];
	work = ((leftt >> 4) ^ right) & 0x0f0f0f0fL;
	
	right ^= work;
	leftt ^= (work << 4);
	work = ((leftt >> 16) ^ right) & 0x0000ffffL;
	
	right ^= work;
	leftt ^= (work << 16);
	work = ((right >> 2) ^ leftt) & 0x33333333L;
	
	leftt ^= work;
	right ^= (work << 2);
	work = ((right >> 8) ^ leftt) & 0x00ff00ffL;
	
	leftt ^= work;
	right ^= (work << 8);
	right = ((right << 1) | ((right >>31) & 1L)) & 0xffffffffL;
	work = (leftt ^ right) & 0xaaaaaaaaL;
	
	leftt ^= work;
	right ^= work;
	leftt = ((leftt << 1) | ((leftt >> 31)&1L)) & 0xffffffffL;
	
	for (round = 0; round < 8; round++) {
		work  = (right << 28) | (right >> 4);
		work ^= *keys++;
		fval  = SP7[ work	& 0x3fL];
		fval |= SP5[(work >>  8) & 0x3fL];
		fval |= SP3[(work >> 16) & 0x3fL];
		fval |= SP1[(work >> 24) & 0x3fL];
		work  = right ^ *keys++;
		fval |= SP8[ work 	& 0x3fL];
		fval |= SP6[(work >>  8) & 0x3fL];
		fval |= SP4[(work >> 16) & 0x3fL];
		fval |= SP2[(work >> 24) & 0x3fL];
		leftt ^= fval;
		work  = (leftt << 28) | (leftt >> 4);
		work ^= *keys++;
		fval  = SP7[ work 	& 0x3fL];
		fval |= SP5[(work >>  8) & 0x3fL];
		fval |= SP3[(work >> 16) & 0x3fL];
		fval |= SP1[(work >> 24) & 0x3fL];
		work  = leftt ^ *keys++;
		fval |= SP8[ work 	& 0x3fL];
		fval |= SP6[(work >>  8) & 0x3fL];
		fval |= SP4[(work >> 16) & 0x3fL];
		fval |= SP2[(work >> 24) & 0x3fL];
		right ^= fval;
	}
	
	right = (right << 31) | (right >> 1);
	work = (leftt ^ right) & 0xaaaaaaaaL;
	leftt ^= work;
	right ^= work;
	leftt = (leftt << 31) | (leftt >> 1);
	work = ((leftt >>  8) ^ right) & 0x00ff00ffL;
	right ^= work;
	leftt ^= (work << 8);
	work = ((leftt >>  2) ^ right) & 0x33333333L;
	right ^= work;
	leftt ^= (work << 2);
	work = ((right >> 16) ^ leftt) & 0x0000ffffL;
	leftt ^= work;
	right ^= (work << 16);
	work = ((right >>  4) ^ leftt) & 0x0f0f0f0fL;
	leftt ^= work;
	right ^= (work << 4);
	*block++ = right;
	*block = leftt;
	return;
}

/*****************************************************************
	OPENCOMM_DesExpandEncKey	: Expand Des Enc Key 扩展des加密密钥
	Return value:
		0         : Success
		other     : failed
	Parameters:
		pbDesKey        : 扩展前的DES密钥(8字节)       input
		ulDesKeyLen     : 扩展前的DES密钥长度          input
		pbDesEncKey     : 扩展后的DES加密密钥(128字节)  output
		*ulDesEncKeyLen : 扩展后的DES加密密钥长度       output
*****************************************************************/
unsigned long OPENCOMM_DesExpandEncKey(
		unsigned char *pbDesKey,
		unsigned long  ulDesKeyLen,
		unsigned char *pbDesEncKey,
		unsigned long *ulDesEncKeyLen)
{
	unsigned long kn[32], dough[32];
	
	if (ulDesKeyLen != 8)
		return 0xEE20;

	deskey(pbDesKey, EN0, kn);
	cookey(kn, dough);
	*ulDesEncKeyLen = DES_KEYBYTES;  //32 long = 128 bytes
	memcpy(pbDesEncKey, dough, *ulDesEncKeyLen);
	
	return 0;
}

/*****************************************************************
	OPENCOMM_DesExpandDecKey	: Expand Des Dec Key 扩展des解密密钥
	Return value:
		0       : Success
		other   : failed
	Parameters:
		pbDesKey        : 扩展前的DES密钥(8字节)      input
		ulDesKeyLen     : 扩展前的DES密钥长度         input
		pbDesDecKey     : 扩展后的DES解密密钥(128字节) output
		*ulDesDecKeyLen : 扩展后的DES解密密钥长度      output
*****************************************************************/
unsigned long OPENCOMM_DesExpandDecKey(
		unsigned char *pbDesKey,
		unsigned long  ulDesKeyLen,
		unsigned char *pbDesDecKey,
		unsigned long *ulDesDecKeyLen)
{
	unsigned long kn[32], dough[32];
	
	if (ulDesKeyLen != 8)
		return 0xEE20;

	deskey(pbDesKey, DE1, kn);
	cookey(kn, dough);
	*ulDesDecKeyLen = DES_KEYBYTES;  //32 long = 128 bytes
	memcpy(pbDesDecKey, dough, *ulDesDecKeyLen);
	
	return 0;
}

/****************************************************************
	OPENCOMM_DesEncRaw		: Des算法加密小整块明文8字节 
	Return value:
		0       : Success
		other   : failed
	Parameters:
		pbDesEncKey    : DES加密密钥    input
		ulDesEncKeyLen : DES加密密钥长度 input
		pbInData       : 待加密的明文    input
		ulInDataLen    : 待加密的明文长度 input
		pbOutData      : 加密后的密文    output
		*ulOutDataLen  : 加密后的密文长度 output
*****************************************************************/
unsigned long OPENCOMM_DesEncRaw(
		unsigned char *pbDesEncKey,
		unsigned long  ulDesEncKeyLen,
		unsigned char *pbInData,
		unsigned long  ulInDataLen,
		unsigned char *pbOutData,
		unsigned long *ulOutDataLen)
{
	unsigned long work[2], ek[DES_KEYLONGS];
	unsigned char cp[DES_BLOCKLEN];

	if (ulInDataLen != DES_BLOCKLEN)
		return 0xEE20;
	
	if (ulDesEncKeyLen != DES_KEYBYTES)
		return 0xEE20;

	memcpy(cp, pbInData, DES_BLOCKLEN);
	scrunch(cp,work);  // 8 bytes -> 2 long
	memcpy(ek, pbDesEncKey, ulDesEncKeyLen);
	desfunc(work,ek);
	unscrun(work,cp); // 2 long -> 8 bytes
	memcpy(pbOutData, cp, DES_BLOCKLEN);
	*ulOutDataLen = DES_BLOCKLEN;

	return 0;
}

/*****************************************************************
	OPENCOMM_DesDecRaw : Des算法解密小整块密文8字节 
	Return value:
		0     : Success
		other : failed
	Parameters:
		pbDesDecKey    : DES解密密钥     input
		ulDesDecKeyLen : DES解密密钥长度  input
		pbInData       : 待解密的密文     input
		ulInDataLen    : 待解密的密文长度  input
		pbOutData      : 解密后的明文     output
		*ulOutDataLen  : 解密后的明文长度  output
*****************************************************************/
unsigned long OPENCOMM_DesDecRaw(
		unsigned char *pbDesDecKey,
		unsigned long  ulDesDecKeyLen,
		unsigned char *pbInData,
		unsigned long  ulInDataLen,
		unsigned char *pbOutData,
		unsigned long *ulOutDataLen)
{
	unsigned long work[2], dk[DES_KEYLONGS];
	unsigned char cp[DES_BLOCKLEN];

	if (ulInDataLen != DES_BLOCKLEN)
		return 0xEE20;
	
	if (ulDesDecKeyLen != DES_KEYBYTES)
		return 0xEE20;

	memcpy(cp, pbInData, DES_BLOCKLEN);
	scrunch(cp,work);  // 8 bytes -> 2 long
	memcpy(dk, pbDesDecKey, ulDesDecKeyLen);
	desfunc(work,dk);
	unscrun(work,cp); // 2 long -> 8 bytes
	memcpy(pbOutData, cp, DES_BLOCKLEN);
//	des_enc(pbDesEncKey, pbInData, pbOutData);
	*ulOutDataLen = DES_BLOCKLEN;

	return 0;
}

/*********************   DES    *********************/

int myic_DESEncrypt(
		unsigned char *pDesKey,
		int            nDesKeyLen,
		unsigned char *pInData,
		int            nInDataLen,
		unsigned char *pOutData,
		int           *pOutDataLen)
{
	unsigned char DesKeyBuf[32];
	unsigned char DesEncKeyBuf[128];
	int EncKeyLen, KeyLen = 0;
	int retval = 0, loops, i;
	
	if(nInDataLen%8 != 0)
		return 0xEE20;
	
	if(nDesKeyLen != 8)
		return 0xEE20;
	KeyLen = nDesKeyLen;
	memcpy(DesKeyBuf, pDesKey, nDesKeyLen);

	
	retval = OPENCOMM_DesExpandEncKey(DesKeyBuf, KeyLen,
		DesEncKeyBuf, (unsigned long *)&EncKeyLen);
	if(retval != 0)
		return retval;

	loops = nInDataLen/8;
	for(i = 0; i < loops; i++)
	{
		retval = OPENCOMM_DesEncRaw(DesEncKeyBuf, EncKeyLen, pInData + i*8,
			8, pOutData + i*8, (unsigned long *)pOutDataLen);
		if(retval != 0)
			return retval;
	}
	*pOutDataLen = nInDataLen;
	return retval;
}


int myic_DESDecrypt(
		unsigned char *pDesKey,
		int            nDesKeyLen,
		unsigned char *pInData,
		int            nInDataLen,
		unsigned char *pOutData,
		int           *pOutDataLen)
{
	unsigned char DesKeyBuf[32];
	unsigned char DesDecKeyBuf[128];
	int DecKeyLen, KeyLen = 0;
	int retval = 0, loops, i;
	
	if(nInDataLen%8 != 0)
		return 0xEE20;
	
	if(nDesKeyLen != 8)
		return 0xEE20;
	KeyLen = nDesKeyLen;
	memcpy(DesKeyBuf, pDesKey, nDesKeyLen);

	retval = OPENCOMM_DesExpandDecKey(DesKeyBuf, KeyLen,
		DesDecKeyBuf, (unsigned long *)&DecKeyLen);
	if(retval != 0)
		return retval;
	
	loops = nInDataLen/8;
	for(i = 0; i < loops; i++)
	{
		retval = OPENCOMM_DesDecRaw(DesDecKeyBuf, DecKeyLen, pInData + i*8,
			8, pOutData + i*8, (unsigned long *)pOutDataLen);
		if(retval != 0)
			return retval;
	}
	*pOutDataLen = nInDataLen;
	return retval;
}


//对称明文数据打pading
void  CW_dataPadAdd(int tag, unsigned char *date, unsigned int dateLen, 
					unsigned char **padDate, unsigned int *padDateLen)
{
	int           i, padLen;
	unsigned char *pTmp   = NULL;
	
	pTmp = (unsigned char *)malloc(dateLen+24);
	if (pTmp == NULL)
	{
		*padDate = NULL;
		return ;
	}
	memset(pTmp, 0, dateLen+24);
	memcpy(pTmp, date, dateLen);
	
	if (tag == 0)
	{
		padLen = 8 - dateLen % 8;
		for (i=0; i<padLen; i++)
		{
			pTmp[dateLen+i] = (char)padLen;
		}
		*padDateLen = dateLen + padLen;
	}
	else
	{
		padLen = 16 - dateLen % 16;
		for (i=0; i<padLen; i++)
		{
			pTmp[dateLen+i] = (char)padLen;
		}		
	}
	
	*padDateLen = dateLen + padLen;	
	*padDate = pTmp;
}

#define  USER_PASSWORD_KEY "abcd1234"


//数据加密
int DesEnc(
		 unsigned char *pInData,
		 int            nInDataLen,
		 unsigned char *pOutData,
		 int           *pOutDataLen)
{
	int				rv;
	unsigned char	*padDate = NULL;
	unsigned int	padDateLen = 0;

	CW_dataPadAdd(0, pInData, (unsigned int )nInDataLen, &padDate, &padDateLen);

	rv = myic_DESEncrypt((unsigned char *)USER_PASSWORD_KEY, strlen(USER_PASSWORD_KEY),
		padDate, (int)padDateLen, pOutData, pOutDataLen);
	if (rv != 0)
	{
		if (padDate != NULL)
		{
			free(padDate);
		}
		return rv;	
	}

	if (padDate != NULL)
	{
		free(padDate);
	}
	return 0;
}


//数据加密
int DesEnc_raw(
	unsigned char *pInData,
	int            nInDataLen,
	unsigned char *pOutData,
	int           *pOutDataLen)
{
	int				rv;
//	unsigned char	*padDate = NULL;
//	unsigned int	padDateLen = 0;

	rv = myic_DESEncrypt((unsigned char *)USER_PASSWORD_KEY, strlen(USER_PASSWORD_KEY),
		pInData, (int)nInDataLen, pOutData, pOutDataLen);
	if (rv != 0)
	{
		return rv;	
	}
	return 0;
}

//解密分配内存错误
#define  ERR_MALLOC 20
//密码长度不是8的整数倍, 不合法
#define  ERR_FILECONT 20


//用户使用函数des解密
int DesDec(
		   unsigned char *pInData,
		   int            nInDataLen,
		   unsigned char *pOutData,
		   int           *pOutDataLen)
{
	int				rv;
	char			padChar;
	unsigned char 	*tmpPlain = NULL;
	
	tmpPlain =		(unsigned char *)malloc(nInDataLen+24);
	if (tmpPlain == NULL)
	{
		return ERR_MALLOC;
	}
	memset(tmpPlain, 0, nInDataLen+24);

	//解密
	rv = myic_DESDecrypt((unsigned char *)USER_PASSWORD_KEY, strlen(USER_PASSWORD_KEY),
		pInData, nInDataLen, tmpPlain, pOutDataLen);
	if (rv != 0)
	{
		if (tmpPlain != NULL) free(tmpPlain);
		return rv;
	}

	//去pading
	padChar = tmpPlain[*pOutDataLen - 1];
	if ( (int)padChar<=0 || (int)padChar>8) //异常处理
	{
		if (tmpPlain) free(tmpPlain);
		return ERR_FILECONT;
	}

	*pOutDataLen = *pOutDataLen - (int)padChar;	
	//memset(tmpPlain + *pOutDataLen, 0, (int)padChar);	
	memcpy(pOutData, tmpPlain, *pOutDataLen);
	if (tmpPlain) free(tmpPlain);	
	return 0;
}


//用户使用函数des解密
int DesDec_raw(
	unsigned char *pInData,
	int            nInDataLen,
	unsigned char *pOutData,
	int           *pOutDataLen)
{
	int				rv;
	//char			padChar;
	//unsigned char 	*tmpPlain = NULL;

	/*
	tmpPlain =		(unsigned char *)malloc(nInDataLen+24);
	if (tmpPlain == NULL)
	{
		return ERR_MALLOC;
	}
	memset(tmpPlain, 0, nInDataLen+24);
	*/

	//解密
	rv = myic_DESDecrypt((unsigned char *)USER_PASSWORD_KEY, strlen(USER_PASSWORD_KEY),
		pInData, nInDataLen, pOutData, pOutDataLen);
	if (rv != 0)
	{
		//if (tmpPlain != NULL) free(tmpPlain);
		return rv;
	}
	/*
	//去pading
	padChar = tmpPlain[*pOutDataLen - 1];
	if ( (int)padChar<=0 || (int)padChar>8) //异常处理
	{
		if (tmpPlain) free(tmpPlain);
		return ERR_FILECONT;
	}

	*pOutDataLen = *pOutDataLen - (int)padChar;	
	//memset(tmpPlain + *pOutDataLen, 0, (int)padChar);	
	memcpy(pOutData, tmpPlain, *pOutDataLen);
	if (tmpPlain) free(tmpPlain);	
	*/
	return 0;
}



