#!/bin/bash

. ${PWD}/env.sh

proc_name=iptv_server
monitor=$proc_name""_monitor.sh

killall $monitor
killall .$proc_name

echo restart $proc_name....

./.$proc_name 2>&1 &
sleep 180
./$monitor 2>&1 &
#./$proc_name
