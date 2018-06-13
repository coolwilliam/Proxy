local _M = {};

_M.conf_server_port=20000
_M.conn_server_port=20007
_M.proxy_server_port=20001
_M.mutaul_server_port=40003
_M.MAIN_HOST_DOMAIN="";
_M.mutual_server_ip=_M.MAIN_HOST_DOMAIN;
_M.router_hb_time=60;
_M.router_st_time=300;
_M.se_conn_timeout=60;
_M.ssdb_thread_pool_size=10;

--ssdb server ip 
_M.SSDB_SERVER_IP="127.0.0.1";
--ssdb server port
_M.SSDB_SERVER_PORT=8888;


--router heart time xx second
_M.HEART_TIME="10";
_M.CHECK_TIME="5";

--ssdb scan max
_M.SCAN_MAX=10000;


return _M;
