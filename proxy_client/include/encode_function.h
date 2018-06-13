#pragma once
#include <string>

class encode_function
{
public:
	enum enum_aes
	{
		AES_ENCODE = 0,                     //����
		AES_DECODE,                         //����
	};

	//crc32У��
	static unsigned int create_crc(unsigned char* buffer, unsigned int size);
	//aes���ܽ���(1����,0����)
	static unsigned int aes_encode_decode(unsigned char* buffer_in, unsigned int in_len, unsigned char* buffer_out, const std::string& aes_key, unsigned int type = AES_ENCODE);
	//����md5
	static std::string create_md5(const std::string& str_buf);
};

