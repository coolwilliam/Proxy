#!/bin/bash

source ~/.bashrc

#以下变量值会被auto_pack.sh覆盖
#---start--------------
bug_stat=alpha
extern_state=""
date_string=""
#服务名称
server_name:=bs_v3_d

#工程名称
project_name=bs_v3

#版本
version:=""

#定义软件名
soft_name:=""

#原始包名
package_name:=$soft_name.tar.gz

#oem选项
oem_options=(oem1 oem2)

#可执行文件
excutable_files:=""

#---end----------------

#命令
Sed='sed'

if [[ $extern_state != "" ]]; then
	version=$version"-"$extern_state"-"$date_string
fi

AppFolder="/home/work"
InstallPath=



#依赖的软件
depend_rpm=" "
depend_rpm+=" "lua-5.1
#depend_rpm+=" "ssdb

#依赖的命令
depend_cmd=" "
depend_cmd+=" "nc

#软件环境变量名
typeset -u soft_root_macro
soft_root_macro=${project_name}_ROOT

#系统环境变量自定义范围标识
flag_env_begin="#$project_name env begin"
flag_env_end="#$project_name env end"

#系统环境文件列表
rc_file_path=~/.bashrc
env_files=
env_files+=" "$rc_file_path

#修改Config.lua中的配置项
ConfKeys=(device_server_port)

#选择的oem 默认是wayos
oem_choosed=wayos

#打印许可
function printLicense() {
    more <<"EOF"
     
      License Agreement
    .............
EOF
	stty erase '^H'
	read -p "Do you agree to the above license terms? [y/n]:" reply leftpver
	case $reply in
    y* | Y*)
          echo "You have agreed to the license, install start...";;
    n* | N*)
        echo "You don't agreed to the license, you can't install $soft_name."
        exit 1;;
    *)
        echo "Invalid select, exit install ."
        exit 1;;
      esac
}
 
#版本号比较
function VersionComp() {
    if [ "$1" = "$2" ]; then
          echo "eq"
      else
          lrel=`printf "%s\n%s\n" $1 $2 | \
               sort -t . -k 1,1n -k 2,2n -k 3,3n -k 4,4n -k 5,5n | \
                 head -1`
           if [ "$1" = "${lrel}" ]; then
               echo "lt"
           else
               echo "gt"
           fi
      fi
}
 
#软件版本号检测
function VersionCheck() {
    root_dir=`eval echo '$'"$soft_root_macro"`
	if [ -z $root_dir ];then
		echo "You've never installed $server_name before..."
		return
	fi
	
    VersionFile=$root_dir/VERSION
    if [ -e $VersionFile ]; then
        OldVersion=`head -1 $VersionFile | awk '{print $1}' | cut -d"," -f1`
        if [ "gt" = $(VersionComp "$OldVersion" "$version") ]; then
            echo "Newer version ($OldVersion) is already installed, please uninstall first!"
            exit 1
        else
			stty erase '^H'
			read -p "Older version ($OldVersion) is already installed, upgrade to $version? [y/n]:" reply leftover
            case $reply in
            y* | Y*)
				service $server_name stop;;
            n* | N*)
                echo "You canceled to install $soft_name."
                exit 1;;
            *)
                echo "Invalid select, exit install ."
                exit 1;;
            esac
        fi
	else
		echo "Version file : $VersionFile does not exist!"
    fi
}
 
#设置安装路径
function SetInstallPath() {
	stty erase '^H'
    read -p "Install $soft_name to $AppFolder, sure? [y/n]:" reply leftover
    case $reply in
    y* | Y*)
        ;;
    n* | N*)
		stty erase '^H'
        read -p "Please input install path: " AppFolder leftover;;
    *)
        echo "Invalid select, exit install ."
        exit 1;;
    esac
     
    AppFolder=`readlink -f $AppFolder`
    InstallPath=$AppFolder
    mkdir -p $InstallPath
}
 
#文件校验
function CheckSum() {
	if [ ! -x /usr/bin/sum ]; then
		echo "Can't find /usr/bin/sum to do checksum, continuing anyway."
		return 0
    fi
 
    echo "Checksumming..."
    sumret=(`/usr/bin/sum $package_name`)
	echo "sum_v=${sumret[0]}, sum_s=${sumret[1]}"
    if [ ${sumret[0]} != sum_value -o ${sumret[1]} != sum_size ]; then
       	echo "The install file appears to be corrupted."
       	echo "Please do not attempt to install this file."
       	exit 1;
    fi
}

