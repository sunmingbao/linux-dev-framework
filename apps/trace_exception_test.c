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
#include "debug.h"
#include "trace_exception.h"

//下面的代码故意产生一个内存访问异常
static int a;
int __attribute__((noinline)) gen_SIGSEGV(int *bad_pointer)
{
    unsigned long *bp;

    asm ("movl %%ebp, %0":"=qm"(bp));
    DBG_PRINT("bp=%p", bp);

    a = *bad_pointer;
    return a;
}

int __attribute__((noinline)) SIGSEGV_test(int para)
{
        unsigned long *bp;

    asm ("movl %%ebp, %0":"=qm"(bp));
    DBG_PRINT("bp=%p", bp);

    return gen_SIGSEGV(NULL)+para;
}

int __attribute__((noinline)) main(int argc, char *argv[])
{
    unsigned long *bp;

    asm ("movl %%ebp, %0":"=qm"(bp));
    DBG_PRINT("bp=%p", bp);

    trace_exception_init();
    SIGSEGV_test(555);
    return 0;
}



