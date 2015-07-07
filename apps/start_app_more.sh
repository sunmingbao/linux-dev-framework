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


partition_disk () {

    fdisk $1 << EOF
n
p
1


w
EOF
    sync
}

del_partition () {
    fdisk $1 << EOF
d
$2
w
EOF
    sync
}

del_all_partitions () {
   echo $1
   del_partition  $1  4
   del_partition  $1  3
   del_partition  $1  2
   del_partition  $1  1
}

mount_and_prepare_files () {
   mount -t ext3 -o data=writeback,commit=3,noatime,nodiratime $1  /path/to/app/cfg/dir
   cp -rf /etc/app_cfg_dir/cfg_file /path/to/app/cfg/dir/
   if [ ! -f /path/to/app/cfg/dir/cfg_file ]; then
      echo mount $1 failed
      return 1
   fi
   
   touch /path/to/app/cfg/dir/savable_cfg_file
   return 0
}

prepare_fs_sata() {
    umount /dev/sda1
    if [ ! -r /dev/sda ]; then
       echo disk /dev/sda does not exist.
       return 1
    fi
   
    for i in 4 3 2 
    do
        tmp_var="/dev/sda$i"
        echo $tmp_var
    
       if [ -r $tmp_var ]; then
           echo make ext3 fs on /dev/sda1
           del_all_partitions  /dev/sda
           partition_disk      /dev/sda
           mke2fs -j -b 4096 /dev/sda1
           sync
           break
       fi
    
    done

    if [ ! -r /dev/sda1 ]; then
        echo make ext3 fs on /dev/sda1  failed
        return 1
    fi

    e2fsck -p  /dev/sda1
    if [ $? -ne 0 ]; then
        echo e2fsck /dev/sda1 failed.
        return 1
    fi
    
    mount_and_prepare_files  /dev/sda1
    return $?

}

prepare_fs () {

    mount -t auto  /dev/sda1  /path/to/app/cfg/dir
    if [ -f /path/to/app/cfg/dir/savable_cfg_file ]; then
       mount --bind  /path/to/app/cfg/dir /etc/app_cfg_dir
       touch /tmp/app_tmp_dir/fs_good.txt
       return 0
    fi


    prepare_fs_sata
    if [ $? -ne 0 ]; then
        return 1
    fi

    mount --bind  /path/to/app/cfg/dir /etc/app_cfg_dir
    touch /tmp/app_tmp_dir/fs_good.txt
    return 0
}

launch_apps () {

    query_cnt=`ps -ef | grep app_name.exe | grep -v grep | wc -l`
    if [ $? -eq 0 ]; then
        #prepare configuration if needed
        ./app_name.exe
    fi
}


#set CWD to the dir which contains the script file
our_dir=`dirname $0`
cd $our_dir

#do any prepare work needed before app launch
mkdir /tmp/app_tmp_dir

prepare_fs
if [ $? -ne 0 ]; then
    echo  prepare fs failed.
    exit 1
fi


launch_apps