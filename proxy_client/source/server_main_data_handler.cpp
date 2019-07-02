#include "server_main_data_handler.h"
#include "plugin_log_adapter.h"
#include "data_package.h"
#include "encode_function.h"
#include "client.h"
#include "local_proxy_data_handler.h"
#include "server_proxy_data_handler.h"
#include "msg_function.h"

/*server frame headers*/
#include "data_buffer.h"
#include "common_session.h"
#include "session_connection_manager.h"
#include "common_client.h"



server_main_data_handler::server_main_data_handler()
{
}


server_main_data_handler::~server_main_data_handler()
{
}

bool server_main_data_handler::on_received_data(common_session_ptr session, const string & str_data)
{
	device_session_buffer_ptr dsb;
	get_session_buff(session, dsb);
	if (NULL != dsb)
	{
		dsb->buffer->write_byte_array(str_data.c_str(), str_data.size());

		//粘包处理
		while (dsb->buffer->get_data_length() != 0)
		{
			//解析数据包
			int enum_type = decode_msg(session, dsb);
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

bool server_main_data_handler::on_session_accept(common_session_ptr new_session)
{
	LOG_TRACE("New session accepted: " << new_session.get());

	device_session_buffer_ptr buff;
	get_session_buff(new_session, buff);
	if (NULL == buff)
	{
		device_session_buffer_ptr new_buff = device_session_buffer_ptr(new device_session_buffer_t);
		new_buff->buffer = data_buffer_ptr(new data_buffer);
		add_session_buff(make_pair(new_session, new_buff));
	}

	return true;
}

int server_main_data_handler::on_session_close(common_session_ptr session, const string & str_remote_ip)
{
	del_session_buff(session);

	LOG_TRACE("Close session successfully! Session = " << session.get() << ", remote ip = " << str_remote_ip);
	return 0;
}

int server_main_data_handler::on_session_error(common_session_ptr session, int err_code, const string& err_msg)
{
	LOG_ERROR("Session: " << session.get() << ", error_code:" << err_code << ", err_msg: " << err_msg.c_str());
	LOG_ERROR("Exit process");
	exit(-1);
}

void server_main_data_handler::add_session_buff(const map_session_buffer_t::value_type& item)
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

void server_main_data_handler::get_session_buff(const map_session_buffer_t::key_type key, map_session_buffer_t::mapped_type& value_out)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);
	
	map_session_buffer_t::const_iterator it_find = m_map_session_buff.find(key);
	if (it_find != m_map_session_buff.end())
	{
		value_out = it_find->second;
	}
}

void server_main_data_handler::del_session_buff(const map_session_buffer_t::key_type key)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);
	
	m_map_session_buff.erase(key);
}

int server_main_data_handler::decode_msg(common_session_ptr session, device_session_buffer_ptr dsb)
{
	//包头
	if (dsb->buffer->get_data_length() < sizeof(msg_head_t))
	{
		LOG_TRACE("Buffer length: " << dsb->buffer->get_data_length() << " < " << "sizeof(msg_head_t): " << sizeof(msg_head_t));
		return PACKAGE_RECEIVE;
	}

	msg_head_t header;
	memset(&header, 0x00, sizeof(header));
	data_package::unpack(data_package::es_header, &header, *(dsb->buffer));

	char str_cmd[32] = { 0 };
	snprintf(str_cmd, sizeof(str_cmd), "%hu", header.cmd);

	LOG_INFO("header:" << std::endl
		<< "\tversion:\t" << (_u32)header.version << std::endl
		<< "\tcmd:\t" << ((header.cmd < CMD_MAX && header.cmd > CMD_UNKOWN) ?
		g_cmd_str[header.cmd].cmd_str : str_cmd) << "(" << header.cmd << ")" << std::endl
		<< "\tmsg_id:\t" << header.msg_id << std::endl
		<< "\taes_flag:\t" << header.aes_flag << std::endl
		<< "\tcrc_check:\t" << header.crc_check << std::endl
		<< "\tdata_len:\t" << header.data_len);

	//包头校验
	if (header.magic != MSG_MAGIC || header.version != MSG_VERSION
		|| header.cmd <= CMD_UNKOWN || header.cmd >= CMD_MAX
		|| header.aes_flag >= AES_ENCODE_MAX || header.aes_flag < AES_ENCODE_NO)
	{
		LOG_ERROR("Buffer header check failed");

		//重置读位置
		dsb->buffer->reset_readpos();
		return PACKAGE_ERROR;
	}

	//剩余长度
	_u32 surplus_len = dsb->buffer->get_data_length() - dsb->buffer->get_readpos();

	//检查是否接收完整包体
	if (surplus_len < header.data_len)
	{
		LOG_TRACE("Buffer surplus length: " << surplus_len << " < " << "data_len: " << header.data_len);

		//重置读位置
		dsb->buffer->reset_readpos();
		return PACKAGE_RECEIVE;
	}

	_u8* body = (_u8 *)(dsb->buffer->get_data() + dsb->buffer->get_readpos());

	if (header.data_len > 0)
	{
		//crc32校验
		_u32 new_crc = encode_function::create_crc(body, header.data_len);
		if (new_crc != header.crc_check)
		{
			LOG_ERROR("Buffer crc_check failed, new_crc: " << new_crc << " != " << "header.crc_check: " << header.crc_check);

			//清空数据
			dsb->buffer->clear_data(0, dsb->buffer->get_readpos() + header.data_len);
			return PACKAGE_ERROR;
		}
	}

	std::string str_body;

	if (AES_ENCODE_YES == header.aes_flag)
	{
		_u8 *decode_buf = new _u8[header.data_len + 32];
		memset(decode_buf, 0, header.data_len + 32);
		_u32 decode_len = encode_function::aes_encode_decode(body, header.data_len, decode_buf, AES_KEY, encode_function::AES_DECODE);

		str_body.assign((char *)decode_buf, decode_len);

		LOG_TRACE("Decode length: " << decode_len);

		if (decode_buf != NULL)
		{
			delete[] decode_buf;
			decode_buf = NULL;
		}
	}
	else
	{
		str_body.assign((char *)body, header.data_len);
	}

	dsb->buffer->clear_data(0, dsb->buffer->get_readpos() + header.data_len);

	parse_msg(session, header, str_body);

	return PACKAGE_SUCCESS;
}

