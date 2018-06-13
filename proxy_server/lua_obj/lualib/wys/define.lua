local enum = require("enum");
local print = print;
local os = os;
module(...);

enum_cmd_type =
{
    "ELCT_UNKNOWN",
    "ELCT_LOGIN",
    "ELCT_HEARTBEAT",
    "ELCT_GET_CONFIG",
    "ELCT_MAX"
};

local enum_oem = 
{
    "main"
};

EnumCmd = enum.CreatEnumTable(enum_cmd_type, -1);

EnumOem = enum.CreatEnumTable(enum_oem, -1);