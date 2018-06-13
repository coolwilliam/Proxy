#ifndef plugin_log_msg_h__
#define plugin_log_msg_h__

#include <string>

#define PLUGIN_LOG_MSG "PLUGIN_LOG_MSG"
#define PLUGIN_LOG_NAME	"log"

//日志级别
enum enum_log_level
{
	ELL_TRACE = 0,
	ELL_ERROR,
	ELL_CRITICAL,
	ELL_INFO
};

//日志消息
typedef struct __log_msg
{
	enum_log_level		log_level;
	std::string			log_content;

	__log_msg()
	{
		init();
	}
	
	~__log_msg()
	{
		release();
	}
	
	void init()
	{
		//To do initialization here
		log_level = ELL_TRACE;
		log_content.clear();
	}
	
	void release()
	{
		//To do release space here
		log_level = ELL_TRACE;
		log_content.clear();
	}
	
	__log_msg(const __log_msg& obj_copy)
	{
		init();
		
		*this = obj_copy;
	}
	
	const __log_msg& operator=(const __log_msg& obj_copy)
	{
		if (this == &obj_copy)
		{
			return *this;
		}
		
		release();
		
		//To do copy item from obj_copy to this.

		log_level	= obj_copy.log_level;
		log_content = obj_copy.log_content;
		
		return *this;
	}
}log_msg_t, *log_msg_ptr;

#endif // plugin_log_msg_h__

