-- region *.lua
-- Date
-- 此文件由[BabeLua]插件自动生成

local enum = require("enum")
local socket = require("socket")
local type = type;
local pairs = pairs;
local getmetatable = getmetatable;
local setmetatable = setmetatable;
local table = table;
local assert = assert;

local type_table =
{
    "table",
    "string",
    "boolean",
    "number"
};

local EnumLuaType = enum.CreatEnumTable(type_table);

module(...)

function is_number(var)
    if EnumLuaType[type(var)] == EnumLuaType.number then
        return true;
    else
        return false;
    end
end

function is_string(var)
    if EnumLuaType[type(var)] == EnumLuaType.string then
        return true;
    else
        return false;
    end
end

function is_bool(var)
    if EnumLuaType[type(var)] == EnumLuaType.boolean then
        return true;
    else
        return false;
    end
end

function is_table(var)
    if EnumLuaType[type(var)] == EnumLuaType.table then
        return true;
    else
        return false;
    end
end

-- 比较两个表是否相同
function compare_table(first, second, parent_node_key)

    -- 是否成功进行
    local success = true;
    -- 两个表是否完全相同
    local same = true;

    -- 错误信息
    local err_msg = "";

    repeat
        if not(is_table(first) and is_table(second)) then
            success = false;
            same = false;
            err_msg = "Error param type, " ..(parent_node_key and("[" .. parent_node_key .. "] ")) or "" .. "(" .. type(first) .. ", " .. type(second) .. ")";
            break;
        end

        for k, v in pairs(first) do
            local prop_v = second[k];

            -- 1、检查成员变量是否齐全
            if prop_v == nil then
                success = false;
                same = false;
                err_msg =(parent_node_key and("[" .. parent_node_key .. "." .. k .. "] ")) or("[" .. k .. "]") .. " from first, can't be found in second";
                break;
            end

            -- 2、检查各个成员的类型和值是否正确
            if type(prop_v) ~= type(v) then
                success = false;
                same = false;
                err_msg = "Type of " ..(parent_node_key and(parent_node_key .. "." .. k)) or k .. " is different, (" .. type(v) .. "," .. type(prop_v) .. ")"
                break;
            end

            -- 3、检查对应的值是否相同
            if is_table(prop_v) then
                local parent_key =(parent_node_key or "") .. "." .. k;
                success, same, err_msg = compare_table(v, prop_v, parent_key);
                if not success or not same then
                    break;
                end
            elseif prop_v ~= v then
                local fullpath_key =((parent_node_key and(parent_node_key .. ".")) or "") .. k;
                success = true;
                same = false;
                err_msg = "[" .. fullpath_key .. "] Value different, (" .. v .. ", " .. prop_v .. ")"
                break;
            end
        end
    until true;

    return success, same, err_msg;
end

-- 检查属性是否合法
function match_table(table_src, table_base)

    local res = true;
    local msg = "";

    for k, v in pairs(table_base) do
        local prop_v = table_src[k];

        -- 1、检查成员变量是否齐全
        if prop_v == nil then
            res = false;
            msg = "Property value of key[" .. k .. "] is nil";
            break;
        end

        -- 2、检查各个成员的类型和值是否正确
        local type_base = type(v);
        local type_src = type(prop_v);
        if type_base ~= type_src then
            res = false;
            msg = "Failed to match type! base[" .. type_base .. "], src[" .. type_src .. "]";
            break;
        end
    end

    return res, msg;
end

-- 克隆table
function clone(object)
    local lookup_table = { }
    local function copyObj(object)
        if type(object) ~= "table" then
            return object
        elseif lookup_table[object] then
            return lookup_table[object]
        end

        local new_table = { }
        lookup_table[object] = new_table
        for key, value in pairs(object) do
            new_table[copyObj(key)] = copyObj(value)
        end
        return setmetatable(new_table, getmetatable(object))
    end
    return copyObj(object)
end

-- 根据域名获取IP
function get_ip_from_host(hostname)
    local ip, resolved = socket.dns.toip(hostname);

    return ip;
end

-- 分割字符串为数组
function split(data, seperator)
    if data==nil or data=='' or seperator==nil then
		return nil
	end
	
    local result = {}
    for match in (data..seperator):gmatch("(.-)"..seperator) do
        table.insert(result, match)
    end
    return result
end

-- 清除字符串中所有空白
function trim(str)
    local type=type;
    assert(type(str) == "string");

    return str:gsub("[%s\t]", "");
end
-- endregion
