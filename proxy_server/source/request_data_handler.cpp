#include "request_data_handler.h"
#include "plugin_log_adapter.h"
#include "inner_def.h"
#include "device_manager.h"
#include "port_manager.h"
#include "server.h"
#include "proxy_server_data_handler.h"

/*server frame headers*/
#include "data_buffer.h"
#include "common_session.h"
#include "common_server.h"

/*rapid json*/
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

request_data_handler::request_data_handler()
{
}


request_data_handler::~request_data_handler()
{
}

bool request_data_handler::on_received_data(common_session_ptr session, const string & str_data)
{
	LOG_INFO("Session: " << session.get() << ", recv data: " << str_data.c_str());

	web_req_session_buffer_ptr wrsb;
	get_session_buff(session, wrsb);
	if (NULL != wrsb)
	{
		wrsb->buffer->write_byte_array(str_data.c_str(), str_data.size());

		//粘包处理
		while (wrsb->buffer->get_data_length() != 0)
		{
			//解析数据包
			int enum_type = decode_msg(session, wrsb);
			if (enum_type == PACKAGE_RECEIVE)         //继续接收
			{
				LOG_TRACE("Package continue receive");
				return true;
			}

			if (enum_type == PACKAGE_ERROR)          //数据包出错
			{
				LOG_ERROR("Package error, so close the socket, session: " << session.get());

				//关闭连接
				session->close();
				return false;
			}
		}
	}
	else
	{
		return false;
	}

	return true;
}

int request_data_handler::decode_msg(common_session_ptr session, web_req_session_buffer_ptr css)
{
	std::string str_buf(css->buffer->get_data(), css->buffer->get_data_length());
	string::size_type index = str_buf.find("}\r\n");
	if (index == string::npos)
	{
		LOG_TRACE("PACKAGE_RECEIVE ... ");
		return PACKAGE_RECEIVE;
	}

	//去掉\r\n
	str_buf = str_buf.substr(0, index + 1);

	LOG_INFO("Json: " << std::endl << str_buf);

	//解json
	//FIXME: 解析json

	RAPIDJSON_NAMESPACE::Document doc;
	doc.Parse<RAPIDJSON_NAMESPACE::kParseNoFlags>(str_buf.c_str());
	if (doc.HasParseError())
	{
		LOG_ERROR("Decode json failed!");
		//清空数据
		css->buffer->clear_data(0, index + 3);
		return PACKAGE_ERROR;
	}

	static const char* member_cmd = "cmd";
	static const char* member_param = "param";
	static const char* member_ident_code		= "ident_code";
	static const char* member_client_proxy_port = "client_proxy_port";

	static const char* member_status = "status";
	static const char* member_message = "message";
	static const char* member_server_proxy_ip = "server_proxy_ip";
	static const char* member_server_proxy_port = "server_proxy_port";

	RAPIDJSON_NAMESPACE::Document doc_ack;
	doc_ack.SetObject();
	RAPIDJSON_NAMESPACE::Document::AllocatorType& ack_alloc = doc_ack.GetAllocator();

	vect_json_member_t json_member;
	json_member.push_back(member_cmd);
	json_member.push_back(member_param);

	if (check_json_member(doc, json_member))
	{
		RAPIDJSON_NAMESPACE::Value& cmd_value = doc[member_cmd];
		RAPIDJSON_NAMESPACE::Value& param_value = doc[member_param];
		if (strcmp(cmd_value.GetString(), "web_proxy_request") == 0)
		{
			doc_ack.AddMember(RAPIDJSON_NAMESPACE::StringRef("cmd"), RAPIDJSON_NAMESPACE::StringRef("web_proxy_request_ack"), ack_alloc);
			RAPIDJSON_NAMESPACE::Value ack_param_value(RAPIDJSON_NAMESPACE::kObjectType);

			json_member.clear();
			json_member.push_back(member_ident_code);
			json_member.push_back(member_client_proxy_port);
			if (check_json_member(param_value, json_member))
			{
				RAPIDJSON_NAMESPACE::Value& ident_code_value = param_value[member_ident_code];
				RAPIDJSON_NAMESPACE::Value& client_proxy_port_value = param_value[member_client_proxy_port];

				//获取设备对象
				device_ptr dev = device_manager::instance().get(ident_code_value.GetString());
				if (NULL != dev)
				{
					device::port_t svr_port;
					device::port_t clt_port = atoi(client_proxy_port_value.GetString());
					if (!dev->get_mapped_port(clt_port, svr_port))
					{
						svr_port = port_manager::instance().new_port();
						dev->set_port_map(clt_port, svr_port);

						//FIXME: 
						// 1、启动一个代理服务
						boost::asio::ip::tcp::endpoint ep_proxy(boost::asio::ip::tcp::v4(), svr_port);
						common_server_ptr svr_proxy = common_server_ptr(new common_server(server::instance().get_ios(), ep_proxy));
						proxy_server_data_handler* p_psdh = new proxy_server_data_handler;
						p_psdh->set_dev_id(dev->get_id());
						
						data_handler_ptr svr_proxy_data_handler = data_handler_ptr(p_psdh);
						svr_proxy->set_data_handler(svr_proxy_data_handler);

						// 2、将代理服务加入设备代理服务管理中
						dev->set_proxy_server(clt_port, svr_proxy);

						svr_proxy->start();
					}

					ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_status), true, ack_alloc);
					ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_message), RAPIDJSON_NAMESPACE::StringRef(""), ack_alloc);
					ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_client_proxy_port), client_proxy_port_value, ack_alloc);
					std::ostringstream oss_tmp;
					oss_tmp << svr_port;
					ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_server_proxy_port), RAPIDJSON_NAMESPACE::StringRef(oss_tmp.str().c_str()), ack_alloc);
					ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_server_proxy_ip), RAPIDJSON_NAMESPACE::StringRef(""), ack_alloc);
					ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_ident_code), ident_code_value, ack_alloc);

				}
				else
				{
					LOG_ERROR("Requst proxy failed! Device " << ident_code_value.GetString() << " is offline!");
					ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_status), false, ack_alloc);
					ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_message), RAPIDJSON_NAMESPACE::StringRef("Device is not online."), ack_alloc);
				}

				doc_ack.AddMember("param", ack_param_value, ack_alloc);
			}
			else
			{
				LOG_ERROR("Invalid json:" << str_buf);
				ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_status), false, ack_alloc);
				ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_message), RAPIDJSON_NAMESPACE::StringRef("Invalid json."), ack_alloc);
				doc_ack.AddMember("param", ack_param_value, ack_alloc);
			}
		}
		else
		{
			LOG_ERROR("Unknown cmd:" << std::string(cmd_value.GetString()));
			RAPIDJSON_NAMESPACE::Value ack_param_value(RAPIDJSON_NAMESPACE::kObjectType);
			ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_status), false, ack_alloc);
			ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_message), RAPIDJSON_NAMESPACE::StringRef("Unknown cmd."), ack_alloc);
			doc_ack.AddMember("param", ack_param_value, ack_alloc);
		}
	}
	else
	{
		LOG_ERROR("Invalid json param!Json:" << str_buf);
		RAPIDJSON_NAMESPACE::Value ack_param_value(RAPIDJSON_NAMESPACE::kObjectType);
		ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_status), false, ack_alloc);
		ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_message), RAPIDJSON_NAMESPACE::StringRef("Invalid json param."), ack_alloc);
		doc_ack.AddMember("param", ack_param_value, ack_alloc);
	}

	RAPIDJSON_NAMESPACE::StringBuffer buff_ack;
	RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer_ack(buff_ack);
	doc_ack.Accept(writer_ack);
	std::string str_ack;
	str_ack.assign(buff_ack.GetString());
	session->send_msg(str_ack);
	LOG_INFO("Send to request session:" << session.get() << ", data: " << str_ack);
	session->get_socket().get_io_service().post(boost::bind(&request_data_handler::asyn_close_session, this, session));

	//清空数据
	css->buffer->clear_data(0, index + 3);

	return PACKAGE_SUCCESS;
}

