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
#include <pthread.h>
#include <sched.h>
#include "sys_utils.h"
#include "misc_utils.h"

int set_thread_cpu_range(pthread_t thread, int cpu_idx_begin, int cpu_idx_end)
{
    int i;
    cpu_set_t set;
    
    CPU_ZERO(&set);
    for (i=cpu_idx_begin;i<=cpu_idx_end;i++)
    {
        CPU_SET(i, &set);
    }
    

   return pthread_setaffinity_np(thread, sizeof(set), &set);
}

int set_cur_thread_cpu_range(int cpu_idx_begin, int cpu_idx_end)
{
   return set_thread_cpu_range(pthread_self(), cpu_idx_begin, cpu_idx_end);
}

int bind_cur_thread_to_cpu(int cpu_idx)
{
    return set_cur_thread_cpu_range(cpu_idx, cpu_idx);
}

uint64_t get_cpu_freq()
{
    uint64_t t1, t2;
    t1 = rdtsc();
    nano_sleep(1,0);
    t2 = rdtsc();
    return ((t2-t1)/1000000)*1000000;
}

