#include "device_data_handler.h"
#include "plugin_log_adapter.h"
#include "msg_define.h"
#include "data_package.h"
#include "encode_function.h"
#include "ptr_define.h"
#include "device_manager.h"
#include "lua_task.h"
#include "msg_function.h"

/*server frame headers*/
#include "data_buffer.h"
#include "common_session.h"


device_data_handler::device_data_handler()
{
}


device_data_handler::~device_data_handler()
{
}

bool device_data_handler::on_received_data(common_session_ptr session, const string & str_data)
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
				session->get_socket().get_io_service().post(boost::bind(&device_data_handler::asyn_close_session, this, session));
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

bool device_data_handler::on_session_accept(common_session_ptr new_session)
{
	LOG_TRACE("New session accepted: " << new_session.get());

	device_session_buffer_ptr buff;
	get_session_buff(new_session, buff);
	if (NULL == buff)
	{
		device_session_buffer_ptr new_buff = device_session_buffer_ptr(new device_session_buffer_t);
		new_buff->buffer = data_buffer_ptr(new data_buffer);
		new_buff->dev_id = "";
		add_session_buff(make_pair(new_session, new_buff));
	}

	return true;
}

int device_data_handler::on_session_close(common_session_ptr session, const string & str_remote_ip)
{
	device_session_buffer_ptr buff;
	get_session_buff(session, buff);
	if (NULL != buff)
	{
		device_ptr dev = device_manager::instance().get(buff->dev_id);
		if (NULL != dev)
		{
			device_manager::instance().del(dev->get_id(), session);

			LOG_ERROR("Device [" << buff->dev_id << "] is offline");
		}
		else
		{
			LOG_ERROR("Can't find device " << buff->dev_id);
		}
		del_session_buff(session);
	}

	LOG_TRACE("Close session successfully! Session = " << session.get() << ", remote ip = " << str_remote_ip);
	return 0;
}

int device_data_handler::on_session_error(common_session_ptr session, int err_code, const string& err_msg)
{
	LOG_ERROR("Session: " << session.get() << ", error_code:" << err_code << ", err_msg: " << err_msg.c_str());
	return 0;
}

void device_data_handler::add_session_buff(const map_session_buffer_t::value_type& item)
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

void device_data_handler::get_session_buff(const map_session_buffer_t::key_type key, map_session_buffer_t::mapped_type& value_out)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);

	map_session_buffer_t::const_iterator it_find = m_map_session_buff.find(key);
	if (it_find != m_map_session_buff.end())
	{
		value_out = it_find->second;
	}
}

void device_data_handler::del_session_buff(const map_session_buffer_t::key_type key)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);

	m_map_session_buff.erase(key);
}

void device_data_handler::modify_session_buff(const map_session_buffer_t::key_type key, const device::id_t& new_id)
{
	boost::mutex::scoped_lock lck(m_mtx_session_buff);

	map_session_buffer_t::iterator it_find = m_map_session_buff.find(key);
	if (it_find != m_map_session_buff.end())
	{
		it_find->second->dev_id = new_id;
	}
}

int device_data_handler::decode_msg(common_session_ptr session, device_session_buffer_ptr dsb)
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

	parse_msg(session, dsb->dev_id, header, str_body);

	return PACKAGE_SUCCESS;
}

void device_data_handler::parse_msg(common_session_ptr session, const device::id_t& dev_id, const msg_head_t& header, const std::string& str_body)
{
	device::id_t local_dev_id = dev_id;
	
	enum_lua_cmd_type lua_cmd = ELCT_UNKNOWN;

	switch (header.cmd)
	{
	case CMD_LOGIN:
	{
		bool is_new_device = false;
		device_ptr dev = device_ptr(new device);
		bool b_set_property = dev->set_property(str_body);
		if (false == b_set_property)
		{
			LOG_ERROR("Parse property failed! Data:" << str_body);
			session->get_socket().get_io_service().post(boost::bind(&device_data_handler::asyn_close_session, this, session));
			return;
		}

		static std::string json_ident_code = "ident_code";

		bool b_get = dev->get_property(json_ident_code, local_dev_id);

		device_ptr dev_get = device_manager::instance().get(local_dev_id);
		if (dev_get != NULL)
		{
			dev_get->set_session(session);
			dev_get->set_property(str_body);
		}
		else
		{
			is_new_device = true;
			dev->set_id(local_dev_id);
			dev->set_session(session);
			device_manager::instance().add(local_dev_id, dev);
		}

		modify_session_buff(session, local_dev_id);

		lua_cmd = ELCT_LOGIN;

		break;
	}
	case CMD_HEART_BEAT:
	{
		device_ptr dev = device_manager::instance().get(dev_id);
		if (NULL == dev)
		{
			LOG_ERROR("Command: " << std::string(g_cmd_str[header.cmd].cmd_str) << " from unknown device " << dev_id);
			session->get_socket().get_io_service().post(boost::bind(&device_data_handler::asyn_close_session, this, session));
			return;
		}

		lua_cmd = ELCT_HEARTBEAT;
		break;
	}
	default:
		break;
	}

	process_by_lua(lua_cmd, header, local_dev_id, str_body);
}

void device_data_handler::process_by_lua(enum_lua_cmd_type lua_cmd, const msg_head_t& header, const device::id_t& local_dev_id, const std::string& str_body)
{
	std::ostringstream oss_obj;

	lua_task::task_params_t parms;
	oss_obj << lua_cmd;

	parms.push_back(oss_obj.str());

	oss_obj.str("");
	oss_obj << local_dev_id;

	parms.push_back(oss_obj.str());

	parms.push_back(str_body);

	lua_task tsk;
	tsk.set_params(parms);
	tsk.set_func_name(LUA_MAIN_FUNC);
	tsk.set_return_size(1);

	tsk.start();

	std::string err_msg = tsk.get_error();
	if (err_msg.empty() != true)
	{
		LOG_ERROR(err_msg);
	}
	else
	{
		const lua_task::task_returns_t& returns = tsk.get_returns();
		const std::string& str_ret = returns.at(0);

		data_buffer buffer;

		msg_head_t send_header = header;
		send_header.cmd = send_header.cmd + 1;

		msg_function::encode(send_header.cmd, str_ret, buffer);

		device_ptr dev = device_manager::instance().get(local_dev_id);
		if (dev)
		{
			common_session_ptr session = dev->get_session();
			if (session)
			{
				session->send_msg(std::string(buffer.get_data(), buffer.get_data_length()));
			}
			else
			{
				LOG_ERROR("No session for device " << local_dev_id);
			}
		}
		else
		{
			LOG_ERROR("Can't find device for " << local_dev_id);
		}
	}
}

void device_data_handler::asyn_close_session(common_session_ptr session)
{
	if (NULL != session)
	{
		LOG_TRACE("Post close session:" << session.get());

		session->close();
	}
}
