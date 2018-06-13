#include "server.h"
#include "plugin_log_adapter.h"
#include "debug_printer.h"
#include "lua_task.h"
#include "inner_def.h"
#include "device_data_handler.h"
#include "proxy_client_data_handler.h"
#include "proxy_server_data_handler.h"
#include "request_data_handler.h"

/*rapid json headers*/
#include "rapidjson/document.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"

/*system headers*/
#include <string>
#include <iostream>

/*server frame headers*/
#include "plugin_manager.h"
#include "common_function.h"
#include "plugin_base.h"
#include "single_app_instance.h"
#include "simple_kv_config.h"
#include "common_server.h"

/*macros*/
#define CONF_FILE_NAME 		"iptv_server.config"
#define PLUGINS_XML_NAME	"plugins.xml"
#define PLUGIN_DIR_NAME		"plugins"
#define MUTEX_FILE			"/var/run/iptv_server.pid"

bool server::start()
{
	std::string str_full_config = "";

	std::string str_app_dir = common_function::get_module_dir(this);

	//初始化插件管理器
	bool b_mgr_init = plugin_manager::instance().init(&(server::instance().get_ios()));
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

	//启动服务
	server_init();

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

void server::add_server(common_server_ptr svr)
{
	m_map_server.insert(make_pair(svr, svr));
}

void server::del_server(common_server_ptr svr)
{
	m_map_server.erase(svr);
}

bool server::load_lua_config(simple_kv_config_ptr cfg)
{
	std::string str_out;
	std::string str_head_json;
	std::string str_buf;

	std::ostringstream oss_obj;
	oss_obj << ELCT_GET_CONFIG;

	lua_task::task_params_t str_vc;
	str_vc.push_back(oss_obj.str());
	str_vc.push_back(string(""));
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

void server::server_init()
{
	simple_kv_config_ptr config = get_config();

	unsigned short device_svr_port = 0;
	unsigned short web_req_port = 0;
	unsigned short proxy_clt_port = 0;


	config->get(device_server_port, device_svr_port);
	config->get(web_request_port, web_req_port);
	config->get(proxy_client_port, proxy_clt_port);

	boost::asio::ip::tcp::endpoint ep_dev(boost::asio::ip::tcp::v4(), device_svr_port);
	boost::asio::ip::tcp::endpoint ep_wr(boost::asio::ip::tcp::v4(), web_req_port);
	boost::asio::ip::tcp::endpoint ep_pc(boost::asio::ip::tcp::v4(), proxy_clt_port);

	common_server_ptr svr_dev = common_server_ptr(new common_server(m_ios, ep_dev));
	common_server_ptr svr_wr = common_server_ptr(new common_server(m_ios, ep_wr));
	common_server_ptr svr_pc = common_server_ptr(new common_server(m_ios, ep_pc));

	data_handler_ptr dev_handler = data_handler_ptr(new device_data_handler);
	data_handler_ptr wr_handler = data_handler_ptr(new request_data_handler);
	data_handler_ptr pc_handler = data_handler_ptr(new proxy_client_data_handler);

	svr_dev->set_data_handler(dev_handler);
	svr_wr->set_data_handler(wr_handler);
	svr_pc->set_data_handler(pc_handler);

	add_server(svr_dev);
	add_server(svr_wr);
	add_server(svr_pc);

	svr_dev->start();
	svr_wr->start();
	svr_pc->start();

}
