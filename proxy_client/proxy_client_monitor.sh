#!/bin/bash  
. /etc/init.d/functions  
number=1
maxcount=10
connect_timeout=3
mutaul_server_port=10003
#获取当前脚本所在路径
function get_cur_script_location()
{
	pwd_dir=`pwd`
	location_dir=$(cd `dirname $0`; pwd)
	cd  $pwd_dir
	
	echo "$location_dir"
}

C_PATH=$(get_cur_script_location)

while [ 1 ]
do
	num=`echo -n "\n"|nc -w $connect_timeout -z 127.0.0.1 $mutaul_server_port|grep succeeded|wc -l`  
	if [ $num -eq 1 ]
	then
		number=1;
		action "network is open." /bin/true
		sleep 10
	else
		action "network is closed." /bin/false
		sleep 1
		let number++;
		if [ $number -ge $maxcount ]
		then
			number=1;
			mkdir -p $C_PATH/log
			date +'%Y-%m-%d %H:%M:%S' >> $C_PATH/log/crash_log.txt
			echo 'C++ Server no response, restart!!!' >> $C_PATH/log/crash_log.txt
			service iptv_clientd restart
		fi
	fi
done

