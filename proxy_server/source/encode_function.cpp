#include "encode_function.h"
#include "crc32.h"
#include "jhl_crypto.h"
#include "MD5.h"

unsigned int encode_function::create_crc(unsigned char* buffer, unsigned int size)
{
	crc32_init();
	unsigned int new_crc = crc32_create(buffer, size);

	return new_crc;
}

unsigned int encode_function::aes_encode_decode(unsigned char* buffer_in, unsigned int in_len, unsigned char* buffer_out, const std::string& aes_key, unsigned int type /*= AES_ENCODE*/)
{
	struct aes_ctx bdc_aes_ctx;
	memset(&bdc_aes_ctx, 0, sizeof(bdc_aes_ctx));
	aes_set_key(&bdc_aes_ctx, (const unsigned char *)aes_key.c_str(), aes_key.size());
	unsigned int decode_len = aes_aes(&bdc_aes_ctx, buffer_in, in_len, buffer_out, type); //1Ω‚√‹,0º”√‹

	return decode_len;
}

std::string encode_function::create_md5(const std::string& str_buf)
{
	MD5_CTX mdContext;

	MD5Init(&mdContext);
	MD5Update(&mdContext, (unsigned char *)str_buf.c_str(), str_buf.size());
	MD5Final(&mdContext);

	char md5_str[33] = { 0 };
	for (size_t i = 0; i < sizeof(mdContext.digest); i++)
	{
		snprintf(md5_str + i * 2, sizeof(md5_str), "%02x", mdContext.digest[i]);
	}

	return std::string(md5_str);
}
