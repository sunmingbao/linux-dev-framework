/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */


#define _GNU_SOURCE
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>

#include "signal_utils.h"
#include "timer_utils.h"
#include "debug.h"

int create_posix_timer(timer_t *timerid, void *timer_handler, int sec, int nano_sec)
{
    struct sigevent  evp;
    struct itimerspec its;

    
    evp.sigev_notify = SIGEV_THREAD_ID;
    evp.sigev_signo = get_a_free_sig_and_register_proc(timer_handler);
    DBG_PRINT("evp.sigev_signo=%d", (int)evp.sigev_signo);
    evp.sigev_value.sival_ptr = timerid;
    evp._sigev_un._tid = syscall(SYS_gettid);
    DBG_PRINT("evp._sigev_un._tid=%d", (int)evp._sigev_un._tid);
    if (timer_create(CLOCK_REALTIME, &evp, timerid))
        {
            return -1;

        }

   its.it_value.tv_sec = sec;
   its.it_value.tv_nsec = nano_sec;
   its.it_interval.tv_sec = its.it_value.tv_sec;
   its.it_interval.tv_nsec = its.it_value.tv_nsec;

   if (timer_settime(*timerid, 0, &its, NULL))
    {
            return -1;
    }

   return 0;
}


#include <sys/time.h>
void itimer_init(int s, int us)
{
    struct itimerval timer;

    timer.it_value.tv_sec = s;
    timer.it_value.tv_usec = us;

    timer.it_interval.tv_sec = s;
    timer.it_interval.tv_usec = us;

    if (setitimer(ITIMER_REAL, &timer, NULL))
        DBG_PRINT("setitimer failed");

}

