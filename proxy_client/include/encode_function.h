#pragma once
#include <string>

class encode_function
{
public:
	enum enum_aes
	{
		AES_ENCODE = 0,                     //加密
		AES_DECODE,                         //解密
	};

	//crc32校验
	static unsigned int create_crc(unsigned char* buffer, unsigned int size);
	//aes加密解密(1解密,0加密)
	static unsigned int aes_encode_decode(unsigned char* buffer_in, unsigned int in_len, unsigned char* buffer_out, const std::string& aes_key, unsigned int type = AES_ENCODE);
	//产生md5
	static std::string create_md5(const std::string& str_buf);
};

