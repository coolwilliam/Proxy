#include "inner_def.h"
#include "plugin_log_adapter.h"

//////////////////////////////ÅäÖÃÏî¶¨Òå////////////////////////////////////////////
//{{
config_define(network_thread_count);
config_define(lua_file_path);
config_define(device_server_port);
config_define(web_request_port);
config_define(proxy_client_port);
config_define(domain_ip);
//}}

bool check_json_member(const RAPIDJSON_NAMESPACE::Value& json_value, const vect_json_member_t& vect_member)
{
	vect_json_member_t::const_iterator it_member = vect_member.begin();
	for (; it_member != vect_member.end(); ++it_member)
	{
		if (json_value.HasMember(it_member->c_str()) == false)
		{
			RAPIDJSON_NAMESPACE::StringBuffer buffer;
			RAPIDJSON_NAMESPACE::Writer<RAPIDJSON_NAMESPACE::StringBuffer> writer(buffer);
			json_value.Accept(writer);

			LOG_ERROR("Check json member failed ! [" << it_member->c_str() << "] is missing!" << std::endl << "Source json:" << buffer.GetString());
			return false;
		}
	}

	return true;

}
