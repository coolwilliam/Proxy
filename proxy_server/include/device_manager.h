#ifndef device_manager_h__
#define device_manager_h__

#include "device.h"
#include "ptr_define.h"

/*boost headers*/
#include "boost/unordered_map.hpp"
#include "boost/thread/mutex.hpp"

/*server frame headers*/
#include "common_singleton.hpp"

class device_manager :
	public common_singleton<device_manager>
{
public:
	void add(const device::id_t& id, device_ptr dev);
	device_ptr get(const device::id_t& id);
	void del(const device::id_t& id, common_session_ptr dev_session = common_session_ptr());
private:
	friend class common_singleton < device_manager > ;
	typedef boost::unordered_map<device::id_t, device_ptr> map_device_t;

	map_device_t m_map_device;
	boost::mutex m_mtx_device;
};

#endif // device_manager_h__
