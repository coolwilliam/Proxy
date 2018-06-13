#include "server_proxy_data_handler.h"
#include "plugin_log_adapter.h"

/*server frame headers*/
#include "data_buffer.h"
#include "common_session.h"
#include "session_connection_manager.h"
#include "common_client.h"

server_proxy_data_handler::server_proxy_data_handler()
{
}


server_proxy_data_handler::~server_proxy_data_handler()
{
}

bool server_proxy_data_handler::on_received_data(common_session_ptr session, const string & str_data)
{
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

bool server_proxy_data_handler::on_session_accept(common_session_ptr new_session)
{
	//This function won't be called.
	return true;
}

int server_proxy_data_handler::on_session_close(common_session_ptr session, const string & str_remote_ip)
{
	session_connection_ptr sc = session_connection_manager::instance().find_session_connection(session);
	if (NULL != sc)
	{
		sc->send_cache();
		common_session_ptr other = sc->get_other(session);
		if (other)
		{
			other->get_socket().get_io_service().post(boost::bind(&server_proxy_data_handler::asyn_close_session, this, other));
		}
		session_connection_manager::instance().delete_session_connection(sc->get_session_id());
	}

	//del_session_buff(session);

	LOG_TRACE("Close session successfully! Session = " << session.get() << ", remote ip = " << str_remote_ip);
	return 0;
}

int server_proxy_data_handler::on_session_error(common_session_ptr session, int err_code, const string& err_msg)
{
	LOG_ERROR("Session: " << session.get() << ", error_code:" << err_code << ", err_msg: " << err_msg.c_str());
	return 0;
}

void server_proxy_data_handler::add_session_buff(const map_session_buffer_t::value_type& item)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);

	map_session_buffer_t::iterator it_find = m_map_session_buff.find(item.first);
	if (it_find != m_map_session_buff.end())
	{
		it_find->second = item.second;
	}
	else
	{
		m_map_session_buff.insert(item);
	}
}

void server_proxy_data_handler::get_session_buff(const map_session_buffer_t::key_type key, map_session_buffer_t::mapped_type& value_out)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);

	map_session_buffer_t::const_iterator it_find = m_map_session_buff.find(key);
	if (it_find != m_map_session_buff.end())
	{
		value_out = it_find->second;
	}
}

void server_proxy_data_handler::del_session_buff(const map_session_buffer_t::key_type key)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);

	m_map_session_buff.erase(key);
}

void server_proxy_data_handler::asyn_close_session(common_session_ptr session)
{
	if (NULL != session)
	{
		LOG_TRACE("Post close session:" << session.get());

		session->close();
	}
}

boost::mutex& server_proxy_data_handler::get_handler_mtx()
{
	return m_mtx_handler;
}
