#ifndef __CRC32_H__
#define __CRC32_H__

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/stat.h>

void crc32_init();
unsigned int crc32_create(unsigned char *buffer, unsigned int size);


#endif
