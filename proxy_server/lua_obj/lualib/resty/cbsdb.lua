-- Copyright (C) 2013 LazyZhu (lazyzhu.com)
-- Copyright (C) 2013 IdeaWu (ideawu.com)
-- Copyright (C) 2012 Yichun Zhang (agentzh)


local sub = string.sub
local m_socket = require("socket.core")
local insert = table.insert
local concat = table.concat
local len = string.len
--local null = ngx.null
local pairs = pairs
local unpack = unpack
local setmetatable = setmetatable
local tonumber = tonumber
local error = error
local gmatch = string.gmatch
local remove = table.remove


module(...)

_VERSION = '0.02'

local commands = {
    "set",                  "get",                 "del",
	"setx",                 "setnx",               "ttl",
	"expire",               "getset",              "setbit",
	"getbit",               "bitcount",             "substr",
    "scan",                 "rscan",               "keys",
    "incr",                 "decr",                "exists",
    "multi_set",            "multi_get",           "multi_del",
    "multi_exists",         "sp_scan",   "sp_rscan",
	
    "hset",                 "hget",                "hdel",
	"hincr",                "hexists",             "hsize",
	"hlist",                "hkeys",               "hgetall",
    "hscan",                "hrscan",              "hclear",             
    "hdecr",               
	
    --[[ "multi_hset", ]]   "multi_hget",          "multi_hdel",
    "multi_hexists",        "multi_hsize",
    "zset",                 "zget",                "zdel",
    "zscan",                "zrscan",              "zkeys",
    "zincr",                "zdecr",               "zexists",
    "zsize",                "zlist",
    --[[ "multi_zset", ]]   "multi_zget",          "multi_zdel",
    "multi_zexists",        "multi_zsize",
	"qsize",				"qlist",				"qrlist",
	"qclear",				"qfront",				"qback",
	"qget",					"qset",					"qrange",
	"qslice",				"qpush",				"qpush_front",
	"qpush_back",			"qpop",					"qpop_front",
	"qpop_back",			"qtrim_front",			"qtrim_back" ,
    "sp_scan",		"sp_rscan",   "fv_scan" , 
    "fv_rscan",   "fk_keys" , "fk_rkeys", "fjev_scan",
    "del_keys"
}


local mt = { __index = _M }


function new(self)
    local sock, err = m_socket.tcp()
    if not sock then
        return nil, err
    end
    return setmetatable({ sock = sock }, mt)
end


function set_timeout(self, timeout)
    local sock = self.sock
    if not sock then
        return nil, "not initialized"
    end

    return sock:settimeout(timeout)
end


function connect(self, ...)
    local sock = self.sock
    if not sock then
        return nil, "not initialized"
    end

    return sock:connect(...)
end



function close(self)
    local sock = self.sock
    if not sock then
        return nil, "not initialized"
    end

    return sock:close()
end


local function _read_reply(sock)
	local val = {}

	while true do
		-- read block size
		local line, err, partial = sock:receive()
		if not line or len(line)==0 then
			-- packet end
			break
		end
		local d_len = tonumber(line)

		-- read block data
		local data, err, partial = sock:receive(d_len)
		insert(val, data);

		-- ignore the trailing lf/crlf after block data
		local line, err, partial = sock:receive()
	end

	local v_num = tonumber(#val)

	if v_num == 1 then
		return val
	else
		remove(val,1)
		return val
	end
end


local function _gen_req(args)
    local req = {}

    for i = 1, #args do
        local arg = args[i]

        if arg then
            insert(req, len(arg))
            insert(req, "\n")
            insert(req, arg)
            insert(req, "\n")
        else
            return nil, err
        end
    end
    insert(req, "\n")

    -- it is faster to do string concatenation on the Lua land
    -- print("request: ", table.concat(req, ""))

    return concat(req, "")
end


local function _do_cmd(self, ...)
    local args = {...}

    local sock = self.sock
    if not sock then
        return nil, "not initialized"
    end

    local req = _gen_req(args)

    local reqs = self._reqs
    if reqs then
        insert(reqs, req)
        return
    end

    local bytes, err = sock:send(req)
    if not bytes then
        return nil, err
    end

    return _read_reply(sock)
end


for i = 1, #commands do
    local cmd = commands[i]

    _M[cmd] =
        function (self, ...)
            return _do_cmd(self, cmd, ...)
        end
end


function multi_hset(self, hashname, ...)
    local args = {...}
    if #args == 1 then
        local t = args[1]
        local array = {}
        for k, v in pairs(t) do
            insert(array, k)
            insert(array, v)
        end
        -- print("key", hashname)
        return _do_cmd(self, "multi_hset", hashname, unpack(array))
    end

    -- backwards compatibility
    return _do_cmd(self, "multi_hset", hashname, ...)
end


function multi_zset(self, keyname, ...)
    local args = {...}
    if #args == 1 then
        local t = args[1]
        local array = {}
        for k, v in pairs(t) do
            insert(array, k)
            insert(array, v)
        end
        -- print("key", keyname)
        return _do_cmd(self, "multi_zset", keyname, unpack(array))
    end

    -- backwards compatibility
    return _do_cmd(self, "multi_zset", keyname, ...)
end


function init_pipeline(self)
    self._reqs = {}
end


function cancel_pipeline(self)
    self._reqs = nil
end


function commit_pipeline(self)
    local reqs = self._reqs
    if not reqs then
        return nil, "no pipeline"
    end

    self._reqs = nil

    local sock = self.sock
    if not sock then
        return nil, "not initialized"
    end

    local bytes, err = sock:send(reqs)
    if not bytes then
        return nil, err
    end

    local vals = {}
    for i = 1, #reqs do
        local res, err = _read_reply(sock)
        if res then
            insert(vals, res)

        elseif res == nil then
            return nil, err

        else
            insert(vals, err)
        end
    end

    return vals
end


function array_to_hash(self, t)
    local h = {}
    for i = 1, #t, 2 do
        h[t[i]] = t[i + 1]
    end
    return h
end


local class_mt = {
    -- to prevent use of casual module global variables
    __newindex = function (table, key, val)
        error('attempt to write to undeclared variable "' .. key .. '"')
    end
}


function add_commands(...)
    local cmds = {...}
    local newindex = class_mt.__newindex
    class_mt.__newindex = nil
    for i = 1, #cmds do
        local cmd = cmds[i]
        _M[cmd] =
            function (self, ...)
                return _do_cmd(self, cmd, ...)
            end
    end
    class_mt.__newindex = newindex
end


setmetatable(_M, class_mt)