void server_main_data_handler::parse_msg(common_session_ptr session, const msg_head_t& header, const std::string& str_body)
{

	switch (header.cmd)
	{
	case CMD_PROXY_REQUEST:
	{
		simple_kv_config_ptr cfg = client::instance().get_config();
		RAPIDJSON_NAMESPACE::Document doc_request;
		doc_request.Parse<RAPIDJSON_NAMESPACE::kParseNoFlags>(str_body.c_str());
		if (doc_request.HasParseError() || doc_request.IsObject() == false)
		{
			LOG_ERROR("Parse failed, invalid json:" << str_body);
			break;
		}

		static const char* member_cmd = "cmd";
		static const char* member_param = "param";
		static const char* member_session_id = "session_id";
		static const char* member_client_proxy_port = "client_proxy_port";
		static const char* member_server_proxy_ip = "server_proxy_ip";
		static const char* member_server_proxy_port = "server_proxy_port";
		static const char* member_ident_code = "ident_code";
		static const char* member_status = "status";
		static const char* member_message = "message";

		vect_json_member_t vect_member;
		vect_member.push_back(member_cmd);
		vect_member.push_back(member_param);

		if (check_json_member(doc_request, vect_member) == false)
		{
			LOG_ERROR("Invalid json: " << str_body);
			break;
		}

		RAPIDJSON_NAMESPACE::Value& cmd_value = doc_request[member_cmd];
		if (strcmp(cmd_value.GetString(), "proxy_request") == 0)
		{
			RAPIDJSON_NAMESPACE::Value& param_value = doc_request[member_param];

			vect_member.clear();
			vect_member.push_back(member_session_id);
			vect_member.push_back(member_client_proxy_port);
			vect_member.push_back(member_server_proxy_ip);
			vect_member.push_back(member_server_proxy_port);

			if (check_json_member(param_value, vect_member) == false)
			{
				LOG_ERROR("Invalid json: " << str_body);
				break;
			}

			session_connection::session_id_t s_id = param_value[member_session_id].GetUint();
			unsigned short clt_proxy_port = param_value[member_client_proxy_port].GetUint();
			unsigned short svr_proxy_port = param_value[member_server_proxy_port].GetUint();
			const char* svr_proxy_ip = param_value[member_server_proxy_ip].GetString();

			int			err_code;
			std::string err_msg;
			bool		ack_status = true;
			std::string ack_message = "";

			bool b_connect = false;
			session_connection_ptr sc = session_connection_ptr(new session_connection);
			sc->set_close_on_destroy(false);
			//连接本地
			std::string str_local_ip = "127.0.0.1";
			common_client_ptr local_clt = common_client_ptr(new common_client(str_local_ip, clt_proxy_port, client::instance().get_ios()));
			local_proxy_data_handler* lpdh_ptr = new local_proxy_data_handler;
			local_clt->set_data_handler(data_handler_ptr(lpdh_ptr));

			boost::mutex::scoped_lock lck_lpdh(lpdh_ptr->get_handler_mtx());

			b_connect = local_clt->start(err_code, err_msg);

			if (false == b_connect)
			{
				LOG_ERROR("Connect to local [ip:" << str_local_ip << ", port:" << clt_proxy_port << "] failed! Error [code:" << err_code << ", msg:" << err_msg << "]");
				ack_status = false;
				std::ostringstream __msg_oss;
				__msg_oss << "Connect to local [ip:" << str_local_ip << ", port:" << clt_proxy_port << "] failed!";
				ack_message = __msg_oss.str();

				//break;
			}

			// 设置代理接收缓存大小
			unsigned int proxy_cache_size = 0;
			cfg->get(proxy_recv_cache_size, proxy_cache_size);

			if (proxy_cache_size > 0 && b_connect)
			{
				local_clt->get_session()->set_recv_cache_size(proxy_cache_size);
			}
			

			sc->set_session_id(s_id);
			sc->set_session(local_clt->get_session());


			//连接服务端代理
			const std::string& server_proxy_ip = std::string(svr_proxy_ip);
			common_client_ptr svr_proxy_clt = common_client_ptr(new common_client(server_proxy_ip, svr_proxy_port, client::instance().get_ios()));
			server_proxy_data_handler* spdh_ptr = new server_proxy_data_handler;
			svr_proxy_clt->set_data_handler(data_handler_ptr(spdh_ptr));

			boost::mutex::scoped_lock lck_spdh(spdh_ptr->get_handler_mtx());

			b_connect = svr_proxy_clt->start(err_code, err_msg);
			if (false == b_connect)
			{
				LOG_ERROR("Connect to proxy server [ip:" << server_proxy_ip << ", port:" << svr_proxy_port << "] failed! Error [code:" << err_code << ", msg:" << err_msg << "]");
				break;
			}

			if (proxy_cache_size > 0)
			{
				svr_proxy_clt->get_session()->set_recv_cache_size(proxy_cache_size);
			}
			sc->set_session(svr_proxy_clt->get_session());

			session_connection_manager::instance().add_session_connection(sc);


			//发送代理反馈，发送到代理会话中
			RAPIDJSON_NAMESPACE::Document doc_ack;
			RAPIDJSON_NAMESPACE::Document::AllocatorType& ack_alloc = doc_ack.GetAllocator();
			doc_ack.SetObject();

			doc_ack.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_cmd), "proxy_request_ack", ack_alloc);
			
			RAPIDJSON_NAMESPACE::Value ack_param_value(RAPIDJSON_NAMESPACE::kObjectType);


			RAPIDJSON_NAMESPACE::Value session_id_value(RAPIDJSON_NAMESPACE::kNumberType);
			session_id_value.SetInt64(s_id);
			ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_session_id), s_id, ack_alloc);

			std::string str_ident_code;
			cfg->get(device_id, str_ident_code);

			ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_ident_code), RAPIDJSON_NAMESPACE::StringRef(str_ident_code.c_str()), ack_alloc);
			ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_status), ack_status, ack_alloc);
			ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_message), RAPIDJSON_NAMESPACE::StringRef(ack_message.c_str()), ack_alloc);
			ack_param_value.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_client_proxy_port), clt_proxy_port, ack_alloc);

			doc_ack.AddMember(RAPIDJSON_NAMESPACE::StringRef(member_param), ack_param_value, ack_alloc);

			RAPIDJSON_NAMESPACE::StringBuffer buff_ack;
			RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer_ack(buff_ack);
			doc_ack.Accept(writer_ack);
			std::string str_ack;
			str_ack.assign(buff_ack.GetString());

			data_buffer buff_send;
			msg_function::encode(CMD_PROXY_REQUEST_ACK, str_ack, buff_send);
			std::string str_send;
			str_send.assign(buff_send.get_data(), buff_send.get_data_length());

			client::instance().add_client(local_clt);
			client::instance().add_client(svr_proxy_clt);

			svr_proxy_clt->get_session()->send_msg(str_send);
		}

		break;
	}
	case CMD_LOGIN_ACK:
	{
		RAPIDJSON_NAMESPACE::Document doc_request;
		doc_request.Parse<RAPIDJSON_NAMESPACE::kParseNoFlags>(str_body.c_str());
		if (doc_request.HasParseError() || doc_request.IsObject() == false)
		{
			LOG_ERROR("Parse failed, invalid json:" << str_body);
			break;
		}

		static const char* member_stat = "stat";
		static const char* member_ret = "ret";
		static const char* member_proxy_recv_cache_size = "proxy_recv_cache_size";

		vect_json_member_t vect_member;
		vect_member.push_back(member_stat);
		vect_member.push_back(member_ret);
		if (check_json_member(doc_request, vect_member) == false)
		{
			LOG_ERROR("Invalid json: " << str_body);
			break;
		}

		RAPIDJSON_NAMESPACE::Value& ret_val = doc_request[member_ret];

		vect_member.clear();
		vect_member.push_back(member_proxy_recv_cache_size);
		if (check_json_member(ret_val, vect_member) == false)
		{
			break;
		}

		RAPIDJSON_NAMESPACE::Value& pcs = ret_val[member_proxy_recv_cache_size];
		unsigned int proxy_cache_size = pcs.IsString() ? atoi(pcs.GetString()) : pcs.GetUint();
		
		client::instance().get_config()->set(proxy_recv_cache_size, proxy_cache_size);
		LOG_TRACE("Set proxy receive cache size to " << proxy_cache_size);

		break;
	}
	default:
		break;
	}
}