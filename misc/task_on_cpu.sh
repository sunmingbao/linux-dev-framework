#!/bin/sh

# 
# 本软件为免费、开源软件。
# 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
# 您可以自由使用、传播本软件。
# 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
# =====================
# 作者: 孙明保
# 邮箱: sunmingbao@126.com
# 


if [ ! $# -ge 1 ]; then
    echo  "usage: $0 <cpu_idx>"
    exit 0
fi

cpu_idx=$1
ps -eLo ruser,lwp,pid,ppid,psr,args | awk '{if ($5=="'$cpu_idx'") print $0}'

#提取第一个以pr开头的单词
# cmd | grep -o 'pr[a-z,A-Z,0-9,.,_,-]*' | head -n 1
