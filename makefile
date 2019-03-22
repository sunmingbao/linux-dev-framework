# 
# 本软件为免费、开源软件。
# 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
# 您可以自由使用、传播本软件。
# 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
# =====================
# 作者: 孙明保
# 邮箱: sunmingbao@126.com
# 

#for cross compile, you can set CROSS_COMPILE through command line.
#e.g. CROSS_COMPILE=arm-linux-
CROSS_COMPILE?=
COMPILE_TYPE?=release
USE_OPEN_SSL?=false

#generate toolchain and command strings
export CC:=$(CROSS_COMPILE)gcc
export LD:=$(CROSS_COMPILE)ld
export AR:=$(CROSS_COMPILE)ar
export RANLIB:=$(CROSS_COMPILE)ranlib
export RM:=rm -rf
export MKDIR:=mkdir -p

#src dirs
export project_root:=$(shell pwd)
export lib_root:=$(project_root)/libs
export app_root:=$(project_root)/apps
export simple_app_root:=$(project_root)/simple_apps

#compile output dirs
export project_target_root:=$(project_root)/target
export lib_target_root:=$(project_target_root)/libs
export apps_target_root:=$(project_target_root)/apps
export simple_apps_target_root:=$(project_target_root)/simple_apps

#create output dirs
__dummy:=$(shell  $(MKDIR) $(lib_target_root))
__dummy:=$(shell  $(MKDIR) $(apps_target_root))
__dummy:=$(shell  $(MKDIR) $(simple_apps_target_root))


#generate lib name and lib file list
export lib_list:=$(shell  ls $(lib_root))
export lib_file_list:=$(addprefix $(lib_target_root)/lib, $(addsuffix .a, $(lib_list)))
export lib_file_dynamic_list:=$(addprefix $(lib_target_root)/lib, $(addsuffix .so, $(lib_list)))

#generate apps list
export app_list:=$(shell ls $(app_root))

#generate compile/link flags, -Ixxxs, -lxxxs
#submakefile can change CFLAGS/INC_DIRS/LDFLAGS/LIBS according to lib/app name
COMMON_CFLAGS:=-Wall -g -fno-strict-overflow -fno-strict-aliasing -fno-omit-frame-pointer -pthread
COMMON_C_LIBS:=-ldl -pthread -lrt -lutil

ifeq ($(COMPILE_TYPE),debug)
    COMMON_CFLAGS+=-D_DEBUG
else
    COMMON_CFLAGS+=-O2
endif

ifeq ($(USE_OPEN_SSL),true)
    COMMON_CFLAGS+=-D_USE_OPENSSL
    COMMON_C_LIBS+=-lcrypto
endif

export COMMON_CFLAGS
export COMMON_C_LIBS
export COMMON_LDFLAGS:=-rdynamic

export INC_ALL_PRJ_LIB_HDR_DIR:=$(addprefix -I$(lib_root)/, $(addsuffix /api, $(lib_list)))
export LINK_ALL_PRJ_LIB:=-L$(lib_target_root) -Wl,--whole-archive $(addprefix -l, $(lib_list)) -Wl,--no-whole-archive


export CFLAGS:=$(COMMON_CFLAGS)
export INC_DIRS := -I. 
export LDFLAGS:=$(COMMON_LDFLAGS)
export LIBS:=$(COMMON_C_LIBS) $(LINK_ALL_PRJ_LIB)

#finally targets list
.PHONY:default  clean

default:
	make -f make_libs  $(lib_list)
	make -f make_simple_apps
	make -f make_apps  $(app_list)
	@echo build project done!

clean:
	$(RM)  $(project_target_root)
	@echo clean project done!

