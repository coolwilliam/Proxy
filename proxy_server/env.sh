#!/bin/bash

webSdkRootPath=/usr/local/wayosWebProSDK
webSvrPath=/home/work/streamMedia
wx_lua_path="$webSvrPath/lua/?.lua;$webSvrPath/lua/public/?.lua;$webSvrPath/lua/base/?.lua;$webSvrPath/lua/shared/?.lua;$webSdkRootPath/openresty/lualib/?.lua"
wx_lua_cpath="$webSvrPath/lua/?.so;$webSvrPath/lua/public/?.so;$webSdkRootPath/openresty/lualib/?.so;$webSdkRootPath/openresty/luajit/lib/?.so"

export LUA_CPATH="./?.so;/usr/local/lib/lua/5.1/?.so;${PWD}/lua_obj/lualib/?.so;$wx_lua_cpath"
export LUA_PATH="./?.lua;/usr/share/luajit-2.0.4/?.lua;/usr/local/share/lua/5.1/?.lua;/usr/local/share/lua/5.1/?/init.lua;${PWD}/lua_obj/?.lua;${PWD}/lua_obj/lua/bin/?.lua;${PWD}/lua_obj/lualib/?.lua;$wx_lua_path"

#echo $LUA_CPATH;
#echo $LUA_PATH;