bool request_data_handler::on_session_accept(common_session_ptr new_session)
{
	LOG_TRACE("New session accepted: " << new_session.get());

	web_req_session_buffer_ptr buff;
	get_session_buff(new_session, buff);
	if (NULL == buff)
	{
		web_req_session_buffer_ptr new_buff = web_req_session_buffer_ptr(new web_req_session_buffer_t);
		new_buff->buffer = data_buffer_ptr(new data_buffer);
		add_session_buff(make_pair(new_session, new_buff));
	}

	return true;
}

int request_data_handler::on_session_close(common_session_ptr session, const string & str_remote_ip)
{
	del_session_buff(session);
	return 0;
}

int request_data_handler::on_session_error(common_session_ptr session, int err_code, const string& err_msg)
{
	LOG_ERROR("Session: " << session.get() << ", error_code:" << err_code << ", err_msg: " << err_msg.c_str());
	return 0;
}


void request_data_handler::add_session_buff(const map_session_buffer_t::value_type& item)
{
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

void request_data_handler::get_session_buff(const map_session_buffer_t::key_type key, map_session_buffer_t::mapped_type& value_out) const
{
	map_session_buffer_t::const_iterator it_find = m_map_session_buff.find(key);
	if (it_find != m_map_session_buff.end())
	{
		value_out = it_find->second;
	}
}

void request_data_handler::del_session_buff(const map_session_buffer_t::key_type key)
{
	m_map_session_buff.erase(key);
}

void request_data_handler::asyn_close_session(common_session_ptr session)
{
	if (NULL != session)
	{
		LOG_TRACE("Post close session:" << session.get());

		session->close();
	}
}