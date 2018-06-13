#ifndef server_main_data_handler_h__
#define server_main_data_handler_h__
#include "inner_def.h"
#include "msg_define.h"

/*server frame headers*/
#include "data_handler.h"

/*boost headers*/
#include "boost/unordered_map.hpp"

class server_main_data_handler :
	public data_handler
{
public:
	server_main_data_handler();
	~server_main_data_handler();

	virtual bool on_received_data(common_session_ptr session, const string & str_data);


	virtual bool on_session_accept(common_session_ptr new_session);


	virtual int on_session_close(common_session_ptr session, const string & str_remote_ip);


	virtual int on_session_error(common_session_ptr session, int err_code, const string& err_msg);
private:
	typedef boost::unordered_map<common_session_ptr, device_session_buffer_ptr> map_session_buffer_t;

	void add_session_buff(const map_session_buffer_t::value_type& item);
	void get_session_buff(const map_session_buffer_t::key_type key, map_session_buffer_t::mapped_type& value_out) const;
	void del_session_buff(const map_session_buffer_t::key_type key);

	int decode_msg(common_session_ptr session, device_session_buffer_ptr dsb);
	void parse_msg(common_session_ptr session, const msg_head_t& header, const std::string& str_body);
private:
	map_session_buffer_t m_map_session_buff;

};
#endif // server_main_data_handler_h__

