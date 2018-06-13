#include "msg_function.h"
#include "encode_function.h"
#include "data_package.h"

void msg_function::encode(_u16 cmd, const std::string& body, data_buffer& data_out, msg_head_ptr src_head /*= NULL*/)
{
	msg_head_t head;
	head.aes_flag = AES_ENCODE_YES;
	head.magic = MSG_MAGIC;
	head.version = MSG_VERSION;
	head.cmd = cmd;

	size_t len = body.size();

	_u8 *encode_buf = NULL;
	_u32 data_len = 0;
	if (head.aes_flag == AES_ENCODE_YES)
	{
		size_t encode_len = len + 32;
		//aesº”√‹
		encode_buf = new _u8[encode_len];
		memset(encode_buf, 0, encode_len);
		data_len = encode_function::aes_encode_decode((_u8 *)body.data(), body.size(), encode_buf, AES_KEY, encode_function::AES_ENCODE);
	}
	else
	{
		data_len = len;
		encode_buf = (_u8*)body.c_str();
	}

	//crc32–£—È
	_u32 new_crc = encode_function::create_crc(encode_buf, data_len);

	head.crc_check = new_crc;
	head.data_len = data_len;

	data_package::pack(data_package::es_header, &head, data_out);
	data_out.write_byte_array((const char *)encode_buf, data_len);

	if (head.aes_flag == AES_ENCODE_YES && encode_buf != NULL)
	{
		delete[] encode_buf;
		encode_buf = NULL;
	}
}
