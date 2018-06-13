#include "device.h"
#include "plugin_log_adapter.h"

/*rapidjson headers*/
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

/*server frame headers*/
#include "common_server.h"

device::device()
{
	m_property = simple_kv_config_ptr(new simple_kv_config);
}


device::~device()
{
	m_map_port.clear();
	m_map_server.clear();
}

bool device::set_property(const std::string& json_data)
{
	RAPIDJSON_NAMESPACE::Document doc_ret;
	doc_ret.Parse<RAPIDJSON_NAMESPACE::kParseNoFlags>(json_data.c_str());
	if (doc_ret.HasParseError() || false == doc_ret.IsObject())
	{
		LOG_ERROR("Parse failed, invalid json:" << json_data);
		return false;
	}

	RAPIDJSON_NAMESPACE::Document::ConstMemberIterator it_member = doc_ret.MemberBegin();

	for (; it_member != doc_ret.MemberEnd(); ++it_member)
	{
		set_property(it_member->name.GetString(), it_member->value.GetString());
	}

	return true;
}

void device::set_property(const std::string& key, const std::string& value)
{
	m_property->set(key, value);
}

bool device::get_property(const std::string& key, std::string& value_out)
{
	return m_property->get(key, value_out);
}

void device::set_port_map(const port_t& clt_port, const port_t& svr_port)
{
	map_port_t::iterator it_find = m_map_port.find(clt_port);
	if (it_find != m_map_port.end())
	{
		it_find->second = svr_port;
	}
	else
	{
		m_map_port.insert(make_pair(clt_port, svr_port));
	}
}

void device::del_port_map(const port_t& clt_port)
{
	m_map_port.erase(clt_port);
}

bool device::get_mapped_port(const port_t& clt_port, port_t& svr_port_out)
{
	bool b_find = true;
	map_port_t::iterator it_find = m_map_port.find(clt_port);
	if (it_find != m_map_port.end())
	{
		svr_port_out = it_find->second;
		b_find = true;
	}
	else
	{
		b_find = false;
	}

	return b_find;
}

bool device::get_dest_port(const port_t& svr_port, port_t& clt_port_out)
{
	bool b_find = false;
	map_port_t::iterator it_port = m_map_port.begin();
	for (;it_port != m_map_port.end(); ++it_port)
	{
		if (svr_port == it_port->second)
		{
			clt_port_out = it_port->first;
			b_find = true;
			break;
		}
	}

	return b_find;
}

void device::set_proxy_server(const port_t& dst_port, common_server_ptr proxy_server)
{
	map_proxy_server_t::iterator it_find = m_map_server.find(dst_port);
	if (it_find != m_map_server.end())
	{
		it_find->second = proxy_server;
	}
	else
	{
		m_map_server.insert(make_pair(dst_port, proxy_server));
	}
}

void device::del_proxy_server(const port_t& dst_port)
{
	m_map_server.erase(dst_port);
}

bool device::get_proxy_server(const port_t& dst_port, common_server_ptr& proxy_server_out)
{
	bool b_find = false;
	map_proxy_server_t::iterator it_find = m_map_server.find(dst_port);
	if (it_find != m_map_server.end())
	{
		proxy_server_out = it_find->second;
		b_find = true;
	}
	
	return b_find;
}

void device::stop_all_server()
{
	map_proxy_server_t::iterator it_svr = m_map_server.begin();
	for (; it_svr != m_map_server.end(); ++it_svr)
	{
		it_svr->second->stop();
	}
}
