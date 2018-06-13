#!/bin/bash

source ~/.bashrc

#以下变量的值，会被auto_pack.sh替换
#---start---
version=1
soft_name=name
server_name=server
flag_env_begin="#begin"
flag_env_end="#end"
env_files=files
soft_root_macro=WX_SVR_ROOT
#---end---

#本地自定义变量
#---start---

#---end---

#停止相关程序
function shutdown_program()
{
	echo "Shuting down $server_name..."
	service $server_name stop
	echo "Shutdown $server_name complete !"
}

#清除环境变量
function clear_env()
{
	date_string=`date +%Y%m%d%H%M%S`
	
	echo -e "Clearing environment values ...\c"
	
	for env_file in $env_files;do
		#判断文件是否存在
		if [ ! -z $env_file -a -e $env_file ];then
			cp $env_file $env_file.bak$date_string
			sed -i "/^$flag_env_begin/,/$flag_env_end$/d" $env_file
#			sed -i "/$flag_env_begin/d" $env_file
#			sed -i "/$flag_env_end/d" $env_file
		fi
	done
	
	#在环境变量中去除$soft_root_macro 之前的值
	unset $soft_root_macro
	
	
	ldconfig
	source ~/.bashrc
	
	echo -e "\rClear environment values complete !"
}

#删除注册信息
function del_registration()
{
	echo -e "Delete $server_name from system ...\c"
	
	chkconfig $server_name off
	chkconfig --del $server_name
	
	echo -e "\rDelete $server_name from system complete !"
	
}

#获取当前脚本所在路径
function get_cur_script_location()
{
	pwd_dir=`pwd`
	location_dir=$(cd `dirname $0`; pwd)
	cd  $pwd_dir
	
	echo "$location_dir"
}

#删除安装目录
function del_install_dir()
{
	location=$(get_cur_script_location)
	echo "Delete install dir $location..."
	
	stty erase '^H'
	read -p "Directory [$location] will be removed now, sure? [y/n]:" reply leftover
    case $reply in
    y* | Y*)
        rm -rf $location
		echo "Directory [$location] has been removed successfully !";;
    n* | N*)
        ;;
    *)
        ;;
    esac
}

#卸载
function do_uninstall()
{
	echo "Begin to uninstall ..."
	#当前脚本所在路径
	location=$(get_cur_script_location)
	
	#当前版本的软件根目录
	cur_root=`eval echo '$'"$soft_root_macro"`
	
	if [ "$location" = "$cur_root" ];then
		#如果当前脚本所在路径与环境中所在路径相同同，则进行一系列的清除动作
		shutdown_program
		del_registration
		clear_env
	fi
	
	#最后清除脚本所在安装目录
	del_install_dir
	
	echo "Uninstall complete !"
}

#主函数
function main()
{

	stty erase '^H'
	read -p "$soft_name will be uninstall now, sure? [y/n]:" reply leftover
    case $reply in
    y* | Y*)
        do_uninstall;;
    *)
        echo "Uninstall canceled !";;
    esac
}

main
