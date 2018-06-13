#include "data_package.h"
#include "msg_define.h"
#include "data_buffer.h"

////////////////////////////////=pack functions=//////////////////////////////////////////
void pack_header(void* var, data_buffer &buffer)
{
	msg_head_t *var_ptr = (msg_head_t *)var;

	buffer << var_ptr->magic;
	buffer << var_ptr->version;
	buffer << var_ptr->cmd;
	buffer << var_ptr->msg_id;
	buffer << var_ptr->aes_flag;
	buffer << var_ptr->crc_check;
	buffer << var_ptr->data_len;
}

///////////////////////////////=unpack functions=/////////////////////////////////////////
void unpack_header(void *var, data_buffer &buffer)
{
	msg_head_t *var_ptr = (msg_head_t *)var;

	buffer >> var_ptr->magic;
	buffer >> var_ptr->version;
	buffer >> var_ptr->cmd;
	buffer >> var_ptr->msg_id;
	buffer >> var_ptr->aes_flag;
	buffer >> var_ptr->crc_check;
	buffer >> var_ptr->data_len;
}

/////////////////////////////////////////////////////////////////////////////////////////
void data_package::pack(int em_type, void *var_in, data_buffer &buffer_out)
{
	if (NULL == var_in)
	{
		return ;
	}

	switch (em_type)
	{
		case es_header:
		{
			pack_header(var_in, buffer_out);
			break;
		}
		default:
		{
			break;
		}
	}
}

void data_package::unpack(int em_type, void *var_out, data_buffer &buffer_in)
{
	switch (em_type)
	{
		case es_header:
		{
			unpack_header(var_out, buffer_in);
			break;
		}
		default:
		{
			break;
		}
	}
}
