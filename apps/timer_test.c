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
#include "timer_utils.h"
#include "debug.h"
static void    hehe(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{
    DBG_PRINT("==");
    
}

int main(int argc, char *argv[])
{
    timer_t  timerid;
    create_posix_timer(&timerid, hehe, 1, 0);

    while (1) sleep(5);
    return 0;
}

