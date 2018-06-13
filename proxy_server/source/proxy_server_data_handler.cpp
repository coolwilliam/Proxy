#include "proxy_server_data_handler.h"
#include "device_manager.h"
#include "session_id_creator.h"
#include "inner_def.h"
#include "server.h"
#include "plugin_log_adapter.h"

/*server frame headers*/
#include "common_session.h"
#include "session_connection_manager.h"
#include "data_buffer.h"
#include "msg_function.h"

/*rapidjson headers*/
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

proxy_server_data_handler::proxy_server_data_handler()
{
}


proxy_server_data_handler::~proxy_server_data_handler()
{
}

bool proxy_server_data_handler::on_received_data(common_session_ptr session, const string & str_data)
{
	session_connection_ptr s_con = session_connection_manager::instance().find_session_connection(session);
	if (NULL != s_con)
	{
		data_buffer_ptr new_buff = data_buffer_ptr(new data_buffer);
		new_buff->write_byte_array(str_data.c_str(), str_data.size());
		s_con->send_data(new_buff, session);
	}
	return true;
}

bool proxy_server_data_handler::on_session_accept(common_session_ptr new_session)
{
	//获取本地端口
	boost::system::error_code err;
	device::port_t svr_proxy_port = new_session->get_socket().local_endpoint(err).port();

	device_ptr dev = device_manager::instance().get(get_dev_id());
	if (NULL != dev)
	{
		//根据本地监听映射端口，获取目的端口
		device::port_t dest_port = 0;
		if (dev->get_dest_port(svr_proxy_port, dest_port))
		{
			session_connection::session_id_t s_id = session_id_creator::create_session_id();
			session_connection_ptr s_con = session_connection_ptr(new session_connection(new_session, common_session_ptr((common_session*)NULL), s_id));
			s_con->set_close_on_destroy(false);
			session_connection_manager::instance().add_session_connection(s_con);

			//生成请求报文
			RAPIDJSON_NAMESPACE::Document doc_request;
			doc_request.SetObject();
			RAPIDJSON_NAMESPACE::Document::AllocatorType& req_alloc = doc_request.GetAllocator();
			doc_request.AddMember("cmd", "proxy_request", req_alloc);

			RAPIDJSON_NAMESPACE::Value param_value(RAPIDJSON_NAMESPACE::kObjectType);
			param_value.AddMember("session_id", s_id, req_alloc);
			param_value.AddMember("client_proxy_port", dest_port, req_alloc);
			std::string str_host;
			simple_kv_config_ptr cfg = server::instance().get_config();
			cfg->get(domain_ip, str_host);
			param_value.AddMember("server_proxy_ip", RAPIDJSON_NAMESPACE::StringRef(str_host.c_str(), str_host.size()), req_alloc);
			device::port_t svr_prx_port = 0;
			cfg->get(proxy_client_port, svr_prx_port);
			param_value.AddMember("server_proxy_port", svr_prx_port, req_alloc);

			doc_request.AddMember("param", param_value, req_alloc);
			
			RAPIDJSON_NAMESPACE::StringBuffer buff;
			RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buff);
			doc_request.Accept(writer);

			std::string str_body;
			str_body.assign(buff.GetString());

			data_buffer send_buff;
			msg_function::encode(CMD_PROXY_REQUEST, str_body, send_buff);

			std::string str_send;
			str_send.assign(send_buff.get_data(), send_buff.get_data_length());
			
			dev->get_session()->send_msg(str_send);
		}
		else
		{
			LOG_ERROR("No dest port for server proxy port[" << svr_proxy_port << "] on device [" << get_dev_id() << "]!");
			new_session->close();
		}
	}
	else
	{
		LOG_ERROR("Device " << get_dev_id() << " doesn't exist!");
	}

	return true;
}

int proxy_server_data_handler::on_session_close(common_session_ptr session, const string & str_remote_ip)
{
	LOG_TRACE("Close session successfully! Session = " << session.get() << ", remote ip = " << str_remote_ip);

	session_connection_ptr sc = session_connection_manager::instance().find_session_connection(session);
	if (NULL != sc)
	{
		sc->send_cache();
		common_session_ptr other = sc->get_other(session);
		if (other)
		{
			other->get_socket().get_io_service().post(boost::bind(&proxy_server_data_handler::asyn_close_session, this, other));
		}
		session_connection_manager::instance().delete_session_connection(sc->get_session_id());
	}

	return 0;
}

int proxy_server_data_handler::on_session_error(common_session_ptr session, int err_code, const string& err_msg)
{
	LOG_ERROR("Session: " << session.get() << ", error_code:" << err_code << ", err_msg: " << err_msg.c_str());
	return 0;
}


void proxy_server_data_handler::asyn_close_session(common_session_ptr session)
{
	if (NULL != session)
	{
		LOG_TRACE("Post close session:" << session.get());

		session->close();
	}
}