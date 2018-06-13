#pragma once
#include "inner_def.h"

/*server frame headers*/
#include "data_handler.h"

/*boost headers*/
#include "boost/unordered_map.hpp"
#include "boost/thread/mutex.hpp"

class proxy_client_data_handler :
	public data_handler
{
public:
	proxy_client_data_handler();
	virtual ~proxy_client_data_handler();

	virtual bool on_received_data(common_session_ptr session, const string & str_data);

	virtual bool on_session_accept(common_session_ptr new_session);

	virtual int on_session_close(common_session_ptr session, const string & str_remote_ip);

	virtual int on_session_error(common_session_ptr session, int err_code, const string& err_msg);
private:
	typedef boost::unordered_map<common_session_ptr, proxy_clt_session_buffer_ptr> map_session_buffer_t;

	void add_session_buff(const map_session_buffer_t::value_type& item);
	void get_session_buff(const map_session_buffer_t::key_type key, map_session_buffer_t::mapped_type& value_out);
	void del_session_buff(const map_session_buffer_t::key_type key);

	void asyn_close_session(common_session_ptr session);
private:
	map_session_buffer_t	m_map_session_buff;
	boost::mutex	m_mtx_session_buff;
};

