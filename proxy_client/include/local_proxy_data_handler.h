#ifndef local_proxy_data_handler_h__
#define local_proxy_data_handler_h__
/*server frame headers*/
#include "data_handler.h"

/*boost headers*/
#include "boost/thread.hpp"

class local_proxy_data_handler :
	public data_handler
{
public:
	local_proxy_data_handler();
	~local_proxy_data_handler();

	virtual bool on_received_data(common_session_ptr session, const string & str_data);


	virtual bool on_session_accept(common_session_ptr new_session);


	virtual int on_session_close(common_session_ptr session, const string & str_remote_ip);


	virtual int on_session_error(common_session_ptr session, int err_code, const string& err_msg);

	boost::mutex& get_handler_mtx();

private:
	void asyn_close_session(common_session_ptr session);

	boost::mutex	m_mtx_handler;
};
#endif // local_proxy_data_handler_h__

