#include "local_proxy_data_handler.h"
#include "plugin_log_adapter.h"

/*server frame headers*/
#include "data_buffer.h"
#include "common_session.h"
#include "session_connection_manager.h"


local_proxy_data_handler::local_proxy_data_handler()
{
}


local_proxy_data_handler::~local_proxy_data_handler()
{
}

bool local_proxy_data_handler::on_received_data(common_session_ptr session, const string & str_data)
{
	boost::mutex::scoped_lock lck(m_mtx_handler);

	session_connection_ptr sc = session_connection_manager::instance().find_session_connection(session);
	if (NULL != sc)
	{
		data_buffer_ptr send_buff = data_buffer_ptr(new data_buffer);
		send_buff->write_byte_array(str_data.data(), str_data.size());
		sc->send_data(send_buff, session);
		return true;
	}
	else
	{
		LOG_ERROR("No session connection for session " << session.get());
		return false;
	}
}

bool local_proxy_data_handler::on_session_accept(common_session_ptr new_session)
{
	throw std::logic_error("The method or operation is not implemented.");
}

int local_proxy_data_handler::on_session_close(common_session_ptr session, const string & str_remote_ip)
{
	LOG_TRACE("Close session successfully! Session = " << session.get() << ", remote ip = " << str_remote_ip);
	session_connection_ptr sc = session_connection_manager::instance().find_session_connection(session);
	if (NULL != sc)
	{
		sc->send_cache();
		common_session_ptr other = sc->get_other(session);
		if (other)
		{
			other->get_socket().get_io_service().post(boost::bind(&local_proxy_data_handler::asyn_close_session, this, other));
		}
		session_connection_manager::instance().delete_session_connection(sc->get_session_id());
	}
	return 0;
}

int local_proxy_data_handler::on_session_error(common_session_ptr session, int err_code, const string& err_msg)
{
	LOG_ERROR("Session: " << session.get() << ", error_code:" << err_code << ", err_msg: " << err_msg.c_str());
	return 0;
}

void local_proxy_data_handler::asyn_close_session(common_session_ptr session)
{
	if (NULL != session)
	{
		LOG_TRACE("Post close session:" << session.get());

		session->close();
	}
}

boost::mutex& local_proxy_data_handler::get_handler_mtx()
{
	return m_mtx_handler;
}
