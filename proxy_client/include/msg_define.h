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

#define MSG_VERSION (0x01)			//版本号
#define MSG_MAGIC	(0XABCDEEFF)	//消息识别码

#ifndef AES_KEY
#define AES_KEY "~!h@t*u#a&c#d^b$"
#endif // !AES_KEY

enum enum_aes
{
	AES_ENCODE = 0,                     //加密
	AES_DECODE,                         //解密
};

enum enum_aes_flag
{
	AES_ENCODE_NO = 0,                  //不加密
	AES_ENCODE_YES,                     //加密
	AES_ENCODE_MAX
};

//消息类型
enum enum_msg_type
{
	CMD_UNKOWN = 0,

	CMD_LOGIN,						//设备发送登录请求
	CMD_LOGIN_ACK,					//回复

	CMD_PROXY_REQUEST,				//服务器向客端发送代理请求
	CMD_PROXY_REQUEST_ACK,			//回复

	CMD_HEART_BEAT,					//设备发送心跳包
	CMD_HEART_BEAT_ACK,				//回复

	CMD_MAX
};

typedef struct __cmd_str
{
	const char* cmd_str;
}cmd_str_t;

//命令字符串集合
extern cmd_str_t g_cmd_str[];

//类型定义
typedef unsigned char		_u8;
typedef unsigned short		_u16;
typedef unsigned int		_u32;
typedef unsigned long long	_u64;
typedef char				_s8;

#pragma pack(1)
//消息头
typedef struct __msg_head
{
	_u32    magic;                  //包头识别码
	_u8		version;				//版本号
	_u16	cmd;					//消息类别
	_u32	msg_id;					//消息ID(在ack消息中原样返回)
	_u32	aes_flag;				//是否加密(0:不加密 1:加密)
	_u32	crc_check;				//crc32校验值
	_u32	data_len;				//变长数据长度

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
