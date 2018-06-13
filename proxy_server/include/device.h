#ifndef device_h__
#define device_h__
/*!
 * \file device.h
 * \date 2018/03/23 15:34
 *
 * \author William Wu
 * Contact: wuhaitao@wayos.cn
 *
 * \brief Define device class
 *
 * TODO: long description
 *
 * \note
*/

/*system headers*/
#include <string>
#include <map>

/*server frame headers*/
#include "simple_kv_config.h"
#include "tool_ptr_define.h"
#include "network_ptr_define.h"

/*boost headers*/
#include "boost/unordered_map.hpp"

class device
{
public:
	device();
	~device();

	typedef std::string id_t;
	typedef unsigned short port_t;

	device::id_t get_id() const { return m_id; }
	void set_id(const device::id_t& val) { m_id = val; }

	bool set_property(const std::string& json_data);
	void set_property(const std::string& key, const std::string& value);
	bool get_property(const std::string& key, std::string& value_out);

	common_session_ptr get_session() const { return m_session; }
	void set_session(common_session_ptr val) { m_session = val; }

	void set_port_map(const port_t& clt_port, const port_t& svr_port);
	void del_port_map(const port_t& clt_port);
	bool get_mapped_port(const port_t& clt_port, port_t& svr_port_out);
	bool get_dest_port(const port_t& svr_port, port_t& clt_port_out);

	void set_proxy_server(const port_t& dst_port, common_server_ptr proxy_server);
	void del_proxy_server(const port_t& dst_port);
	bool get_proxy_server(const port_t& dst_port, common_server_ptr& proxy_server_out);

	void stop_all_server();
private:
	typedef std::map<port_t, port_t> map_port_t;
	typedef boost::unordered_map<port_t, common_server_ptr> map_proxy_server_t;
	id_t m_id;
	simple_kv_config_ptr	m_property;
	common_session_ptr		m_session;
	map_port_t				m_map_port;		//客户端port与服务端port映射表
	map_proxy_server_t		m_map_server;	//设备代理服务映射表
};
#endif // device_h__

