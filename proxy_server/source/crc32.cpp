
#include "crc32.h" 



static unsigned int crc_table[256];
//const static char * program_name = "crc32";

static void init_crc_table(void);
//static unsigned int crc32(unsigned int crc, unsigned char * buffer, unsigned int size);

/*
**初始化crc表,生成32位大小的crc表
**也可以直接定义出crc表,直接查表,
**但总共有256个,看着眼花,用生成的比较方便.
*/
static void init_crc_table(void)
{
	unsigned int c;
	unsigned int i, j;
	
	for (i = 0; i < 256; i++) {
		c = (unsigned int)i;
		for (j = 0; j < 8; j++) {
			if (c & 1)
				c = 0xedb88320L ^ (c >> 1);
			else
				c = c >> 1;
		}
		crc_table[i] = c;
	}
}

/*计算buffer的crc校验码*/
unsigned int crc32_create(unsigned char *buffer, unsigned int size)
{
    unsigned int crc = 0xffffffff;
    unsigned int i;
    for (i = 0; i < size; i++) {
    	crc = crc_table[(crc ^ buffer[i]) & 0xff] ^ (crc >> 8);
    }
    return crc ;
}


void crc32_init()
{
    init_crc_table();
}
