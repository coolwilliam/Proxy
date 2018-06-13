
#ifndef MD5_H__
#define MD5_H__ "jhl_md5.h"

typedef unsigned char __u8;
typedef unsigned short __u16;
typedef unsigned int __u32;


typedef struct {
  __u32 i[2];                  
  __u32 buf[4];               
  unsigned char in[64];               
  unsigned char digest[16];        
} JHL_MD5_CTX;



void   JHL_MD5Init ( JHL_MD5_CTX * mdContext);
void   JHL_MD5Update ( JHL_MD5_CTX * mdContext,
					   unsigned char * inBuf,
					   unsigned int inLen);	

void   JHL_MD5Final( JHL_MD5_CTX *mdContext);

#endif

