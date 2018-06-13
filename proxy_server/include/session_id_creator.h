///////////////////////////////////////////////////////////
//  session_id_creator.h
//  Implementation of the Class session_id_creator
//  Created on:      09-八月-2016 11:35:03
//  Original author: Administrator
///////////////////////////////////////////////////////////

#if !defined(EA_DA111CD6_2D0B_4823_A77A_E2CF60EC4406__INCLUDED_)
#define EA_DA111CD6_2D0B_4823_A77A_E2CF60EC4406__INCLUDED_

#include "session_connection.h"
#include "common_macro.h"
#include "boost/thread.hpp"


/**
 * 会话ID生成器
 */
class session_id_creator
{

public:

	static session_connection::session_id_t create_session_id();

	//生成msg_id
	static session_connection::session_id_t create_msg_id();

private:
	DISABLE_COPY(session_id_creator)
private:
	static boost::mutex	m_mtx_session_id;
	static boost::mutex m_mtx_msg_id;
};
#endif // !defined(EA_DA111CD6_2D0B_4823_A77A_E2CF60EC4406__INCLUDED_)
