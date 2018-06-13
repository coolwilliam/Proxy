#pragma once
#include "device.h"

/*server frame headers*/
#include "data_handler.h"

class proxy_server_data_handler :
	public data_handler
{
public:
	proxy_server_data_handler();
	virtual ~proxy_server_data_handler();

	virtual bool on_received_data(common_session_ptr session, const string & str_data);


	virtual bool on_session_accept(common_session_ptr new_session);


	virtual int on_session_close(common_session_ptr session, const string & str_remote_ip);


	virtual int on_session_error(common_session_ptr session, int err_code, const string& err_msg);

	device::id_t get_dev_id() const { return m_dev_id; }
	void set_dev_id(const device::id_t& val) { m_dev_id = val; }

private:
	void asyn_close_session(common_session_ptr session);

private:
	// 对应的设备ID
	device::id_t m_dev_id;

};

