#include "device_manager.h"


void device_manager::add(const device::id_t& id, device_ptr dev)
{
	map_device_t::iterator it_find = m_map_device.find(id);
	if (it_find != m_map_device.end())
	{
		it_find->second = dev;
	}
	else
	{
		m_map_device.insert(make_pair(id, dev));
	}
}

device_ptr device_manager::get(const device::id_t& id) const
{
	device_ptr dev_find;
	map_device_t::const_iterator it_find = m_map_device.find(id);
	if (it_find != m_map_device.end())
	{
		dev_find = it_find->second;
	}

	return dev_find;
}

void device_manager::del(const device::id_t& id)
{
	m_map_device.erase(id);
}
