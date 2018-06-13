#!/usr/sbin/lua
--region *.lua
--Date
--此文件由[BabeLua]插件自动生成
local socket = require("socket.core");
local define = require("wys.define");
local cf = require("wys.config");
local cjson = require("cjson");
local c_log = require("c_log");

local os = os;

function connect(host, port)
    local stat = true;
    local res = "";
    local sck, err;
    repeat
        sck, err = socket.tcp();
        if not sck then
            stat = false;
            res = err;
            break;
        end

        set_timeout(sck, 3000);

        stat, res = sck:connect(host, port);
        if stat then
            stat = true;
            res = sck;
            break;
        end
    until true;

    return stat, res;
end

function set_timeout(sock, timeout)
    return sock:settimeout(timeout);
end

function requestToCppSvr(data, needReply)
    local stat = true;
    local res = "";

    repeat
        local connStat, sock = connect("127.0.0.1", cf.conf_server_port);
        if not connStat then
            c_log.log4cpp(c_log.ELL_ERROR, "Connect to server failed! Error:" .. sock);
            stat = connStat;
            break;
        end

        local bytes, err = sock:send(cjson.encode(data) .. "\r\n");
        if not bytes then
            c_log.log4cpp(c_log.ELL_ERROR, "Send data to server failed! Error:" .. err);
            stat = bytes;
            break;
        end

        sock:settimeout(5000);
        local recvData, err, partial;
        if needReply == nil or needReply == true then
            recvData, err, partial = sock:receive("*a");
            if not recvData then
                c_log.log4cpp(c_log.ELL_ERROR, "Receive from server failed! Error:" .. err);
                stat = recvData;
                res = err;
                break;
            end
        elseif needReply ~= nil and needReply == false then
            -- Do nothing;
        end

        stat = true;
        res = recvData;

        c_log.log4cpp(c_log.ELL_INFO, "Receive data:" .. "\r\n" .. (recvData or "null"));
    until true;

    return stat, res;
end

function testNotifyUpdateFirmware()
    local stat = true;
    local res = "";

    repeat
        local data =
        {
            version = 1,
            cmd = define.EnumWebApiCmd.EWCC_NOTIFY_CHECK_FIRMWARE_UPDATE,
            mode = 2,
            msg_id = 0,
            parms =
            {
                rid_array = 
                {
                    "10001",
                    "10002"
                }
            }
        }

        stat, res = requestToCppSvr(data, false);
    until true;

    return stat, res;
end

while true do
    -- testGetOnlineUser();
    if not testNotifyUpdateFirmware() then break; end
end


--endregion
