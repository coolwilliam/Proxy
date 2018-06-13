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
	* ��������	pack
	* ��  �ܣ�	���
	* ��  ����
	*			em_type
	*			var_in
	*			buffer_out
	* ����ֵ:	void
	************************************/
	static void pack(int em_type, void *var_in, data_buffer &buffer_out);

	/************************************
	* ��������	unpack
	* ��  �ܣ�	���
	* ��  ����
	*			em_type
	*			var_out
	*			buffer_in
	* ����ֵ:	void
	************************************/
	static void unpack(int em_type, void *var_out, data_buffer &buffer_in);
};

