#ifndef inner_def_h__
#define inner_def_h__
/*!
 * \file inner_def.h
 * \date 2018/03/23 9:57
 *
 * \author William Wu
 * Contact: wuhaitao@wayos.cn
 *
 * \brief Contains definations for this project only.
 *
 * 
 *
 * \note
 * First create by wuhaitao at 2018/03/23 9:57
*/

#include "device.h"

/*server frame headers*/
#include "tool_ptr_define.h"

/*system headers*/
#include <vector>


/*rapid json headers*/
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

enum enum_lua_cmd_type
{
	ELCT_UNKNOWN,
	ELCT_LOGIN,			//注册
	ELCT_HEARTBEAT,		//心跳
	ELCT_GET_CONFIG,	//获取config.lua的内容
	ELCT_MAX
};

#define  LUA_MAIN_FUNC  "lobj_main"

#define config_define(x) const char* x = #x
#define config_declare(x) extern const char* x
//////////////////////////////配置项定义////////////////////////////////////////////
//{{
config_declare(network_thread_count);
config_declare(lua_file_path);
config_declare(device_server_port);
config_declare(web_request_port);
config_declare(proxy_client_port);
config_declare(domain_ip);
config_declare(proxy_recv_cache_size);
config_declare(proxy_for_lo_only);
//}}

enum enum_package_status
{
	PACKAGE_SUCCESS = 0,				//解析成功
	PACKAGE_RECEIVE,					//继续接收
	PACKAGE_ERROR,                  	//数据包出错
};

//设备会话缓存
typedef struct __device_session_buffer
{
	data_buffer_ptr buffer;
	device::id_t	dev_id;
}device_session_buffer_t;

typedef boost::shared_ptr<device_session_buffer_t> device_session_buffer_ptr;

//web request session buffer
typedef struct __web_req_session_buffer 
{
	data_buffer_ptr buffer;
}web_req_session_buffer_t;

typedef boost::shared_ptr<web_req_session_buffer_t> web_req_session_buffer_ptr;

//proxy client session buffer
typedef struct __proxy_clt_session_buffer 
{
	bool is_new;
	data_buffer_ptr buffer;
}proxy_clt_session_buffer_t;

typedef boost::shared_ptr<proxy_clt_session_buffer_t> proxy_clt_session_buffer_ptr;

typedef std::vector<std::string> vect_json_member_t;

bool check_json_member(const RAPIDJSON_NAMESPACE::Value& json_value, const vect_json_member_t& vect_member);

#endif // inner_def_h__
