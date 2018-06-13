local cf = require("wys.config");
local c_log = require("c_log");
local def = require("wys.define");
local cjson = require("cjson");
local c_func = require("common_function");

local pcall = pcall;
local srv;
local r_stat;

-- 根据各个OEM配置的业务处理模块
local svr_modules = {};

svr_modules[def.EnumOem.main] = "lua.wys_srv";

-- 根据配置加载不同的模块
local local_module = svr_modules[cf.oem];
if not local_module then
    c_log.log4cpp(c_log.ELL_ERROR, "There is no bussiness module for oem " .. (cf.oem or "") .. " !");
    return;
end

r_stat, srv = pcall(require, local_module);
if not r_stat then
    c_log.log4cpp(c_log.ELL_ERROR, "Require " .. local_module .. " failed!" .. ((srv and " Error message: " .. srv) or ""));
    return;
end

function lobj_main(cmd, param)
  local ret = "";
  if nil ~= param and c_func.is_string(param) and param:len() ~= 0 then
  	param = cjson.decode(tostring(param));
  end

  ret = srv.cmd_run(cmd, param);

  return ret;

end

