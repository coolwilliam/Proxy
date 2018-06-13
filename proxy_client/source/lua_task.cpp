///////////////////////////////////////////////////////////
//  lua_task.cpp
//  Implementation of the Class lua_task
//  Created on:      16-°ËÔÂ-2016 10:18:27
//  Original author: wuht
///////////////////////////////////////////////////////////

#include "lua_task.h"
#include "lua_operator.h"
#include "plugin_log_adapter.h"
#include "client.h"
#include "inner_def.h"

/*server frame headers*/
#include "simple_kv_config.h"

/*system headers*/
#include <stdio.h>

lua_task::lua_task(){

}

lua_task::~lua_task(){

}

int lua_task::start()
{
	lua_operator_ptr p_lua = lua_operator_ptr(new lua_operator);
	string str_lua_path;
	simple_kv_config_ptr config = client::instance().get_config();

	config->get(string(lua_file_path), str_lua_path);
	p_lua->init(str_lua_path);
	bool bcall = p_lua->call(m_func_name, m_params, m_returns, m_error);
	if (false == bcall)
	{
		m_error = p_lua->get_errror();
		LOG_ERROR(m_error.c_str());
	}

	if (m_func_call_back)
	{
		m_func_call_back(shared_from_this());
	}
	return 0;
}

std::string lua_task::get_error() const
{

	return m_error;
}

std::string lua_task::get_func_name() const
{

	return m_func_name;
}

lua_task::task_params_t lua_task::get_params() const
{

	return m_params;
}

lua_task::task_returns_t lua_task::get_returns() const
{

	return m_returns;
}

void lua_task::set_call_back(lua_task::func_lua_operate_t newVal){

	m_func_call_back = newVal;
}

void lua_task::set_error(const std::string& newVal){

	m_error = newVal;
}

void lua_task::set_func_name(const std::string& newVal){

	m_func_name = newVal;
}

void lua_task::set_params(const lua_task::task_params_t& newVal){

	m_params = newVal;
}

void lua_task::set_returns(const lua_task::task_returns_t& newVal){

	m_returns = newVal;
}

void lua_task::set_return_size(size_t size)
{
	m_returns.resize(size);
}
