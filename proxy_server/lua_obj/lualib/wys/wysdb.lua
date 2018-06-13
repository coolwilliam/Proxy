-- local say   = ngx.say
local config = require "wys.config"
local cjson = require "cjson"
local ssdb = require "resty.cbsdb"
local db;
local OK 	= true
local ERR 	= false

local _M = { _VERSION = '0.01' }
local mt = { __index = _M }


-- 计数器前缀
_M.COUNTER = 'ct-'

-- 给指定前缀计数器+1
function _M.set_counter(pre)
    local tkey = _M.COUNTER .. pre

    if _M.new() then
        if db:exists(tkey) then
            return db:incr(tkey, 1)
        else
            return db:set(tkey, 1)
        end
    end

    return ERR;
end 

-- 给指定前缀计数器-1
function _M.sub_counter(pre)
    local tkey = _M.COUNTER .. pre

    if _M.new() then
        if db:exists(tkey) then
            return db:incr(tkey, -1)
        end
    end

    return ERR;
end 

-- 获取指定前缀计数器
function _M.get_counter(pre)
    local res = _M.get(_M.COUNTER, pre)
    local cnt = 0
    if res[1] ~= "not_found" then
        cnt = res
    end
    return cnt
end 


-- 模糊查询
function _M.fv_scan(pre, s, e, tet, snum, enum)
    local t1, t2
    if string.len(s) == 0 then
        t1 = pre
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = string.gsub(pre, '-', '.')
    else
        t2 = pre .. e
    end

    if _M.new() then
        return db:fv_scan(t1, t2, tet, snum, enum)
    end

    return ERR;
end

-- fjv_scan
function _M.fjv_scan(pre, s, e, tet, snum, enum)
    local t1, t2
    if string.len(s) == 0 then
        t1 = pre
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = string.gsub(pre, '-', '.')
    else
        t2 = pre .. e
    end

    if _M.new() then
        return db:fjv_scan(t1, t2, tet, snum, enum)
    end

    return ERR;
end

-- fjev_scan
function _M.fjev_scan(pre, s, e, tet, snum, enum)
    local t1, t2
    if string.len(s) == 0 then
        t1 = pre
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = string.gsub(pre, '-', '.')
    else
        t2 = pre .. e
    end

    if _M.new() then
        return db:fjev_scan(t1, t2, tet, snum, enum)
    end

    return ERR;
end

-- 对值的模糊查询
function _M.fv_rscan(pre, s, e, tet, snum, enum)
    local t1, t2
    if string.len(s) == 0 then
        t1 = string.gsub(pre, '-', '.')
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = pre
    else
        t2 = pre .. e
    end

    if _M.new() then
        return db:fv_rscan(t1, t2, tet, snum, enum)
    end

    return ERR;
end


function _M.fk_rkeys(pre, s, e, tet, st, et)
    local t1, t2
    if string.len(s) == 0 then
        t1 = string.gsub(pre, '-', '.')
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = pre
    else
        t2 = pre .. e
    end

    if _M.new() then
        return db:fk_rkeys(t1, t2, tet, st, et)
    end

    return ERR;
end


function _M.resFun(pre, res)
    local tt = { }
    local pre_len = string.len(pre) + 1

    for key, value in ipairs(res) do
        if (key % 2) == 0 then
            tt[key] = value
        else
            tt[key] = string.sub(value, pre_len)
        end
    end
    return tt
end


function _M.set(pre, key, val)
    local tkey = pre .. key

    if _M.new() then
        return db:set(tkey, val)
    end

    return ERR;
end

function _M.setx(pre, key, val, time)
    local tkey = pre .. key;

    if _M.new() then
        return db:setx(tkey, val, time);
    end

    return ERR;
end

function _M.setnx(pre, key, val)
    local tkey = pre .. key

    if _M.new() then
        return db:setnx(tkey, val)
    end

    return ERR;
end


function _M.get(pre, key)
    local tkey = pre .. key

    if _M.new() then
        return db:get(tkey)
    end

    return ERR;
end


function _M.exists(pre, key)
    local tkey = pre .. key

    if _M.new() then
        return db:exists(tkey)
    end

    return ERR;
end


function _M.del(pre, key)
    local tkey = pre .. key

    if _M.new() then
        return db:del(tkey)
    end

    return ERR;
end


function _M.incr(pre, key, val)
    local tkey = pre .. key

    if _M.new() then
        return db:incr(tkey, val)
    end

    return ERR;
end


function _M.decr(pre, key, val)
    local tkey = pre .. key

    if _M.new() then
        return db:decr(tkey, val)
    end

    return ERR;
end

function _M.hset(pre, hname, key, val)
    local thname = pre .. hname
    local tkey = pre .. key

    if _M.new() then
        return db:hset(thname, tkey, val)
    end

    return ERR;
