
		linux-dev-framework简介
		
======================

    linux-dev-framework是一个Linux程序开发框架，基于C语言和gcc工具链。 
    linux-dev-framework内含一份简单的源码目录结构、一些常用的C例程和makefile编译脚本。
    用户可以以此为基础，快速开始自己的Linux应用程序开发。
    
    使用此框架，在框架中新增程序及库，也无需修改makefile。

    本软件是一款开源、免费软件。
    具体版权说明见COPYING.txt。
    本软件的编译方法见Build.txt。

    最新源码：https://github.com/sunmingbao/linux-dev-framework/archive/master.zip

本软件的目录结构大致如下：

|
|-- makefiles
|
|-- libs
|   |-- sub dirs of each lib
|
|-- simple_apps
|   |-- *.c
|
|-- apps
|   |-- sub dirs of each app
|
|-- misc
|   |-- *


各目录项包含的内容如下：
libs             各个库的源码。一个文件夹对应一个库。
simple_apps      简单c程序（每个程序仅由单个c文件构成）源码。
apps             简单c程序（每个程序可由任意多个c文件构成）源码。一个文件夹对应一个程序。
makefiles 对上述目录中的源码进行编译。

misc     目录中是一些可能有用但比较杂乱的文件，他们不参与上述编译活动。

编译的过程如下：
libs目录下的每个文件夹被编译为一个库。库名就是文件夹名。
simple_apps目录下的每个.c文件被单独编译成一个可执行程序。
apps目录下的每个文件夹中的全部.c文件被编译成一个可执行程序，程序名就是文件夹名。

simple_apps及apps中的程序，均和libs目录中的库进行链接。
======================
作者: 孙明保
邮箱: sunmingbao@126.com
