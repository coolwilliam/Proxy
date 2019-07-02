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

local local_module = "enum";
local r_stat, enum = pcall(require, local_module);
if not r_stat then
    c_log.log4cpp(c_log.ELL_ERROR, "Require " .. local_module .. " failed!" .. ((enum and " Error message: " ..enum) or "null"));
    return;
end

module(...);

-- 获取配置
function get_config(rid, parms)
    local new_domain_ip = c_func.get_ip_from_host(cf.domain_ip);
    if nil ~= new_domain_ip and new_domain_ip:len() ~= 0 then
        cf.domain_ip = new_domain_ip;
    end
        -- 获取设备ID
    local interface_module = "iptvauth";
    local stat, ret = pcall(require, interface_module);
    if not stat then
        c_log.log4cpp(c_log.ELL_ERROR, "Require " .. interface_module .. " failed! Error message: " .. ret);
    else
        local interface = ret;
        stat, ret = interface.state();
        if stat then
            cf.device_id = ret.serial;
        else
            c_log.log4cpp(c_log.ELL_ERROR, "Call interface.state failed! Error message:" .. ret);
        end
    end
    
    local cf_content = cjson.encode(cf);
    c_log.log4cpp(c_log.ELL_INFO, "Config:" .. cf_content);
    return cf_content;
end

-- 命令对应的操作
local cmd_funs = { };

cmd_funs[df.EnumCmd.ELC_GET_CONFIG] = get_config;

function cmd_run(cmd, parms)
    c_log.log4cpp(c_log.ELL_TRACE, "Cmd=" .. cmd .. ", parms=" .. cjson.encode(parms));
    local fun = cmd_funs[tonumber(cmd)];

    if fun then
        return fun(parms);
    else
        c_log.log4cpp(c_log.ELL_ERROR, "No function for cmd " .. cmd);
        return "";
    end
end
