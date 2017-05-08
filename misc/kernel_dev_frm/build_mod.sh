#!/bin/sh

target=modules
RETVAL=0

if [ $# -gt 0 ]; then
    target=$1
fi

case "$target" in
  modules|modules_install|clean)
        make  -C  /lib/modules/`uname -r`/build/  SUBDIRS=`pwd` $target
        RETVAL=$?
        ;;
  *)
        echo "Usage: $0 <modules|modules_install|clean>"
        RETVAL=2
esac

exit $RETVAL
