/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __SYS_UTILS_H__
#define  __SYS_UTILS_H__
#include <stdint.h>
#include <pthread.h>

#if defined( __i386) || defined( __x86_64)
static inline uint64_t rdtsc()
{
        uint32_t lo,hi;

        __asm__ __volatile__
        (
         "rdtsc":"=a"(lo),"=d"(hi)
        );
        return (uint64_t)hi<<32|lo;
}
#else
static inline uint64_t rdtsc()
{
        return 0;
}
#endif

int set_thread_cpu_range(pthread_t thread, int cpu_idx_begin, int cpu_idx_end);
int bind_cur_thread_to_cpu(int cpu_idx);
int set_cur_thread_cpu_range(int cpu_idx_begin, int cpu_idx_end);
int bind_thread_to_cpu(pthread_t thread, int cpu_idx);

int create_thread_full(pthread_t *thread, void *fn, void *arg, int cpu_idx_begin, int cpu_idx_end, int policy, int sched_priority);

uint64_t get_cpu_freq();
#endif

