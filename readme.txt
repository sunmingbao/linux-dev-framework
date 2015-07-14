
		linux-dev-framework简介
		
======================

    linux-dev-framework是一个Linux程序开发框架，基于C语言和gcc工具链。 
    linux-dev-framework内含一份简单的源码目录结构、一些常用的C例程和makefile编译脚本。
    用户可以以此为基础，快速开始自己的Linux应用程序开发。

    本软件是一款开源、免费软件。
    具体版权说明见COPYING.txt。
    本软件的编译方法见Build.txt。

    最新源码：http://sourceforge.net/p/linux-dev-framework/code/ci/master/tree/

本软件的目录结构大致如下：

|
|-- makefile
|
|-- inc
|   |-- *.h
|
|-- src
|   |-- *.c
|
|-- apps
|   |-- *.c
|
|-- misc
|   |-- *

各目录项的用途如下：
inc      目录中存放.h文件。
src      目录中存放不带main函数的.c文件。
apps     目录中存放带有main函数的.c文件。
makefile 对上述目录中的源码进行编译。

misc     目录中是一些可能有用但比较杂乱的文件，他们不参与上述编译活动。

编译的过程如下：
src目录下的所有.c文件被编译合并成一个库，库名为app_lib。
apps目录下的每一个.c文件分别被编译链接为一个可执行程序。先编译后，然后与app_lib进行链接。

======================
作者: 孙明保
邮箱: sunmingbao@126.com
