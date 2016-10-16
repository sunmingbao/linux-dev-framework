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
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <inttypes.h>
#include <pthread.h>
#include <string.h>
#include "measure.h"
#include "misc_utils.h"
#include "debug.h"

t_measure  **g_pt_measures;
static unsigned  cur_measures_cnt, max_measures_cnt;
static pthread_mutex_t measures_lock = PTHREAD_MUTEX_INITIALIZER;

static uint64_t cpu_freq;

int init_measure(int max_cnt)
{
    g_pt_measures = malloc(sizeof(t_measure)*max_cnt);
    max_measures_cnt = max_cnt;
    cpu_freq=get_cpu_freq();
    return NULL==g_pt_measures;
}

void add_measure(void *pt_measure)
{
    pthread_mutex_lock(&measures_lock);
    g_pt_measures[cur_measures_cnt] = pt_measure;
    cur_measures_cnt++;
    pthread_mutex_unlock(&measures_lock);
}

void report_measure()
{
    int i;
    t_measure  *pt_measure;
    printf("cpu freq = %"PRIu64"\n", cpu_freq);
    for (i=0; i<cur_measures_cnt; i++)
    {
        pt_measure = g_pt_measures[i];
        printf("%s "
            "exec %"PRIu64" times "
            "%"PRIu64" cycles in total "
            "%"PRIu64" cycles per time\n"
            ,pt_measure->name
            ,pt_measure->total_cnt
            ,pt_measure->total_cycles
            ,pt_measure->total_cnt>0?(pt_measure->total_cycles/pt_measure->total_cnt):0);

    }

}

