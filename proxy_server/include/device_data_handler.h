#ifndef device_data_handler_h__
#define device_data_handler_h__
#include "inner_def.h"
#include "msg_define.h"

/*server frame headers*/
#include "data_handler.h"

/*system headers*/

/*boost headers*/
#include "boost/unordered_map.hpp"
#include "boost/thread/mutex.hpp"

class device_data_handler :
	public data_handler
{
public:
	device_data_handler();
	virtual ~device_data_handler();

	virtual bool on_received_data(common_session_ptr session, const string & str_data);


	virtual bool on_session_accept(common_session_ptr new_session);


	virtual int on_session_close(common_session_ptr session, const string & str_remote_ip);


	virtual int on_session_error(common_session_ptr session, int err_code, const string& err_msg);

private:
	typedef boost::unordered_map<common_session_ptr, device_session_buffer_ptr> map_session_buffer_t;
	void add_session_buff(const map_session_buffer_t::value_type& item);
	void get_session_buff(const map_session_buffer_t::key_type key, map_session_buffer_t::mapped_type& value_out);
	void del_session_buff(const map_session_buffer_t::key_type key);
	void modify_session_buff(const map_session_buffer_t::key_type key, const device::id_t& new_id);
	int decode_msg(common_session_ptr session, device_session_buffer_ptr dsb);
	void parse_msg(common_session_ptr session, const device::id_t& dev_id, const msg_head_t& header, const std::string& str_body);
	void process_by_lua(enum_lua_cmd_type lua_cmd, const msg_head_t& header, const device::id_t& local_dev_id, const std::string& str_body);
	void asyn_close_session(common_session_ptr session);
private:
	map_session_buffer_t	m_map_session_buff;

	boost::mutex	m_mtx_session_buff;
};

#endif // device_data_handler_h__
