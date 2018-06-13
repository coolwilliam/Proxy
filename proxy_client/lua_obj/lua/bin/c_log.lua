-- region *.lua
-- Date
-- 此文件由[BabeLua]插件自动生成
local enum = require("enum");
local common_func = require("common_function");
local assert = assert;
local tostring = tostring;
local debug = debug;
local print = print;

-- cpp中的回调函数
local write_log = write_log;

local enum_log_level =
{
    "ELL_TRACE",
    "ELL_ERROR",
    "ELL_CRITICAL",
    "ELL_INFO"
};

local EnumLogLevel = enum.CreatEnumTable(enum_log_level, -1);

module(...);

dbg = debug;

-- 日志级别
ELL_TRACE = EnumLogLevel.ELL_TRACE;
ELL_ERROR = EnumLogLevel.ELL_ERROR;
ELL_CRITICAL = EnumLogLevel.ELL_CRITICAL;
ELL_INFO = EnumLogLevel.ELL_INFO;

-- 写日志函数
function log4cpp(log_level, log_content, debug_table)
    assert(common_func.is_number(log_level) and common_func.is_string(log_content));

    if not write_log then
        write_log = print;
    end

    if write_log == print then
        log_level = enum_log_level[log_level + 1];
    end

    local dbg_table = debug_table or dbg.getinfo(2);

    write_log(log_level, "\n[" .. dbg_table.short_src .. ":" .. dbg_table.currentline .. ":" ..(dbg_table.name or tostring(dbg_table.func) or "") .. "]: " .. log_content);
end

-- endregion
