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

module(...);

-- 获取配置
function get_config(id, parms)
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
        local cgi_module = "cgi.deviceHeartbeat";
        stat, ret = pcall(require, cgi_module);
        if not stat then
            c_log.log4cpp(c_log.ELL_ERROR, "Require " .. cgi_module .. " failed! Error message: " .. ret);
            break;
        end

        cgi_module = ret;

        stat, ret = cgi_module.deviceOnlineUpdate(id);
        if not stat then
             c_log.log4cpp(c_log.ELL_ERROR, "Call deviceOnlineUpdate(" .. id .. ") failed! Error message: " .. ret);
             break;
        end

    until true;
    return cjson.encode({stat = stat, ret = ret});
end

-- 命令对应的操作
local cmd_funs = { };

cmd_funs[df.EnumCmd.ELCT_LOGIN] = login;
cmd_funs[df.EnumCmd.ELCT_HEARTBEAT] = login;
cmd_funs[df.EnumCmd.ELCT_GET_CONFIG] = get_config;

function cmd_run(cmd, device_id, parms)
    c_log.log4cpp(c_log.ELL_TRACE, "Cmd=" .. cmd .. ", parms=" .. cjson.encode(parms));
    local fun = cmd_funs[tonumber(cmd)];

    if fun then
        return fun(device_id, parms);
    else
        c_log.log4cpp(c_log.ELL_ERROR, "No function for cmd " .. cmd);
        return "";
    end
end
