#pragma once
#include "msg_define.h"

/*system headers*/
#include <string>

/*server frame headers*/
#include "data_buffer.h"

class msg_function
{
public:
	static void encode(_u16 cmd, const std::string& body, data_buffer& data_out, msg_head_ptr src_head = NULL);

};

