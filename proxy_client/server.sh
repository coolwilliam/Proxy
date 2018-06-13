#!/bin/bash

. ${PWD}/env.sh

proc_name=iptv_client

killall .$proc_name

echo restart $proc_name....

./.$proc_name 2>&1 &
#./$proc_name
