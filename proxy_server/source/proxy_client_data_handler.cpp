#include "proxy_client_data_handler.h"
#include "plugin_log_adapter.h"
#include "msg_define.h"
#include "data_package.h"
#include "encode_function.h"

/*server frame headers*/
#include "data_buffer.h"
#include "common_session.h"
#include "session_connection_manager.h"

proxy_client_data_handler::proxy_client_data_handler()
{
}


proxy_client_data_handler::~proxy_client_data_handler()
{
}

bool proxy_client_data_handler::on_received_data(common_session_ptr session, const string & str_data)
{
	bool is_new_session = false;
	//FIXME:
	// 1、如果是新的连接，则进行解码操作，判断是否接受完整
	proxy_clt_session_buffer_ptr buff;
	get_session_buff(session, buff);
	if (NULL != buff)
	{
		is_new_session = buff->is_new;
	}

	data_buffer_ptr recv_buff = data_buffer_ptr(new data_buffer);
	recv_buff->write_byte_array(str_data.c_str(), str_data.size());

	if (is_new_session)
	{
		
		msg_head_t header;
		memset(&header, 0x00, sizeof(header));
		data_package::unpack(data_package::es_header, &header, *recv_buff);
		
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
			session->close();
			return false;
		}

		_u32 surplus_len = recv_buff->get_data_length() - recv_buff->get_readpos();

		//检查是否接收完整包体
		if (surplus_len < header.data_len)
		{
			LOG_ERROR("Buffer surplus length: " << surplus_len << " < " << "data_len: " << header.data_len);

			return false;
		}

		_u8* body = (_u8 *)(recv_buff->get_data() + recv_buff->get_readpos());

		if (header.data_len > 0)
		{
			//crc32校验
			_u32 new_crc = encode_function::create_crc(body, header.data_len);
			if (new_crc != header.crc_check)
			{
				LOG_ERROR("Buffer crc_check failed, new_crc: " << new_crc << " != " << "header.crc_check: " << header.crc_check);

				return false;
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


		RAPIDJSON_NAMESPACE::Document doc;
		doc.Parse<RAPIDJSON_NAMESPACE::kParseNoFlags>(str_body.c_str());
		if (doc.HasParseError())
		{
			LOG_ERROR("Decode json failed! Json:" << str_body);
			session->close();
			return false;
		}

		static const char* member_cmd = "cmd";
		static const char* member_param = "param";
		static const char* member_ident_code = "ident_code";
		static const char* member_client_proxy_port = "client_proxy_port";
		static const char* member_status = "status";
		static const char* member_message = "message";
		static const char* member_session_id = "session_id";

		vect_json_member_t json_member;
		json_member.push_back(member_cmd);
		json_member.push_back(member_param);
		if (check_json_member(doc, json_member))
		{
			RAPIDJSON_NAMESPACE::Value& cmd_value = doc[member_cmd];
			RAPIDJSON_NAMESPACE::Value& param_value = doc[member_param];
			if (strcmp(cmd_value.GetString(), "proxy_request_ack") == 0)
			{
				json_member.clear();
				json_member.push_back(member_ident_code);
				json_member.push_back(member_status);
				json_member.push_back(member_session_id);
				json_member.push_back(member_message);
				json_member.push_back(member_client_proxy_port);
				if (check_json_member(param_value, json_member))
				{
					// 2、解出代理会话ID，并设置到会话联接中，推送会话缓存，进行交互
					session_connection::session_id_t s_id = param_value[member_session_id].GetInt64();
					session_connection_ptr sc = session_connection_manager::instance().find_session_connection(s_id);
					if (NULL != sc)
					{
						sc->set_session(session);
						sc->send_cache();
					}
					else
					{
						LOG_ERROR("Can't find session connection for session " << session.get() << ", session id " << s_id);
					}
				}
				else
				{
					LOG_ERROR("Invalid param! Json:" << str_body);
				}
			}
			else
			{
				LOG_ERROR("Unknown cmd " << cmd_value.GetString());
			}
		}
		else
		{
			LOG_ERROR("Invalid json:" << str_body);
		}

		buff->is_new = false;
	}
	else
	{
		// 3、如果不是新的连接，则将收到的数据直接推送到会话连接中。
		session_connection_ptr sc = session_connection_manager::instance().find_session_connection(session);
		if (NULL != sc)
		{
			LOG_TRACE("Found session id " << sc->get_session_id() << ", local session " << session.get() << ", the other session " << sc->get_other(session).get());
			sc->send_data(recv_buff, session);
		}
		else
		{
			LOG_ERROR("Can't find session connection for session " << session.get());
		}
	}

	return true;
}

bool proxy_client_data_handler::on_session_accept(common_session_ptr new_session)
{
	LOG_TRACE("New session accepted: " << new_session.get());

	proxy_clt_session_buffer_ptr buff;
	get_session_buff(new_session, buff);
	if (NULL == buff)
	{
		proxy_clt_session_buffer_ptr new_buff = proxy_clt_session_buffer_ptr(new proxy_clt_session_buffer_t);
		new_buff->buffer = data_buffer_ptr(new data_buffer);
		new_buff->is_new = true;
		add_session_buff(make_pair(new_session, new_buff));
	}

	return true;
}

int proxy_client_data_handler::on_session_close(common_session_ptr session, const string & str_remote_ip)
{
	del_session_buff(session);
	session_connection_ptr sc = session_connection_manager::instance().find_session_connection(session);
	if (NULL != sc)
	{
		sc->send_cache();
		common_session_ptr other = sc->get_other(session);
		if (other)
		{
			other->get_socket().get_io_service().post(boost::bind(&proxy_client_data_handler::asyn_close_session, this, other));
		}
		session_connection_manager::instance().delete_session_connection(sc->get_session_id());
	}
	return 0;
}

int proxy_client_data_handler::on_session_error(common_session_ptr session, int err_code, const string& err_msg)
{
	LOG_ERROR("Session: " << session.get() << ", error_code:" << err_code << ", err_msg: " << err_msg.c_str());
	return 0;
}

void proxy_client_data_handler::add_session_buff(const map_session_buffer_t::value_type& item)
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

void proxy_client_data_handler::get_session_buff(const map_session_buffer_t::key_type key, map_session_buffer_t::mapped_type& value_out)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);

	map_session_buffer_t::const_iterator it_find = m_map_session_buff.find(key);
	if (it_find != m_map_session_buff.end())
	{
		value_out = it_find->second;
	}
}

void proxy_client_data_handler::del_session_buff(const map_session_buffer_t::key_type key)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);

	m_map_session_buff.erase(key);
}

void proxy_client_data_handler::asyn_close_session(common_session_ptr session)
{
	if (NULL != session)
	{
		LOG_TRACE("Post close session:" << session.get());

		session->close();
	}
}