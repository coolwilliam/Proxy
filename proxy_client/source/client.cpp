#include "client.h"
#include "plugin_log_adapter.h"
#include "debug_printer.h"
#include "lua_task.h"
#include "inner_def.h"
#include "server_main_data_handler.h"
#include "msg_function.h"
#include "msg_define.h"
#include "request_data_handler.h"

/*server frame headers*/
#include "plugin_manager.h"
#include "common_function.h"
#include "plugin_base.h"
#include "single_app_instance.h"
#include "simple_kv_config.h"
#include "common_client.h"
#include "common_session.h"
#include "common_server.h"

/*macros*/
#define CONF_FILE_NAME 		"iptv_client.config"
#define PLUGINS_XML_NAME	"plugins.xml"
#define PLUGIN_DIR_NAME		"plugins"
#define MUTEX_FILE			"/var/run/iptv_client.pid"

bool client::start()
{
	std::string str_full_config = "";

	std::string str_app_dir = common_function::get_module_dir(this);

	//初始化插件管理器
	bool b_mgr_init = plugin_manager::instance().init(&(client::instance().get_ios()));
	if (false == b_mgr_init)
	{
		std::cout << "Init plugin manager failed!" << std::endl;
		return false;
	}

	//设置主程序所在路径
	plugin_manager::instance().set_app_path(str_app_dir);

	if ('/' != *str_app_dir.rbegin())
	{
		str_app_dir.push_back('/');
	}
	//加载插件
	bool b_load_plugin = plugin_manager::instance().load_plugins(str_app_dir + std::string(PLUGINS_XML_NAME), str_app_dir + std::string(PLUGIN_DIR_NAME));
	if (false == b_load_plugin)
	{
		std::cout << "Load plugins failed!" << std::endl;
		return false;
	}

	//输出已经加载的插件
	const plugin_manager_base::map_plugins_t& plugins = plugin_manager::instance().get_all_plugins();
	std::cout << "Plugins size:" << plugins.size() << std::endl;
	std::cout << "Plugins:" << std::endl << "{" << std::endl;
	plugin_manager_base::map_plugins_t::const_iterator it_plugin = plugins.begin();
	for (; it_plugin != plugins.end(); ++it_plugin)
	{
		std::cout << "\tPlugin name:" << it_plugin->second->get_plugin_name().c_str() << std::endl;
	}
	std::cout << "}" << std::endl;

	//同时只能存在一个程序实例
	single_app_instance sai(MUTEX_FILE);
	if (sai.is_running())
	{
		std::string str_err_msg = "An instance is already running!";
		std::cerr << str_err_msg << std::endl;
		LOG_ERROR(str_err_msg);
		return false;
	}

	//读取配置文件
	simple_kv_config_ptr config = simple_kv_config_ptr(new simple_kv_config);
	str_full_config = str_app_dir + CONF_FILE_NAME;
	if (false == config->load_file(str_full_config))
	{
		std::string str_err = "Load file " + str_full_config + " failed!";
		DEBUG_ERROR(str_err);
		LOG_ERROR(str_err);
		return false;
	}

	set_config(config);

	//读取lua配置
	bool b_get_config = load_lua_config(config);
	if (false == b_get_config)
	{
		LOG_ERROR("Get lua config values failed!");
		return false;
	}

	//获取网络线程个数
	unsigned int ui_thread_count = 0;
	config->get(network_thread_count, ui_thread_count);
	if (0 == ui_thread_count)
	{
		LOG_ERROR("Thread pool size is 0!");
	}

	//连接服务端
	client_init();

	try
	{
		if (0 == ui_thread_count)
		{
			m_ios.run();
		}
		else
		{
			//并发网络线程
			typedef boost::shared_ptr<boost::thread> thread_ptr;
			typedef std::vector<thread_ptr> vect_thread_pool_t;
			vect_thread_pool_t tp;
			for (unsigned int i = 0; i < ui_thread_count; ++i)
			{
				thread_ptr new_thread = thread_ptr(new boost::thread(boost::bind(&boost::asio::io_service::run, &m_ios)));
				tp.push_back(new_thread);
			}

			for (unsigned int i = 0; i < ui_thread_count; ++i)
			{
				tp.at(i)->join();
			}
		}
	}
	catch (std::exception& e) {
		LOG_ERROR("Exception in io_service.run() :" << std::string(e.what()).c_str());
		std::cout << "Exception in io_service.run() :" << std::string(e.what()).c_str() << std::endl;
		return false;
	}

	return true;
}

void client::add_client(common_client_ptr clt)
{
	m_map_client.insert(make_pair(clt, clt));
}

void client::del_client(common_client_ptr clt)
{
	m_map_client.erase(clt);
}

