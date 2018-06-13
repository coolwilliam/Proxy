#include "plugin_log_adapter.h"
#include "plugin_manager.h"
#include "plugin_log_msg.h"

void plugin_log_adapter::log_trace(const std::string& str_log)
{
	boost::mutex::scoped_lock(m_mtx_log);

	log_msg_t log_msg;
	log_msg.log_level = ELL_TRACE;
	log_msg.log_content = str_log;
	plugin_manager_base::vect_plugin_names_t vect_plugins;
	vect_plugins.push_back(PLUGIN_LOG_NAME);

	plugin_manager::instance().send_message(PLUGIN_LOG_MSG, 0, &log_msg, vect_plugins);
}

void plugin_log_adapter::log_error(const std::string& str_log)
{
	boost::mutex::scoped_lock(m_mtx_log);

	log_msg_t log_msg;
	log_msg.log_level = ELL_ERROR;
	log_msg.log_content = str_log;
	plugin_manager_base::vect_plugin_names_t vect_plugins;
	vect_plugins.push_back(PLUGIN_LOG_NAME);

	plugin_manager::instance().send_message(PLUGIN_LOG_MSG, 0, &log_msg, vect_plugins);
}

void plugin_log_adapter::log_info(const std::string& str_log)
{
	boost::mutex::scoped_lock(m_mtx_log);

	log_msg_t log_msg;
	log_msg.log_level = ELL_INFO;
	log_msg.log_content = str_log;
	plugin_manager_base::vect_plugin_names_t vect_plugins;
	vect_plugins.push_back(PLUGIN_LOG_NAME);

	plugin_manager::instance().send_message(PLUGIN_LOG_MSG, 0, &log_msg, vect_plugins);
}

void plugin_log_adapter::log_critical(const std::string& str_log)
{
	boost::mutex::scoped_lock(m_mtx_log);

	log_msg_t log_msg;
	log_msg.log_level = ELL_CRITICAL;
	log_msg.log_content = str_log;
	plugin_manager_base::vect_plugin_names_t vect_plugins;
	vect_plugins.push_back(PLUGIN_LOG_NAME);

	plugin_manager::instance().send_message(PLUGIN_LOG_MSG, 0, &log_msg, vect_plugins);
}

bool plugin_log_adapter::has_log_plugin() const
{
	plugin_ptr p_plugin = plugin_manager::instance().get_plugin(PLUGIN_LOG_NAME);
	if (NULL == p_plugin)
	{
		return false;
	}

	return true;
}