#编译LUA文件
function Compile_lua()
{
	echo -e "Compiling lua scripts..."
	
	current_path=`pwd`
	
	cd $InstallPath/$soft_name/lua_obj
	if [ -e compile_lua.sh ]; then
		chmod +x compile_lua.sh
		./compile_lua.sh
	else
		echo "compile_lua.sh missing !"
	fi
	cd $current_path
	
	echo -e "Compile lua scripts complete !"
}
 
#解包
function Unpack() {
    echo "Unpacking..."
    lines=`awk '/^exit 0/{print NR+1;exit 1;}' "$0"`
	echo "shell lines=$lines"
    tail -n +$lines "$0" > $package_name      2>/dev/null
    
    CheckSum
    
    tar -zvxf $package_name -C $InstallPath/  2>/dev/null
     
    rm $package_name
}
 
#写环境变量
function WriteEnv() {
	date_string=`date +%Y%m%d%H%M%S`
	#备份配置文件
		
	echo "Backup environment file..."
	
	cp $rc_file_path $rc_file_path.bak$date_string
	
	echo "Update environment file..."
	
#删除指定范围的文本
	
	sed -i "/^$flag_env_begin/,/$flag_env_end$/d" $rc_file_path
#	sed -i "/$flag_env_begin/d" $rc_file_path
#	sed -i "/$flag_env_end/d" $rc_file_path
	
#重新设置环境变量
	
    echo "$flag_env_begin
export $soft_root_macro=$InstallPath/$soft_name
export PATH=\$(echo \$PATH | sed 's/:/\n/g' | sort | uniq | tr -s '\n' ':' | sed 's/:\$//g')
export LD_LIBRARY_PATH=\$(echo \$LD_LIBRARY_PATH | sed 's/:/\n/g' | sort | uniq | tr -s '\n' ':' | sed 's/:\$//g')
$flag_env_end" >>$rc_file_path

	source $rc_file_path

#    echo "Please restart, to make environment variable effect."
}

#注册服务
function Regist_Server()
{
	echo "Regist server $server_name..."
	server_script=$InstallPath/$soft_name/$server_name
	
	if [ -e $server_script ]; then
		#将脚本中的变量值替换
		sed -i "s#^SERVER_DIR=.*#SERVER_DIR=$InstallPath/$soft_name#g" $server_script
		sed -i "s#^SERVER_BIN=.*#SERVER_BIN=$project_name#g" $server_script
		chmod +x $server_script
		#拷贝服务脚本到系统目录下，并配置
		cp $server_script /etc/init.d/
		chkconfig --add $server_name
		chkconfig $server_name on;
	else
		echo "Regist server $server_name failed : can't find script->$server_script !"
	fi
}

#检查框架依赖
function Check_Depends()
{
	echo "Checking depends..."
	
	sf=${SF_ROOT}
	
	if [ -z $sf ]; then
		echo "Error: You need to install server_frame first !"
		exit 1;
	fi
	
	#检查依赖的软件是否存在
	for soft in $depend_rpm; do 
		echo -e "Checking soft $soft...\c"; 
		if [ `rpm -qa | grep $soft |wc -l` -ne 0 ];then
			echo -n -e "\r\033[K\033[0m";
			echo -e "\rCheck $soft success!"
		else
			echo -n -e "\r\033[K\033[0m";
			echo -e "\rError: Check $soft failed, you should install it first!"
			exit 1;
		fi
	done
	
	#检查依赖的命令是否存在
	for cmd in $depend_cmd; do
		echo -e "Checking command: $cmd...\c";
		command -v $cmd >/dev/null 2>&1
		if [ $? -eq 0 ];then
			echo -n -e "\r\033[K\033[0m";
			echo -e "Check $cmd success!		";
		else
			echo -n -e "\r\033[K\033[0m";
			echo -e "\rError: Check $cmd failed, you should install it first!";
			exit 1;
		fi
	done
}

#启动服务
function Start_service()
{
	stty erase '^H'
	read -p "Do you want to start $server_name right now? [y/n]:" reply leftover
    case $reply in
    y* | Y*)
        service $server_name restart;;
    n* | N*)
        echo "You can run command : [service $server_name restart] to start service manually";;
    *)
        echo "Invalid select, exit install ."
        exit 1;;
    esac
}

#修改配置
function Modify_config()
{
	config_file=$InstallPath/$soft_name/$project_name.config
	
	echo "Modify config file: $config_file..."
	
	lua_file_path=$InstallPath/$soft_name/lua_obj/lobj.lua
	
	echo "Set lua_file_path = $lua_file_path"
	
	sed -i "s#^lua_file_path=.*#lua_file_path=$lua_file_path#g" $config_file
	
	echo "Modify config file: $config_file complete !"
}

