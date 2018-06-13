#pragma once
/*server frame headers*/
#include "common_singleton.hpp"

/*boost headers*/
#include "boost/unordered_map.hpp"
#include "boost/thread/mutex.hpp"

/*system headers*/
#include <map>

class port_manager :
	public common_singleton<port_manager>
{
public:
	enum 
	{
		default_min_port = 20000,
		default_max_port = 0xFFFF
	};
	typedef unsigned short port_t;
	void set_min_max(const port_t& min = default_min_port, const port_t& max = default_max_port);

	void del(const port_t& key);

	port_t new_port();

	bool exist(const port_t& key);

	static bool is_bind(const port_t& port);
private:
	friend class common_singleton<port_manager>;

	port_manager();

	typedef std::map<unsigned short, unsigned short> map_port_t;

	map_port_t m_map_port;

	port_t m_min_port;
	port_t m_max_port;

	boost::mutex m_mtx_port;
};