end

function _M.hget(pre, hname, key, val)
    local thname = pre .. hname
    local tkey = pre .. key

    if _M.new() then
        return db:hget(thname, tkey, val)
    end

    return ERR;
end

function _M.hdel(pre, hname, key)
    local thname = pre .. hname
    local tkey = pre .. key

    if _M.new() then
        return db:hdel(thname, tkey)
    end

    return ERR;
end

function _M.hincr(pre, hname, key, num)
    local thname = pre .. hname
    local tkey = pre .. key
    
    if _M.new() then
        return db:hincr(thname, tkey, num)
    end

    return ERR;
end

function _M.hexists(pre, hname, key)
    local thname = pre .. hname
    local tkey = pre .. key

    if _M.new() then
        return db:hexists(thname, tkey)
    end
    
    return ERR;
end

function _M.hsize(pre, hname)
    local thname = pre .. hname

    if _M.new() then
        return db:hsize(thname)
    end

    return ERR;
end


function _M.multi_set(pre, objs)
    local arr = { }

    for key, value in ipairs(objs) do
        -- index 从1 开始的
        if (key % 2) == 0 then
            arr[key] = value
        else
            arr[key] = pre .. value
        end
    end

    if _M.new() then
        return db:multi_set(unpack(arr))
    end

    return ERR;
end


function _M.multi_get(pre, objs)
    local arr = { }

    for key, value in ipairs(objs) do
        arr[key] = pre .. value
    end

    if _M.new() then
        return db:multi_get(unpack(arr))
    end

    return ERR;
end


function _M.multi_del(pre, objs)
    local arr = { }

    for key, value in ipairs(objs) do
        arr[key] = pre .. value
    end

    if _M.new() then
        return db:multi_del(unpack(arr))
    end

    return ERR;
end


function _M.multi_exists(pre, objs)
    local arr = { }
    for key, value in ipairs(objs) do
        arr[key] = pre .. value
    end

    if _M.new() then
        return db:multi_exists(unpack(arr))
    end

    return ERR;
end

function _M.scan(pre, s, e, num)
    local t1, t2
    if string.len(s) == 0 then
        t1 = pre
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = string.gsub(pre, '-', '.')
    else
        t2 = pre .. e
    end

    if _M.new() then
        return db:scan(t1, t2, num)
    end

    return ERR;
end

function _M.rscan(pre, s, e, num)
    local t1, t2
    if string.len(s) == 0 then
        t1 = pre
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = string.gsub(pre, '-', '.')
    else
        t2 = pre .. e
    end

    if _M.new() then
        return db:rscan(t1, t2, num)
    end

    return ERR;
end

--[[ 查找某一段数据中的一段数据 ]] 
function _M.sp_scan(pre, s, e, num_s, num_e)
    local t1, t2
    if string.len(s) == 0 then
        t1 = pre
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = string.gsub(pre, '-', '.')
    else
        t2 = pre .. e
    end

    if _M.new() then
        return db:sp_scan(t1, t2, num_s, num_e)
    end

    return ERR;
end
 
--[[ 查找某一段数据中的一段数据，反向 ]]
function _M.sp_rscan(pre, s, e, num_s, num_e)
    local t1, t2
    if string.len(s) == 0 then
        t1 = string.gsub(pre, '-', '.')
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = pre
    else
        t2 = pre .. e
    end
    if _M.new() then
        return db:sp_rscan(t1, t2, num_s, num_e)
    end

    return ERR;
end


function _M.qpush_back(pre, key, val)
    local name = pre .. key
    if _M.new() then
        return db:qpush_back(name, val)
    end

    return ERR;
end 

function _M.qpop_back(pre, key, size)
    local name = pre .. key

    if _M.new() then
        return db:qpop_back(name, size)
    end
    return ERR;
end 

-- 根据键值数组删除数据
function _M.del_keys(pre, s, e, num_max)

    local t1, t2
    if string.len(s) == 0 then
        t1 = pre
    else
        t1 = pre .. s
    end

    if string.len(e) == 0 then
        t2 = string.gsub(pre, '-', '.')
    else
        t2 = pre .. e
    end
    if _M.new() then
        return db:del_keys(t1, t2, num_max);
    end

    return OK;
end

-- 关闭数据库客户端
function _M.close()
    return db:close()
end

-- 新建数据库客户端对象
function _M.new()
    -- 如果不为空，则不重新创建
    if nil ~= db then
        return OK;
    end

    db = ssdb:new();
    local ok, err = db:set_timeout(5000)
    local ok, err = db:connect(config.SSDB_SERVER_IP, config.SSDB_SERVER_PORT)
    if ok then
       _M.db = db;
       return OK;
    else
        _M.db = nil;
        return ERR;
    end
end

return _M
