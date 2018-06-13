#ifndef server_h__
#define server_h__


/*!
 * \file server.h
 * \date 2018/03/22 14:16
 *
 * \author William Wu
 * Contact: wuhaitao@wayos.cn
 *
 * \brief 
 *
 * TODO: Define class server
 *
 * Class server defines the process of iptv proxy service.
 * \note
*/
/*server frame headers*/
#include "common_singleton.hpp"
#include "tool_ptr_define.h"
#include "network_ptr_define.h"

/*boost headers*/
#include "boost/asio.hpp"
#include "boost/unordered_map.hpp"

class server: public common_singleton<server>
{
public:
	/************************************
	* 函数名:   	start
	* 功  能:	启动服务进程
	* 参  数: 	
	* 返回值:   	bool
	************************************/
	bool start();

	boost::asio::io_service& get_ios() { return m_ios; }

	simple_kv_config_ptr get_config() const { return m_config; }
	void set_config(simple_kv_config_ptr val) { m_config = val; }

	void add_server(common_server_ptr svr);
	void del_server(common_server_ptr svr);

private:
	/************************************
	* 函数名:   	load_lua_config
	* 功  能:	加载lua配置文件
	* 参  数: 	
	*			kv_config
	* 返回值:   	bool
	************************************/
	bool load_lua_config(simple_kv_config_ptr kv_config);

	/************************************
	* 函数名:   	server_init
	* 功  能:	启动网络服务
	* 参  数: 	
	* 返回值:   	void
	************************************/
	void server_init();
private:
	friend class common_singleton<server>;

	typedef boost::unordered_map<common_server_ptr, common_server_ptr> map_server_t;

	// asio service
	boost::asio::io_service m_ios;

	// configure
	simple_kv_config_ptr m_config;

	map_server_t m_map_server;
};

#endif // server_h__
