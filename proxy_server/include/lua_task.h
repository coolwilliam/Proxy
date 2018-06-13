///////////////////////////////////////////////////////////
//  lua_task.h
//  Implementation of the Class lua_task
//  Created on:      16-����-2016 10:18:27
//  Original author: wuht
///////////////////////////////////////////////////////////

#if !defined(EA_FDAFAC6B_463D_4250_A034_BBBB09877AD2__INCLUDED_)
#define EA_FDAFAC6B_463D_4250_A034_BBBB09877AD2__INCLUDED_

#include "task_obj.h"
#include "ptr_define.h"

/*boost headers*/
#include <boost/function.hpp>
#include <boost/enable_shared_from_this.hpp>

/*system headers*/
#include <vector>
#include <string>

/*server frame headers*/
#include "common_macro.h"


/**
 * lua����
 */
class lua_task : public task_obj, public boost::enable_shared_from_this<lua_task>
{

public:
	typedef boost::function<void(lua_task_ptr)> func_lua_operate_t;

	typedef std::vector<std::string> task_params_t; 

	typedef std::vector<std::string> task_returns_t;


	lua_task();
	virtual ~lua_task();

	virtual int start();
	/**
	 * ������Ϣ
	 */
	std::string get_error() const;
	/**
	 * lua������
	 */
	std::string get_func_name() const;
	/**
	 * �������
	 */
	lua_task::task_params_t get_params() const;
	/**
	 * ����ֵ
	 */
	lua_task::task_returns_t get_returns() const;
	/**
	 * ����ص���������ִ����ɺ󣬵��ã�
	 */
	void set_call_back(lua_task::func_lua_operate_t newVal);
	/**
	 * ������Ϣ
	 */
	void set_error(const std::string& newVal);
	/**
	 * lua������
	 */
	void set_func_name(const std::string& newVal);
	/**
	 * �������
	 */
	void set_params(const lua_task::task_params_t& newVal);
	/**
	 * ����ֵ
	 */
	void set_returns(const lua_task::task_returns_t& newVal);

	/**
	 * ���÷���ֵ������С
	 */
	void set_return_size(size_t size);
private:
	/**
	 * ������Ϣ
	 */
	std::string m_error;
	/**
	 * ����ص���������ִ����ɺ󣬵��ã�
	 */
	lua_task::func_lua_operate_t m_func_call_back;
	/**
	 * lua������
	 */
	std::string m_func_name;
	/**
	 * �������
	 */
	lua_task::task_params_t m_params;
	/**
	 * ����ֵ
	 */
	lua_task::task_returns_t m_returns;

private:
	DISABLE_COPY(lua_task)

};
#endif // !defined(EA_FDAFAC6B_463D_4250_A034_BBBB09877AD2__INCLUDED_)
