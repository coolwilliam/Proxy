-- region *.lua
-- Date
-- 此文件由[BabeLua]插件自动生成
module(...);

-- 心跳数据定义
heart_beat =
{
    cmd = "",
    sn = "",
    platform = "",
    version = "",
    vs = "",
    svn = "",
    wmac = "",
    last_task = "",
    last_taskid = "",
    proxy_status = "",
    proxy_srv = "",
    proxy_port = ""
};

-- 代理任务回复
proxy_ack =
{
    cmd = "pong",
    task = "proxy",
    task_id = "123456",
    params =
    {
        server = "c.wamwifi.com",
        port = "3333"
    }
};

-- http请求任务
http_ack=
{
    cmd="",
    task=""
};

-- endregion
