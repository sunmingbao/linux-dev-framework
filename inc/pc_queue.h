/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */


/* 生产者-消费者 队列*/
#ifndef  __PC_QUEUE_H__
#define __PC_QUEUE_H__

#include <stdint.h>


typedef struct
{
void *                for_align;
volatile    uint32_t  valid;
volatile    int32_t   para_int;
    void    *volatile para;
    void    *volatile data;
} t_pc_que_entry;

typedef struct
{
void *                for_align;
    uint32_t          nr_entry;
    t_pc_que_entry    *pt_head;
    t_pc_que_entry    *pt_tail;
    t_pc_que_entry    *pt_p_ptr;
    t_pc_que_entry    *pt_c_ptr;
    t_pc_que_entry    p_at_entries[0];

} t_pc_que;

void* create_pc_que(uint32_t size);

void* pc_que_outq_try(t_pc_que *pt_q, int32_t   *para_int, void **para);
int   pc_que_enq_try(t_pc_que *pt_q, void *data, int32_t   para_int, void *para);
#endif

