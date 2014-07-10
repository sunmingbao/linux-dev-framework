/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <stdio.h>
#include "trace_exception.h"

int a;
int *pa=(void *)0xfa;


int gen_SIGSEGV()
{
    a = *pa;
    return 5;
}

int SIGSEGV_test()
{
    int ret = 8;

    return gen_SIGSEGV()+ret;
}

int main(int argc, char *argv[])
{
    trace_exception_init();

    return SIGSEGV_test();
}