bool client::load_lua_config(simple_kv_config_ptr cfg)
{
	std::string str_out;
	std::string str_head_json;
	std::string str_buf;

	std::ostringstream oss_obj;
	oss_obj << ELCT_GET_CONFIG;

	lua_task::task_params_t str_vc;
//	str_vc.push_back(string(""));
	str_vc.push_back(oss_obj.str());
	str_vc.push_back(string(""));

	lua_task p_task;
	p_task.set_params(str_vc);
	p_task.set_func_name(string(LUA_MAIN_FUNC));
	p_task.set_return_size(1);

	p_task.start();

	if (false == p_task.get_error().empty())
	{
		LOG_ERROR("Operate ssdb failed, error: " << p_task.get_error());
		return false;
	}

	const lua_task::task_returns_t& returns = p_task.get_returns();
	const std::string& str_ret = returns.at(0);

	//解析json
	RAPIDJSON_NAMESPACE::Document doc;
	doc.Parse<RAPIDJSON_NAMESPACE::kParseNoFlags>(str_ret.c_str());
	if (doc.HasParseError())
	{
		LOG_ERROR("Parse failed! Source json:" << str_ret);
		return false;
	}

	RAPIDJSON_NAMESPACE::Document::MemberIterator it = doc.MemberBegin();
	for (; it != doc.MemberEnd(); ++it)
	{
		std::string name = it->name.GetString();
		if (it->value.IsUint() || it->value.IsInt())
		{
			unsigned int value = it->value.GetUint();
			cfg->set(name, value);
		}
		else if (it->value.IsString())
		{
			std::string value = it->value.GetString();

			cfg->set(name, value);
		}
		else if (it->value.IsBool())
		{
			bool value = it->value.GetBool();
			cfg->set(name, value);
		}
	}

	return true;
}

bool client::client_init()
{
	std::string svr_ip;
	unsigned short svr_port = 0;

	simple_kv_config_ptr config = get_config();
	config->get(device_server_port, svr_port);
	config->get(domain_ip, svr_ip);

	common_client_ptr clt = common_client_ptr(new common_client(svr_ip, svr_port, get_ios()));
	int			err_code;
	std::string err_msg;
	/*
		FIXME:
		设置数据处理器
	*/

	clt->set_data_handler(data_handler_ptr(new server_main_data_handler));

	bool b_connect = clt->start(err_code, err_msg);
	if (false == b_connect)
	{
		LOG_ERROR("Connect to server [ip:" << svr_ip << ", port:" << svr_port << "] failed!");
		return false;
	}

	data_handler_ptr dh = clt->get_data_handler();
	dh->on_session_accept(clt->get_session());

	add_client(clt);

	/*
		启动内部请求服务
	*/
	unsigned short req_svr_port = 0;
	config->get(device_request_port, req_svr_port);

	boost::asio::ip::tcp::endpoint ep_req(boost::asio::ip::tcp::v4(), req_svr_port);

	common_server_ptr req_svr = common_server_ptr(new common_server(get_ios(), ep_req));
	data_handler_ptr req_handler = data_handler_ptr(new request_data_handler);

	req_svr->set_data_handler(req_handler);

	add_server(req_svr);

	req_svr->start();

	//发送登陆命令
	RAPIDJSON_NAMESPACE::Document doc_login;
	doc_login.SetObject();
	RAPIDJSON_NAMESPACE::Document::AllocatorType& doc_alloc = doc_login.GetAllocator();
	std::string str_ident_code;
	bool b_get = config->get(device_id, str_ident_code);
	if (false == b_get)
	{
		LOG_ERROR("Get device id failed!");
		return false;
	}
	doc_login.AddMember(RAPIDJSON_NAMESPACE::StringRef("ident_code"), RAPIDJSON_NAMESPACE::StringRef(str_ident_code.c_str()), doc_alloc);

	RAPIDJSON_NAMESPACE::StringBuffer buff;
	RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buff);
	doc_login.Accept(writer);

	std::string str_body;
	str_body.assign(buff.GetString());

	data_buffer send_buff;
	msg_function::encode(CMD_LOGIN, str_body, send_buff);

	std::string str_send;
	str_send.assign(send_buff.get_data(), send_buff.get_data_length());

	clt->get_session()->send_msg(str_send);

	//启动心跳流程
	unsigned int time_span;
	bool bget = config->get(heartbeat_time, time_span);
	if (false == bget)
	{
		time_span = default_heartbeat_time;
		config->set(heartbeat_time, time_span);
	}

	m_heartbeat_timer = timer_ptr(new timer_ptr::element_type(get_ios()));
	m_heartbeat_timer->expires_from_now(boost::posix_time::seconds(time_span));
	m_heartbeat_timer->async_wait(boost::bind(&client::heartbeat_timer, this, time_span, clt));

	return true;
}

void client::heartbeat_timer(unsigned int time_interval, common_client_ptr clt)
{
	simple_kv_config_ptr config = get_config();

	//发送心跳命令
	RAPIDJSON_NAMESPACE::Document doc_login;
	doc_login.SetObject();
	RAPIDJSON_NAMESPACE::Document::AllocatorType& doc_alloc = doc_login.GetAllocator();
	std::string str_ident_code;
	bool b_get = config->get(device_id, str_ident_code);
	if (false == b_get)
	{
		LOG_ERROR("Get device id failed!");
		return;
	}

	doc_login.AddMember(RAPIDJSON_NAMESPACE::StringRef("ident_code"), RAPIDJSON_NAMESPACE::StringRef(str_ident_code.c_str()), doc_alloc);

	RAPIDJSON_NAMESPACE::StringBuffer buff;
	RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buff);
	doc_login.Accept(writer);

	std::string str_body;
	str_body.assign(buff.GetString());

	data_buffer send_buff;
	msg_function::encode(CMD_HEART_BEAT, str_body, send_buff);

	std::string str_send;
	str_send.assign(send_buff.get_data(), send_buff.get_data_length());

	clt->get_session()->send_msg(str_send);

	m_heartbeat_timer->expires_from_now(boost::posix_time::seconds(time_interval));
	m_heartbeat_timer->async_wait(boost::bind(&client::heartbeat_timer, this, time_interval, clt));
}

void client::add_server(common_server_ptr svr)
{
	m_map_server.insert(make_pair(svr, svr));
}

void client::del_server(common_server_ptr svr)
{
	m_map_server.erase(svr);
}