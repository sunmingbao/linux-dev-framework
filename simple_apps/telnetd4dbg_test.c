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
 * 本程序实现了查看进程中的全局变量，以及执行进程中的调试函数的功能。
 * 程序运行后，用户通过telnet ip 10000命令即可登陆到此程序的shell
 * 用户在shell中输入变量名或函数调用命令，即可看到相应的输出结果。
 * 例如，
 * 输入 my_var，即可查看变量my_var的信息。
 * 输入 my_func(1, "good", 0x123) 即可使用输入的参数执行函数my_func
 * 目前最大支持8个参数，且每个参数size必须等于sizeof(long)
 * my_func函数的各个入参就是一个符合要求的例子。
 *
 * 输入 d(mem_addr, len) 可以查看内存的内容
 * 例如，
 * 输入 d(0x12345678, 32) 即可查看内存地址0x12345678处的32个字节的内容。
 * 
 */

#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include "debug.h"
#include "telnetd4dbg.h"

void hello()
{
    DBG_PRINT("777");
}

int my_var = 1234;
int my_func(long para1, char *para2, unsigned long para3)
{
    printf("para1=%ld para2=%p para3=%lu\n", para1, para2, para3);
    printf("para2 contents = %s", para2);
    return 0;
}

int main(int argc, char *argv[])
{
    telnetd4dbg_init(10000);

    while (1)
    {
        sleep(5);
    }

    return 0;
}

