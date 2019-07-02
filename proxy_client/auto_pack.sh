#!/bin/bash
#PATH=/usr/bin:/bin

date_string=`date +%Y%m%d%H%M%S`

bug_stats=(alpha beta release)

extern_states=(patch)

bug_stat=alpha
extern_state=""

#工程名称
project_name=iptv_client

#检查输入的bug状态是否在规定的数组内
#---start-----------------------
i_bug=0
b_find=0
b_extern=0
i_extern=0

input_bug=unknown
input_extern=unknown

if [ ! -z $1 ]; then
	input_bug=$1
fi

for i_bug in ${bug_stats[*]}
do
	if [[ $input_bug == $i_bug ]]; then
		bug_stat=$i_bug
		b_find=1
		break
	fi
done
if [ $b_find -eq 0 ]; then
	echo "Invalid bug stat $1 !"
	echo "You should input bug state as follows:"
	for dsp_bug in ${bug_stats[*]}
	do
		echo -e "\t$dsp_bug"
	done

	exit 1
fi

if [ ! -z $2 ]; then
	input_extern=$2
	for i_extern in ${extern_states[*]}
	do
		if [[ $input_extern == $i_extern ]]; then
			extern_state=$i_extern
			b_extern=1
			break;
		fi
	done
	
	if [ $b_extern -eq 0 ]; then
		echo "Invalid extern state $2 !"
		echo "You should input extern state as follows:"
		for dsp_extern in ${extern_states[*]}
		do
			echo -e "\t$dsp_extern"
		done

		exit 1
	fi
fi


#---end-------------------------

main_incre=20
sub_incre=1000
svn_version=`svnversion -c |sed 's/^.*://' |sed 's/[A-Z]*$//'`

fix_version=$(($svn_version%$sub_incre))
sub_version=$((($(($svn_version/$sub_incre))+1)%$main_incre))
main_version=$((($(($svn_version/$sub_incre))+1)/$main_incre))

version=$main_version.$sub_version.$fix_version

#定义软件名
soft_name=$project_name
#soft_name+=$date_string
soft_name+=-
soft_name+=$bug_stat
soft_name+=-
soft_name+=$version

package_name=$soft_name.tar.gz

bin_name=""
if [ $b_extern -eq 1 ]; then
	version=$version"-"$extern_state"-"$date_string
	bin_name=$soft_name"-"$extern_state"-"$date_string.bin
else
	version=$main_version.$sub_version.$fix_version
	bin_name=$soft_name.bin
fi

#服务名称
server_name=$project_name""d

#指定需要安装的文件
pack_files=" "
pack_files+=" "$project_name.config
pack_files+=" "urconfig.properties
pack_files+=" "lua_obj
pack_files+=" "plugins.xml
pack_files+=" "plugins
pack_files+=" "oem_configs
pack_files+=" "iptables_list.json

pack_dest_dir=`pwd`/release/$soft_name

bin_path=$pack_dest_dir/../$bin_name
package_path=$pack_dest_dir/../$package_name
temp_bin_path=$pack_dest_dir/../temp.bin

oem_options=()

#可执行文件
excutable_files=" "
excutable_files+=" "$server_name
excutable_files+=" "$project_name
excutable_files+=" "server.sh
excutable_files+=" ".$project_name
excutable_files+=" "uninstall.sh
excutable_files+=" "env.sh
#excutable_files+=" "print_cmd.sh
#excutable_files+=" "print_cmd.lua
excutable_files+=" "$project_name""_monitor.sh

pack_files+=$excutable_files

#拷贝源码目录构建安装目录


#删除多余的文件和目录


#将安装文件打成tar包
function tar_package()
{
	echo "Start tar package:$package_name...."

	echo "Create version file...."
	#生成VERSION文件
	echo "$version">./VERSION
	
	#将VERSION文件加入打包文件列表中
	pack_files+=" "VERSION

	echo "making dir $pack_dest_dir"
	mkdir -p $pack_dest_dir

	echo "copy files to $pack_dest_dir"

	cp -rf $pack_files $pack_dest_dir/

	echo "tar package $package_name"

	c_path=`pwd`

	cd $pack_dest_dir/../

	tar -zcvf $package_name $soft_name

	cd $c_path

	echo "tar package:$package_name complete!"
}


#计算文件校验值并写到安装脚本中
function add_sum()
{
	echo "Add sum to $package_path..."

	soft_sum=(`/usr/bin/sum $package_path`)
	sum_value=${soft_sum[0]}
	sum_size=${soft_sum[1]}
	echo "sum_value=$sum_value, sum_size=$sum_size"
	
	cat install.sh | sed -e s/sum_value/$sum_value/ \
	-e s/sum_size/$sum_size/ \
	-e "s#^bug_stat=.*#bug_stat=$bug_stat#g" \
	-e "s#^version:=.*#version=$version#g" \
	-e "s#^soft_name:=.*#soft_name=$soft_name#g" \
	-e "s#^package_name:=.*#package_name=$package_name#g" \
	-e "s#^server_name:=.*#server_name=$server_name#g" \
	-e "s#^project_name=.*#project_name=$project_name#g" \
	-e "s#^oem_options=.*#oem_options=(${oem_options[*]})#g" \
	-e "s#^extern_state=.*#extern_state='$extern_state'#g" \
	-e "s#^excutable_files:=.*#excutable_files='$excutable_files'#g" \
	-e "s#^date_string=.*#date_string=$date_string#g" \
	> $temp_bin_path
}


#将安装脚本和目标文件一起打成bin文件
function make_bin()
{
	echo "Start make bin: $bin_path..."

	cat $temp_bin_path $package_path > $bin_path
	chmod a+x $bin_path
	
	echo "Make bin: $bin_path complete!"
}

#清理临时文件
function clean_temps()
{
	echo "Clean temp files..."
	rm -rf $temp_bin_path $package_path $pack_dest_dir
}

#获取oem配置项
function check_oem_options
{
	#根据oem_config下面的文件获取
	local oem_config_tmps=`ls ./oem_configs`
	for oem_file in $oem_config_tmps; do
		local file_name=${oem_file%.*}
		local extension=${oem_file##*.}
		if [ $extension == "tmp" ]; then
			oem_options=(${oem_options[@]} $file_name)
		fi
	done
}

check_oem_options
tar_package
add_sum
make_bin
clean_temps



