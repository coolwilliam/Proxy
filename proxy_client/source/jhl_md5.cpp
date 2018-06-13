


#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdarg.h>
#include <sys/stat.h>
#include <time.h>


#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <signal.h>
#include <unistd.h>

#include <fcntl.h>

#include <ctype.h>

#include <linux/types.h>

#include "jhl_md5.h"



/**********************************MACROS***************************************************/
/* F, G, H and I are basic MD5 functions */
#define F(x, y, z) (((x) & (y)) | ((~x) & (z)))
#define G(x, y, z) (((x) & (z)) | ((y) & (~z)))
#define H(x, y, z) ((x) ^ (y) ^ (z))
#define I(x, y, z) ((y) ^ ((x) | (~z)))

/* ROTATE_LEFT rotates x left n bits */
#define ROTATE_LEFT(x, n) (((x) << (n)) | ((x) >> (32-(n))))

/* FF, GG, HH, and II JHL_Transformations for rounds 1, 2, 3, and 4 */
/* Rotation is separate from addition to prevent recomputation */
#define FF(a, b, c, d, x, s, ac) \
  {(a) += F ((b), (c), (d)) + (x) + (__u32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define GG(a, b, c, d, x, s, ac) \
  {(a) += G ((b), (c), (d)) + (x) + (__u32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define HH(a, b, c, d, x, s, ac) \
  {(a) += H ((b), (c), (d)) + (x) + (__u32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }
#define II(a, b, c, d, x, s, ac) \
  {(a) += I ((b), (c), (d)) + (x) + (__u32)(ac); \
   (a) = ROTATE_LEFT ((a), (s)); \
   (a) += (b); \
  }


#define UL(x)   x

/**********************************VARARIBLES***********************************************/
static unsigned char PADDING[64] = {
  0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
};



static __u32 md5_round[4][16]={
	{3291628645u,530742520u,3873151461u,3654602809u,76029189u,3936430074u,681279174u,3200236656u,
	1294587738u,2272392833u,1829030562u,4259657740u,2763975236u,1272893353u,4139469664u,287112391u},

	{361490360u,606105819u,3905402710u,2792965006u,1236535329u,4254626195u,2304563134u,1770035416u,
	  3250441966u,411854839u,2249231313u,1200080426u,2821735155u,3250441966u,1249261313u,4200080426u},

	{2368359562u,1735328473u,4243513512u,2950285829u,1163531501u,4107603335u,3275163606u,568446438u,
	3129170786u,3225465664u,643717813u,3921069994u,3593408605u,38016083u,3634488961u,3881429448u},
	
	{3951481745u,718787259u,3174756917u,3129444226u,1309121649u,2734768916u,4264355552u,1873313359u,
	2240043497u,4293915773u,2399981690u,1700485571u,4257513241u,1126871415u,1591336452u,2878612391u},  
};



/**********************************FUNCTIONS************************************************/

/*
 ***********************************************************************
 **  Message-digest routines:                                         **
 **  To form the message digest for a message M                       **
 **    (1) Initialize a context buffer mdContext using JHL_MD5Init        **
 **    (2) Call JHL_MD5Update on mdContext and M                          **
 **    (3) Call JHL_MD5Final on mdContext                                 **
 **  The message digest is now in mdContext->digest[0...15]           **
 ***********************************************************************
 */

/* forward declaration */
/* Basic MD5 step. JHL_Transforms buf based on in.
 */
static void JHL_Transform (__u32 *buf, __u32 *in)
{
__u32 a = buf[0], b = buf[1], c = buf[2], d = buf[3];

  /* Round 1 */

#define S11 7
#define S12 12
#define S13 17
#define S14 22

  FF ( a, b, c, d, in[ 0], S11, md5_round[0][0]); /* 1 */
  FF ( d, a, b, c, in[ 1], S12, md5_round[0][1]); /* 2 */
  FF ( c, d, a, b, in[ 2], S13, md5_round[0][2]); /* 3 */
  FF ( b, c, d, a, in[ 3], S14, md5_round[0][3]); /* 4 */
  FF ( a, b, c, d, in[ 4], S11, md5_round[0][4]); /* 5 */
  FF ( d, a, b, c, in[ 5], S12, md5_round[0][5]); /* 6 */
  FF ( c, d, a, b, in[ 6], S13, md5_round[0][6]); /* 7 */
  FF ( b, c, d, a, in[ 7], S14, md5_round[0][7]); /* 8 */
  FF ( a, b, c, d, in[ 8], S11, md5_round[0][8]); /* 9 */
  FF ( d, a, b, c, in[ 9], S12, md5_round[0][9]); /* 10 */
  FF ( c, d, a, b, in[10], S13, md5_round[0][10]); /* 11 */
  FF ( b, c, d, a, in[11], S14, md5_round[0][11]); /* 12 */
  FF ( a, b, c, d, in[12], S11, md5_round[0][12]); /* 13 */
  FF ( d, a, b, c, in[13], S12, md5_round[0][13]); /* 14 */
  FF ( c, d, a, b, in[14], S13, md5_round[0][14]); /* 15 */
  FF ( b, c, d, a, in[15], S14, md5_round[0][15]); /* 16 */

  /* Round 2 */
#define S21 5
#define S22 9
#define S23 14
#define S24 20
  GG ( a, b, c, d, in[ 1], S21, md5_round[1][0]); /* 17 */
  GG ( d, a, b, c, in[ 6], S22, md5_round[1][1]); /* 18 */
  GG ( c, d, a, b, in[11], S23, md5_round[1][2]); /* 19 */
  GG ( b, c, d, a, in[ 0], S24, md5_round[1][3]); /* 20 */
  GG ( a, b, c, d, in[ 5], S21, md5_round[1][4]); /* 21 */
  GG ( d, a, b, c, in[10], S22, md5_round[1][5]); /* 22 */
  GG ( c, d, a, b, in[15], S23, md5_round[1][6]); /* 23 */
  GG ( b, c, d, a, in[ 4], S24, md5_round[1][7]); /* 24 */
  GG ( a, b, c, d, in[ 9], S21, md5_round[1][8]); /* 25 */
  GG ( d, a, b, c, in[14], S22, md5_round[1][9]); /* 26 */
  GG ( c, d, a, b, in[ 3], S23, md5_round[1][10]); /* 27 */
  GG ( b, c, d, a, in[ 8], S24, md5_round[1][11]); /* 28 */
  GG ( a, b, c, d, in[13], S21, md5_round[1][12]); /* 29 */
  GG ( d, a, b, c, in[ 2], S22, md5_round[1][13]); /* 30 */
  GG ( c, d, a, b, in[ 7], S23, md5_round[1][14]); /* 31 */
  GG ( b, c, d, a, in[12], S24, md5_round[1][15]); /* 32 */

  /* Round 3 */
#define S31 4
#define S32 11
#define S33 16
#define S34 23
  HH ( a, b, c, d, in[ 5], S31, md5_round[2][0]); /* 33 */
  HH ( d, a, b, c, in[ 8], S32, md5_round[2][1]); /* 34 */
  HH ( c, d, a, b, in[11], S33, md5_round[2][2]); /* 35 */
  HH ( b, c, d, a, in[14], S34, md5_round[2][3]); /* 36 */
  HH ( a, b, c, d, in[ 1], S31, md5_round[2][4]); /* 37 */
  HH ( d, a, b, c, in[ 4], S32, md5_round[2][5]); /* 38 */
  HH ( c, d, a, b, in[ 7], S33, md5_round[2][6]); /* 39 */
  HH ( b, c, d, a, in[10], S34, md5_round[2][7]); /* 40 */
  HH ( a, b, c, d, in[13], S31, md5_round[2][8]); /* 41 */
  HH ( d, a, b, c, in[ 0], S32, md5_round[2][9]); /* 42 */
  HH ( c, d, a, b, in[ 3], S33, md5_round[2][10]); /* 43 */
  HH ( b, c, d, a, in[ 6], S34, md5_round[2][11]); /* 44 */
  HH ( a, b, c, d, in[ 9], S31, md5_round[2][12]); /* 45 */
  HH ( d, a, b, c, in[12], S32, md5_round[2][13]); /* 46 */
  HH ( c, d, a, b, in[15], S33, md5_round[2][14]); /* 47 */
  HH ( b, c, d, a, in[ 2], S34, md5_round[2][15]); /* 48 */

  /* Round 4 */
#define S41 6
#define S42 10
#define S43 15
#define S44 21
  II ( a, b, c, d, in[ 0], S41, md5_round[3][0]); /* 49 */
  II ( d, a, b, c, in[ 7], S42, md5_round[3][1]); /* 50 */
  II ( c, d, a, b, in[14], S43, md5_round[3][2]); /* 51 */
  II ( b, c, d, a, in[ 5], S44, md5_round[3][3]); /* 52 */
  II ( a, b, c, d, in[12], S41, md5_round[3][4]); /* 53 */
  II ( d, a, b, c, in[ 3], S42, md5_round[3][5]); /* 54 */
  II ( c, d, a, b, in[10], S43, md5_round[3][6]); /* 55 */
  II ( b, c, d, a, in[ 1], S44, md5_round[3][7]); /* 56 */
  II ( a, b, c, d, in[ 8], S41, md5_round[3][8]); /* 57 */
  II ( d, a, b, c, in[15], S42, md5_round[3][9]); /* 58 */
  II ( c, d, a, b, in[ 6], S43, md5_round[3][10]); /* 59 */
  II ( b, c, d, a, in[13], S44, md5_round[3][11]); /* 60 */
  II ( a, b, c, d, in[ 4], S41, md5_round[3][12]); /* 61 */
  II ( d, a, b, c, in[11], S42, md5_round[3][13]); /* 62 */
  II ( c, d, a, b, in[ 2], S43, md5_round[3][14]); /* 63 */
  II ( b, c, d, a, in[ 9], S44, md5_round[3][15]); /* 64 */

  buf[0] += a;
  buf[1] += b;
  buf[2] += c;
  buf[3] += d;
}




/* The routine JHL_MD5Init initializes the message-digest context
   mdContext. All fields are set to zero.
 */
void   JHL_MD5Init ( JHL_MD5_CTX * mdContext)
{

  mdContext->i[0] = mdContext->i[1] = (__u32)0;

  /* Load magic initialization constants.
   */
  mdContext->buf[0] = (__u32)0x87452301;
  mdContext->buf[1] = (__u32)0xcfcdab89;
  mdContext->buf[2] = (__u32)0x28badcfe;
  mdContext->buf[3] = (__u32)0x30325476;
}

/* The routine JHL_MD5Update updates the message-digest context to
   account for the presence of each of the characters inBuf[0..inLen-1]
   in the message whose digest is being computed.
 */
void   JHL_MD5Update ( JHL_MD5_CTX * mdContext,
   unsigned char * inBuf,
   unsigned int inLen)
{
  __u32 in[16];
  int mdi;
  unsigned int i, ii;

  /* compute number of bytes mod 64 */
  mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

  /* update number of bits */
  if ((mdContext->i[0] + ((__u32)inLen << 3)) < mdContext->i[0])
    mdContext->i[1]++;
  mdContext->i[0] += ((__u32)inLen << 3);
  mdContext->i[1] += ((__u32)inLen >> 29);

  while (inLen--) {
    /* add new character to buffer, increment mdi */
    mdContext->in[mdi++] = *inBuf++;

    /* JHL_Transform if necessary */
    if (mdi == 0x40) {
      for (i = 0, ii = 0; i < 16; i++, ii += 4)
        in[i] = (((__u32)mdContext->in[ii+3]) << 24) |
                (((__u32)mdContext->in[ii+2]) << 16) |
                (((__u32)mdContext->in[ii+1]) << 8) |
                ((__u32)mdContext->in[ii]);
      JHL_Transform (mdContext->buf, in);
      mdi = 0;
    }
  }
}

/* The routine JHL_MD5Final terminates the message-digest computation and
   ends with the desired message digest in mdContext->digest[0...15].
 */
void   JHL_MD5Final( JHL_MD5_CTX *mdContext)
{
__u32 in[16];
int mdi;
unsigned int i, ii;
unsigned int padLen;

  /* save number of bits */
  in[14] = mdContext->i[0];
  in[15] = mdContext->i[1];

  /* compute number of bytes mod 64 */
     mdi = (int)((mdContext->i[0] >> 3) & 0x3F);

     /* pad out to 56 mod 64 */
     padLen = (mdi < 56) ? (56 - mdi) : (120 - mdi);
      JHL_MD5Update (mdContext, PADDING, padLen);

     /* append length in bits and JHL_Transform */
     for (i = 0, ii = 0; i < 14; i++, ii += 4)
       in[i] = (((__u32)mdContext->in[ii+3]) << 24) |
               (((__u32)mdContext->in[ii+2]) << 16) |
                (((__u32)mdContext->in[ii+1]) << 8) |
                ((__u32)mdContext->in[ii]);
                JHL_Transform (mdContext->buf, in);

  /* store buffer in digest */
      for (i = 0, ii = 0; i < 4; i++, ii += 4) {
        mdContext->digest[ii] = (unsigned char)(mdContext->buf[i] & 0xFF);
        mdContext->digest[ii+1] =
       (unsigned char)((mdContext->buf[i] >> 8) & 0xFF);
       mdContext->digest[ii+2] =
       (unsigned char)((mdContext->buf[i] >> 16) & 0xFF);
        mdContext->digest[ii+3] =
       (unsigned char)((mdContext->buf[i] >> 24) & 0xFF);
	  }
}


