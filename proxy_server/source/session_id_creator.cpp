///////////////////////////////////////////////////////////
//  session_id_creator.cpp
//  Implementation of the Class session_id_creator
//  Created on:      09-八月-2016 11:35:03
//  Original author: Administrator
///////////////////////////////////////////////////////////

#include "session_id_creator.h"

session_connection::session_id_t gl_session_id = 1;
session_connection::session_id_t gl_msg_id = 1;

boost::mutex session_id_creator::m_mtx_session_id;
boost::mutex session_id_creator::m_mtx_msg_id;

#define MAX_SESSION_ID 4000000000


session_connection::session_id_t session_id_creator::create_session_id(){
	boost::mutex::scoped_lock slock(m_mtx_session_id);

	//生成session_id
	session_connection::session_id_t session_id = gl_session_id++;
	
	if (session_id == MAX_SESSION_ID)
	{
		gl_session_id = 1;
	}

	return session_id;
}

session_connection::session_id_t session_id_creator::create_msg_id()
{
	boost::mutex::scoped_lock slock(m_mtx_msg_id);
	//生成msg_id
	session_connection::session_id_t msg_id = gl_msg_id++;

	if (msg_id == MAX_SESSION_ID)
	{
		gl_msg_id = 1;
	}

	return msg_id;
}