#修改卸载脚本
function Modify_uninstall_script()
{
	local uninstall_file_path=$InstallPath/$soft_name/uninstall.sh
	echo "Modifying uninstall script:[$uninstall_file_path]..."
	if [ ! -e $uninstall_file_path ]; then
		echo "Warning: $uninstall_file_path missing !"
	else
		sed -i "s#^version=.*#version=$version#g" $uninstall_file_path
		sed -i "s#^soft_name=.*#soft_name=$soft_name#g" $uninstall_file_path
		sed -i "s#^server_name=.*#server_name=$server_name#g" $uninstall_file_path
		sed -i "s/^flag_env_begin=.*/flag_env_begin=\"$flag_env_begin\"/g" $uninstall_file_path
		sed -i "s/^flag_env_end=.*/flag_env_end=\"$flag_env_end\"/g" $uninstall_file_path
		sed -i "s#^env_files=.*#env_files='$env_files'#g" $uninstall_file_path
		sed -i "s#^soft_root_macro=.*#soft_root_macro=$soft_root_macro#g" $uninstall_file_path
		
		chmod +x $uninstall_file_path
		
		echo "Modify uninstall script:[$uninstall_file_path] complete !"
	fi
}

#执行解压之后的操作
function After_unpack()
{
	#将可执行脚本的执行权限加上
	
	for scrp in $excutable_files; do 
		echo -e "Add +x to $scrp...\c"
		chmod +x $InstallPath/$soft_name/$scrp
		echo -e "\rAdd +x to $scrp success!"
	done
	
	#将对应OEM的配置文件复制到安装目录下
	local src_file=$InstallPath/$soft_name/oem_configs/$oem_choosed.tmp
	local dst_file=$InstallPath/$soft_name/lua_obj/lualib/wys/config.lua
	echo "Copy $src_file  to .$dst_file"
	cp -f  $src_file $dst_file
	
	SetLuaConfig
}

#选择oem
function Choose_OEM()
{
	local oem_id=1
	local count=${#oem_options[@]};
	echo -e "Please choose one OEM bellow:\n"
	for oem in ${oem_options[*]};do
		echo -e "\t$oem_id:\t$oem"
		let oem_id++
	done
	
	stty erase '^H'
    read -p "Choose OEM(1-$count): " oem_choosed_id leftover
	if [ $oem_choosed_id -gt 0 -a $oem_choosed_id -le $count ]; then
		oem_choosed=${oem_options[$oem_choosed_id-1]}
	else
		echo "Invalid OEM choice, exit!"
		exit 1;
	fi
}

function SetLuaConfig()
{
	#设置monitor.sh配置
	echo "--------Config.lua--------"
	for tkey in ${ConfKeys[*]};do
		SetConfig $tkey  $InstallPath/$soft_name/$project_name""_monitor.sh 0
	done
	
	#设置env.sh配置，
	local envConfKeys=(webSvrPath webSdkRootPath)
	for tkey in ${envConfKeys[*]};do
		SetConfig $tkey $InstallPath/$soft_name/env.sh 0
	done
	
	echo "--------------------------"
}

#根据源端k=v的配置文件，修改目的端配置文件的值
#参数：
#		key:			key值
#		destFilePath:	目的端配置文件路径
#		flag:			是否是带引号的标识
function SetConfig()
{
	local LuaObjPath=$InstallPath/$soft_name/lua_obj
	local Confile=$LuaObjPath/lualib/wys/config.lua
	
	local tkey=$1
	local TargetConfile=$2
	local flag=$3
	local tsed="/^_M.${tkey}=/!d;s/.*=//"
	local tval=$(sed ${tsed} ${Confile})
	tval=${tval// /}
	tval=${tval//\//\\/}
	
	if [ $flag -eq 0 ]
	then
		local csed="-i s/^${tkey}=.*/${tkey}=${tval}/g ${TargetConfile}"
		echo -e "\t${tkey}=${tval}"
	else
		local csed="-i s/^${tkey}=.*/${tkey}=\"${tval}\"/g ${TargetConfile}"
		echo -e "\t${tkey}=\"${tval}\""
	fi
	$($Sed $csed)
}

#清除不必要的文件（夹）
function RemoveTempFiles()
{
	local tmp_files=$InstallPath/$soft_name/oem_configs
	rm -rf $tmp_files
}

Check_Depends
VersionCheck
Choose_OEM
#printLicense
SetInstallPath
Unpack
After_unpack
Modify_uninstall_script
#Compile_lua
Modify_config
RemoveTempFiles
Regist_Server
WriteEnv
Start_service

 
echo "Install complete!"
exit 0
