///////////////////////////////////////////////////////////
//  lua_operator.cpp
//  Implementation of the Class lua_operator
//  Created on:      16-八月-2016 10:18:27
//  Original author: Administrator
///////////////////////////////////////////////////////////

#include "lua_operator.h"
#include <assert.h>
#include "plugin_log_adapter.h"
#include "plugin_log_msg.h"
#include "lua.hpp"
/*
extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
};
*/

#define LUAG_OPERATOR_OBJ_NAME "lua_operator"

lua_operator::lua_operator()
	:m_initialized(false)
	, m_lua_state(NULL)
{
}



lua_operator::~lua_operator(){
}


int write_log(lua_State* L)
{
	int i = lua_gettop(L);
	int log_level = lua_tonumber(L, i - 1);
	const char* log_content = lua_tostring(L, i);

	if (NULL == log_content)
	{
		LOG_ERROR("Lua log content is null");
		return 1;
	}

	switch (log_level)
	{
	case ELL_TRACE:
	{
		LOG_TRACE(log_content);
		break;
	}
	case ELL_INFO:
	{
		LOG_INFO(log_content);
		break;
	}
	case ELL_ERROR:
	{
		LOG_ERROR(log_content);
		break;
	}
	case ELL_CRITICAL:
	{
		LOG_CRITICAL(log_content);
		break;
	}
	default:
	{
		LOG_ERROR("Invalid log level from lua, level(" << log_level << ")");
		break;
	}
	}

	return 0;
}

int luaErrorHandler(lua_State *L)
{

	int pos_topold = lua_gettop(L);

	std::string error_msg = "";

	lua_getglobal(L, LUAG_OPERATOR_OBJ_NAME);
	lua_operator* p_this = (lua_operator*)(lua_touserdata(L, -1));
	if (NULL == p_this)
	{
		LOG_ERROR("Get lua operator pointer failed");
		lua_pop(L, 1);
		lua_settop(L, pos_topold);
		return 1;
	}

	//获取出错时的堆栈 调用 debug.traceback
	lua_getglobal(L, "debug");
	if (!lua_istable(L, -1))
	{
		LOG_ERROR("Get lua debug info failed");
		lua_pop(L, 1);
		lua_settop(L, pos_topold);
		return 1;
	}

	lua_getfield(L, -1, "traceback");
	if (!lua_isfunction(L, -1))
	{
		LOG_ERROR("Get function debug.traceback failed");
		lua_pop(L, 2);
		lua_settop(L, pos_topold);
		return 1;
	}

	lua_pushvalue(L, 1);
	lua_pushinteger(L, 2);

	lua_pcall(L, 2, 1, 0);

	const char* str_traceback = lua_tostring(L, -1);

	std::ostringstream oss_backtrace;

	oss_backtrace << error_msg << std::endl << str_traceback;

	error_msg = oss_backtrace.str();

	p_this->set_error(error_msg);

	lua_settop(L, pos_topold);

	return 0;
}


bool lua_operator::call(const string& func_name, const vector<string>& params, vector<string>& returns, string& error_msg){
	assert(is_initialized() && "lua_operator has not been initialized !");

	bool result = false;

	string ret = "";
	
	//将lua_operator类对象设置进去，进行传递
	lua_pushlightuserdata(m_lua_state, this);
	lua_setglobal(m_lua_state, LUAG_OPERATOR_OBJ_NAME);
	
	lua_pushcfunction(m_lua_state, luaErrorHandler);
	
	int pos_topold = lua_gettop(m_lua_state);
	//设置错误处理函数
	int error_index = pos_topold;

	lua_getglobal(m_lua_state, func_name.c_str());

	vector<string>::const_iterator itr;
	for (itr = params.begin(); itr != params.end(); ++itr)
	{
		lua_pushstring(m_lua_state, (*itr).c_str());
	}
	
	vector<string>::size_type p_size = 0;
	vector<string>::size_type r_size = 0;
	p_size = params.size();
	r_size = returns.size();
	
	LOG_TRACE("Params size = " << p_size << ", return size = " << r_size);

	int rst = lua_pcall(m_lua_state, p_size, r_size, error_index);
	if (rst)
	{
		result = false;
	}
	else
	{
		result = true;

		for (vector<string>::size_type i = 0; i < r_size; ++i)
		{
			if (lua_isstring(m_lua_state, -(r_size - i)) 
				|| lua_isnumber(m_lua_state, -(r_size - i)))
			{
				returns[i] = lua_tostring(m_lua_state, -(r_size - i));
			}
			else
			{
				returns[i] = string("");
			}
		}

		lua_pop(m_lua_state, returns.size());
	}

	lua_settop(m_lua_state, pos_topold);

	if (m_lua_state != NULL)
	{
		lua_close(m_lua_state);
		m_lua_state = NULL;
	}

	return result;
}


int lua_operator::init(const string & str_lua_file){

	if (is_initialized() == false)
	{
		m_lua_state = lua_open();
		if (NULL != m_lua_state)
		{
			luaL_openlibs(m_lua_state);

			//注册写日志的回调函数
			lua_register(m_lua_state, "write_log", write_log);

			luaL_loadfile(m_lua_state, str_lua_file.c_str());
			lua_pcall(m_lua_state, 0, LUA_MULTRET, 0);

			set_initialized(true);
		}
	}

	return 0;
}


bool lua_operator::is_initialized(){

	return m_initialized;
}

void lua_operator::set_initialized(bool val /*= true*/)
{
	m_initialized = val;
}
