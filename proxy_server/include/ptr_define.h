#ifndef ptr_define_h__
#define ptr_define_h__
/*!
 * \file ptr_define.h
 * \date 2018/03/23 10:36
 *
 * \author William Wu
 * Contact: wuhaitao@wayos.cn
 *
 * \brief Contains shared pointer of classes
 *
 *
 * \note
 * First created by wuhaitao at 2018/03/23 10:36
*/

#include <boost/shared_ptr.hpp>

class lua_task;
typedef boost::shared_ptr<lua_task> lua_task_ptr;

class lua_operator;
typedef boost::shared_ptr<lua_operator> lua_operator_ptr;

class device;
typedef boost::shared_ptr<device> device_ptr;

#endif // ptr_define_h__
