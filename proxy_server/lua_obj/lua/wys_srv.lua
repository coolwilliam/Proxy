local cjson = require("cjson")
local cf = require("wys.config")
local df = require("wys.define")
local c_func = require("common_function");
local c_log = require("c_log");
local data_df = require("data_define");

local print = print;
local type = type;
local pcall = pcall;
local pairs = pairs;
local os = os;
local tonumber = tonumber;
local require = require;
local tostring = tostring;

local function updateDeviceStatus(id, params)
	local stat = false;
	local ret = "";

	repeat
		local cgi_module = "cgi.deviceHeartbeat";
		stat, ret = pcall(require, cgi_module);
		if not stat then
			c_log.log4cpp(c_log.ELL_ERROR, "Require " .. cgi_module .. " failed! Error message: " .. ret);
			break;
		end

		cgi_module = ret;

		stat, ret = cgi_module.deviceOnlineUpdate(id, params);
		if not stat then
			c_log.log4cpp(c_log.ELL_ERROR, "Call deviceOnlineUpdate(" .. id .. ") failed! Error message: " .. ret);
			break;
		end

	until true;

	return stat, ret
end

module(...);

-- 获取配置
function get_config(id, params)
    local new_domain_ip = c_func.get_ip_from_host(cf.domain_ip);
    if nil ~= new_domain_ip and new_domain_ip:len() ~= 0 then
        cf.domain_ip = new_domain_ip;
    end
    local cf_content = cjson.encode(cf);
    c_log.log4cpp(c_log.ELL_INFO, "Config:" .. cf_content);
    return cf_content;
end

-- 设备登陆
function login(id, params)
    local stat = false;
    local ret = "";

    repeat
       stat, ret = updateDeviceStatus(id, params);
	   if not stat then
			break;
	   end

	   if c_func.is_string(ret) then
		ret = 
		{
			message = ret
		};
	   end

	   ret.proxy_recv_cache_size = cf.proxy_receive_cache_size

    until true;
    return cjson.encode({stat = stat, ret = ret});
end

-- 心跳
function heart_beat(id, params)
	repeat
       stat, ret = updateDeviceStatus(id, params);
	until true;
    return cjson.encode({stat = stat, ret = ret});
end

-- 命令对应的操作
local cmd_funs = { };

cmd_funs[df.EnumCmd.ELCT_LOGIN] = login;
cmd_funs[df.EnumCmd.ELCT_HEARTBEAT] = heart_beat;
cmd_funs[df.EnumCmd.ELCT_GET_CONFIG] = get_config;

function cmd_run(cmd, device_id, params)
    c_log.log4cpp(c_log.ELL_TRACE, "Cmd=" .. cmd .. ", params=" .. cjson.encode(params));
    local fun = cmd_funs[tonumber(cmd)];

    if fun then
        return fun(device_id, params);
    else
        c_log.log4cpp(c_log.ELL_ERROR, "No function for cmd " .. cmd);
        return "";
    end
end
