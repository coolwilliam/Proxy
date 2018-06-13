#pragma once

/*server frame headers*/
#include "common_singleton.hpp"
#include "simple_kv_config.h"
#include "tool_ptr_define.h"
#include "network_ptr_define.h"

/*boost headers*/
#include "boost/asio.hpp"
#include "boost/unordered_map.hpp"

class client:public common_singleton<client>
{
public:

	/************************************
	* 函数名:   	start
	* 功  能:	启动客户端
	* 参  数: 	
	* 返回值:   	bool
	************************************/
	bool start();

	boost::asio::io_service& get_ios() { return m_ios; }

	simple_kv_config_ptr get_config() const { return m_config; }
	void set_config(simple_kv_config_ptr val) { m_config = val; }

	void add_client(common_client_ptr clt);
	void del_client(common_client_ptr clt);

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

	bool client_init();

	void heartbeat_timer(unsigned int time_interval, common_client_ptr clt);

private:
	friend class common_singleton<client>;

	typedef boost::unordered_map<common_client_ptr, common_client_ptr> map_client_t;
	typedef boost::shared_ptr<boost::asio::deadline_timer> timer_ptr;
	typedef boost::unordered_map<common_server_ptr, common_server_ptr> map_server_t;

	enum
	{
		default_heartbeat_time=60
	};

	boost::asio::io_service m_ios;

	simple_kv_config_ptr m_config;

	map_client_t m_map_client;

	timer_ptr m_heartbeat_timer;

	map_server_t m_map_server;
};

