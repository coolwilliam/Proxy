#pragma once
class data_buffer;
class data_package
{
public:
	enum enum_struct
	{
		es_header = 0
	};

	/************************************
	* 函数名：	pack
	* 功  能：	打包
	* 参  数：
	*			em_type
	*			var_in
	*			buffer_out
	* 返回值:	void
	************************************/
	static void pack(int em_type, void *var_in, data_buffer &buffer_out);

	/************************************
	* 函数名：	unpack
	* 功  能：	解包
	* 参  数：
	*			em_type
	*			var_out
	*			buffer_in
	* 返回值:	void
	************************************/
	static void unpack(int em_type, void *var_out, data_buffer &buffer_in);
};

