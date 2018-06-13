#ifndef plugin_log_adapter_h__
#define plugin_log_adapter_h__

#include "boost/thread.hpp"
#include "common_singleton.hpp"
#include <iosfwd>
using namespace std;

class plugin_log_adapter: public common_singleton<plugin_log_adapter>
{
public:
	void log_trace(const std::string& str_log);
	void log_error(const std::string& str_log);
	void log_info(const std::string& str_log);
	void log_critical(const std::string& str_log);
	bool has_log_plugin() const;
private:
	friend class common_singleton<plugin_log_adapter>;
	plugin_log_adapter(){}
	~plugin_log_adapter(){}
private:
	boost::mutex m_mtx_log;
};

#ifndef LOG_TRACE
#define LOG_TRACE(logs) \
	do \
	{\
		if (plugin_log_adapter::instance().has_log_plugin() == false)\
		{\
			break;\
		}\
		ostringstream var;\
		var << "[" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "] - " <<logs;\
		plugin_log_adapter::instance().log_trace(var.str());\
	}while (0)
#endif

#ifndef LOG_ERROR
#define LOG_ERROR(logs) \
	do \
	{\
		if (plugin_log_adapter::instance().has_log_plugin() == false)\
		{\
			break;\
		}\
		ostringstream var;\
		var << "[" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "] - " <<logs;\
		plugin_log_adapter::instance().log_error(var.str());\
	}while (0)
#endif

#ifndef LOG_INFO
#define LOG_INFO(logs) \
	do \
	{\
		if (plugin_log_adapter::instance().has_log_plugin() == false)\
		{\
			break;\
		}\
		ostringstream var;\
		var << "[" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "] - " <<logs;\
		plugin_log_adapter::instance().log_info(var.str());\
	}while (0)
#endif

#ifndef LOG_CRITICAL
#define LOG_CRITICAL(logs) \
	do \
	{\
		if (plugin_log_adapter::instance().has_log_plugin() == false)\
		{\
			break;\
		}\
		ostringstream var;\
		var << "[" << __FILE__ << ":" << __LINE__ << ":" << __FUNCTION__ << "] - " <<logs;\
		plugin_log_adapter::instance().log_critical(var.str());\
	}while (0)
#endif

#endif // plugin_log_adapter_h__
