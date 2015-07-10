/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

/* 
 * 本程序实现了查看进程中的全局变量值，以及执行进程中的调试函数的功能。
 * 目前功能还不完善。
 */

#include <stdio.h>
#include <errno.h>


#include "symbol_utils.h"


int my_var = 1234;
int my_func(int para1, int para2)
{
    printf("para1=%d, para2=%d\n", para1, para2);
    return 0;
}


int main(int argc, char *argv[])
{
    init_symbol();
    var_value("my_var");
    exec_func("my_func(20, 15)");


    return 0;
}

