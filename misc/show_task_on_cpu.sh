#!/bin/sh
cpu_idx=0
if [ $# -gt 0 ]; then
    cpu_idx=$1
fi

ps -eLo ppid,pid,tid,psr,args | head -n 1
#ps -eLo ppid,pid,tid,psr,args | awk '{if ($(NF-1)=="'"$cpu_idx"'") print $0}'
ps -eLo ppid,pid,tid,psr,args | awk '{if ($4=="'"$cpu_idx"'") print $0}'
