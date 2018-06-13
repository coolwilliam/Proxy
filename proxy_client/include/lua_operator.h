///////////////////////////////////////////////////////////
//  lua_operator.h
//  Implementation of the Class lua_operator
//  Created on:      16-����-2016 10:18:27
//  Original author: Administrator
///////////////////////////////////////////////////////////

#if !defined(EA_AC89CAB8_DD89_4a3b_AFFF_2835A37B4B6A__INCLUDED_)
#define EA_AC89CAB8_DD89_4a3b_AFFF_2835A37B4B6A__INCLUDED_

#include <string>
#include <vector>
using namespace std;

struct lua_State;

/**
 * lua����
 */
class lua_operator
{

public:
	lua_operator();
	virtual ~lua_operator();

	/**
	 * ����lua�ű�
	 */
	bool call(const string& func_name, const vector<string>& params, vector<string>& returns, string& error_msg);
	/**
	 * ��ʼ��
	 */
	int init(const string & str_lua_file);
	/**
	* �ж��Ƿ��Ѿ���ʼ��
	*/
	bool is_initialized();

	std::string get_errror() const { return m_str_error; }
	void set_error(const std::string& str_error){ m_str_error = str_error; }

private:
	/**
	* ���ó�ʼ����ʶ
	*/
	void set_initialized(bool val = true);

private:
	/**
	 * �Ƿ��Ѿ���ʼ��
	 */
	bool m_initialized;

	//lua���
	lua_State* m_lua_state;

	std::string m_str_error;

};
#endif // !defined(EA_AC89CAB8_DD89_4a3b_AFFF_2835A37B4B6A__INCLUDED_)
