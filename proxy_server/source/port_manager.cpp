#include "port_manager.h"
#include "plugin_log_adapter.h"

/*system headers*/
#include <vector>
#include <fstream>

/*boost headers*/
#include "boost/iostreams/device/file_descriptor.hpp"
#include "boost/iostreams/stream.hpp"

port_manager::port_manager()
	:m_min_port(default_min_port)
	, m_max_port(default_max_port)
{
}

void port_manager::set_min_max(const port_t& min /*= default_min_port*/, const port_t& max /*= default_max_port*/)
{
	assert(min < max && "Min port is greater than max port");

	m_min_port = min;
	m_max_port = max;
}

void port_manager::del(const port_t& key)
{
	boost::mutex::scoped_lock lck(m_mtx_port);
	m_map_port.erase(key);
}

port_manager::port_t port_manager::new_port()
{
	boost::mutex::scoped_lock lck(m_mtx_port);

	size_t max_count = m_max_port - m_min_port + 1;
	port_t new_port = m_min_port;
	if (!m_map_port.empty())
	{
		if (m_map_port.size() < max_count)
		{
			new_port = m_map_port.rbegin()->first + 1;
			if (new_port > m_max_port)
			{
				new_port = m_min_port;
			}

			while (true)
			{
				if (exist(new_port))
				{
					//如果存在则往后加
					new_port++;
				}
				else if (is_bind(new_port))
				{
					//如果被占用，则不能被分配
					m_map_port.insert(std::make_pair(new_port, new_port));
					new_port++;
				}
				else
				{
					break;
				}

				if (new_port >= m_max_port)
				{
					//无可用端口
					new_port = 0;
					break;
				}
			}

			m_map_port.insert(std::make_pair(new_port, new_port));
		}
		else
		{
			new_port = 0;
		}
	}
	else
	{
		m_map_port.insert(std::make_pair(new_port, new_port));
	}

	return new_port;
	
}

bool port_manager::exist(const port_t& key)
{
	return (m_map_port.find(key) != m_map_port.end());
}

bool port_manager::is_bind(const port_t& port)
{
	FILE* result = NULL;
	ostringstream os_cmd;
	os_cmd << "netstat -nltp | awk '{print $4}' | awk -F\":\" '{print $2}'| grep -w " << port << ";echo $?";
	std::string str_cmd = os_cmd.str();

	result = popen(str_cmd.c_str(), "r");
	if (NULL == result)
	{
		return false;
	}

	std::vector<std::string> lines;
	std::string line;
	std::string content;
	line.resize(128);
	while (!feof(result))
	{
		fgets((char*)(line.c_str()), line.size(), result);
		lines.push_back(line);
	}

	pclose(result);

	//检查命令执行成功与否，最后一行为状态值
	unsigned int excute_ret = atoi((*lines.rbegin()).c_str());

	return excute_ret == 0 ? true : false;
}
