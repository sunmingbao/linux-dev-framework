/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
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
my_func(my_var, "haha", 1);
    telnetd4dbg_init(10000);

    while (1)
    {
        sleep(5);
    }

    return 0;
}

