#ifndef msg_define_h__
#define msg_define_h__
/*!
 * \file msg_define.h
 * \date 2018/03/23 14:03
 *
 * \author Administrator
 * Contact: user@company.com
 *
 * \brief Define the messages for commulication between iptv client and server.
 *
 * It contains message types and format of data gram.
 *
 * \note
*/
/*system headers*/
#include <string.h>

#define MSG_VERSION (0x01)			//�汾��
#define MSG_MAGIC	(0XABCDEEFF)	//��Ϣʶ����

#ifndef AES_KEY
#define AES_KEY "~!h@t*u#a&c#d^b$"
#endif // !AES_KEY

enum enum_aes
{
	AES_ENCODE = 0,                     //����
	AES_DECODE,                         //����
};

enum enum_aes_flag
{
	AES_ENCODE_NO = 0,                  //������
	AES_ENCODE_YES,                     //����
	AES_ENCODE_MAX
};

//��Ϣ����
enum enum_msg_type
{
	CMD_UNKOWN = 0,

	CMD_LOGIN,						//�豸���͵�¼����
	CMD_LOGIN_ACK,					//�ظ�

	CMD_PROXY_REQUEST,				//��������Ͷ˷��ʹ�������
	CMD_PROXY_REQUEST_ACK,			//�ظ�

	CMD_HEART_BEAT,					//�豸����������
	CMD_HEART_BEAT_ACK,				//�ظ�

	CMD_MAX
};

typedef struct __cmd_str
{
	const char* cmd_str;
}cmd_str_t;

//�����ַ�������
extern cmd_str_t g_cmd_str[];

//���Ͷ���
typedef unsigned char		_u8;
typedef unsigned short		_u16;
typedef unsigned int		_u32;
typedef unsigned long long	_u64;
typedef char				_s8;

#pragma pack(1)
//��Ϣͷ
typedef struct __msg_head
{
	_u32    magic;                  //��ͷʶ����
	_u8		version;				//�汾��
	_u16	cmd;					//��Ϣ���
	_u32	msg_id;					//��ϢID(��ack��Ϣ��ԭ������)
	_u32	aes_flag;				//�Ƿ����(0:������ 1:����)
	_u32	crc_check;				//crc32У��ֵ
	_u32	data_len;				//�䳤���ݳ���

	__msg_head()
	{
		init();
	}
	
	~__msg_head()
	{
		release();
	}
	
	void init()
	{
		//To do initialization here
		memset(this, 0x00, sizeof(__msg_head));
	}
	
	void release()
	{
		//To do release space here
		memset(this, 0x00, sizeof(__msg_head));
	}
	
	__msg_head(const __msg_head& obj_copy)
	{
		init();
		
		*this = obj_copy;
	}
	
	const __msg_head& operator=(const __msg_head& obj_copy)
	{
		if (this == &obj_copy)
		{
			return *this;
		}
		
		release();
		
		//To do copy item from obj_copy to this.
		memcpy(this, &obj_copy, sizeof(obj_copy));

		return *this;
	}
}msg_head_t, *msg_head_ptr;
#pragma pack()

#endif // msg_define_h__
