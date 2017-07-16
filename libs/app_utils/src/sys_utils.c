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
#include <stdlib.h>

#include <pthread.h>
#include <sched.h>

#include "sys_utils.h"
#include "misc_utils.h"
#include "debug.h"

void make_cpu_set(cpu_set_t *set, int cpu_idx_begin, int cpu_idx_end)
{
    int i;
    CPU_ZERO(set);
    for (i=cpu_idx_begin;i<=cpu_idx_end;i++)
    {
        CPU_SET(i, set);
    }

}

int set_thread_cpu_range(pthread_t thread, int cpu_idx_begin, int cpu_idx_end)
{
    cpu_set_t set;
    
    make_cpu_set(&set, cpu_idx_begin, cpu_idx_end);
    return pthread_setaffinity_np(thread, sizeof(set), &set);
}

int bind_cur_thread_to_cpu(int cpu_idx)
{
    return set_thread_cpu_range(pthread_self(), cpu_idx, cpu_idx);
}

int set_cur_thread_cpu_range(int cpu_idx_begin, int cpu_idx_end)
{
    return set_thread_cpu_range(pthread_self(), cpu_idx_begin, cpu_idx_end);
}


int bind_thread_to_cpu(pthread_t thread, int cpu_idx)
{
    return set_thread_cpu_range(thread, cpu_idx, cpu_idx);
}

int create_thread_full(pthread_t *thread, void *fn, void *arg, int cpu_idx_begin, int cpu_idx_end, int policy, int sched_priority)
{
	pthread_attr_t attr;
	cpu_set_t set;
	struct sched_param param = {.sched_priority = sched_priority};
	int s;
	
	s = pthread_attr_init(&attr);
	if (s)
	{
		errno=s;
		ERR_DBG_PRINT("==");
		return -1;
	}

#if 0
	if (stack_size > 0) /* when stack_size == 1024*1024*2 succeed, when  stack_size == 8192 failed */
	{
	
	    void *sp;
	    s = posix_memalign(&sp, sysconf(_SC_PAGESIZE), stack_size);
		if (s)
		{
		    errno=s;
		    ERR_DBG_PRINT_QUIT("==");
			return s;
		}
		
		s = pthread_attr_setstack(&attr, sp, stack_size);
		if (s)
		{
		    errno=s;
		    ERR_DBG_PRINT_QUIT("==");
			return s;
		}


	}
#endif

	if ((cpu_idx_begin>=0) && (cpu_idx_end>=0))
	{
		make_cpu_set(&set, cpu_idx_begin, cpu_idx_end);
		s = pthread_attr_setaffinity_np(&attr, sizeof(set), &set);
		if (s)
		{
			errno=s;
			ERR_DBG_PRINT("==");
			return -1;
		}

	}
	
	s = pthread_attr_setschedpolicy(&attr, policy);// SCHED_FIFO,  SCHED_RR,  or  SCHED_OTHER
	if (s)
	{
		errno=s;
		ERR_DBG_PRINT("==");
		return -1;
	}


    if ((policy != SCHED_FIFO) && (policy != SCHED_RR))
		param.sched_priority = 0;
	
	s = pthread_attr_setschedparam(&attr,  &param);
	if (s)
	{
		errno=s;
		ERR_DBG_PRINT("==");
		return -1;
	}


    s= pthread_create(thread, &attr, fn, arg);
	pthread_attr_destroy(&attr);
	if (s)
	{
		errno=s;
		ERR_DBG_PRINT("==");
		return -1;
	}

	return 0;

}

uint64_t get_cpu_freq()
{
    uint64_t t1, t2;
    t1 = rdtsc();
    nano_sleep(1,0);
    t2 = rdtsc();
    return ((t2-t1)/1000000)*1000000;
}

