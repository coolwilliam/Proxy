#include "device_manager.h"
#include "plugin_log_adapter.h"


void device_manager::add(const device::id_t& id, device_ptr dev)
{
	boost::mutex::scoped_lock lck(m_mtx_device);

	map_device_t::iterator it_find = m_map_device.find(id);
	if (it_find != m_map_device.end())
	{
		LOG_ERROR("Device(" << id << ") is already exist!");
	}
	else
	{
		m_map_device.insert(make_pair(id, dev));
	}
}

device_ptr device_manager::get(const device::id_t& id)
{
	boost::mutex::scoped_lock lck(m_mtx_device);

	device_ptr dev_find;
	map_device_t::const_iterator it_find = m_map_device.find(id);
	if (it_find != m_map_device.end())
	{
		dev_find = it_find->second;
	}

	return dev_find;
}

void device_manager::del(const device::id_t& id, common_session_ptr dev_session /*= common_session_ptr()*/)
{
	boost::mutex::scoped_lock lck(m_mtx_device);
	device_ptr dev_find;
	map_device_t::const_iterator it_find = m_map_device.find(id);
	if (it_find != m_map_device.end())
	{
		if (dev_session != NULL)
		{
			{
				dev_find = it_find->second;
				common_session_ptr session_find = dev_find->get_session();
				// 在删除设备时，如果传入了设备的session，则要比对是否是一样的session，如果不是，则不删除
				if (session_find == dev_session)
				{
					//在删除设备之前，停止所有的server
					dev_find->stop_all_server();
					m_map_device.erase(id);
				}
			}
		}
		else
		{
			//在删除设备之前，停止所有的server
			dev_find->stop_all_server();
			m_map_device.erase(id);
		}
	}
}
