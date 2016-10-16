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
#include "pc_queue.h"
#include "debug.h"

void* create_pc_que(uint32_t size)
{
    int total_size = sizeof(t_pc_que) + size*sizeof(t_pc_que_entry);
    t_pc_que *pt_que = malloc(total_size);

    if (NULL==pt_que) goto EXIT;
    
    memset(pt_que, 0, total_size);
    pt_que->nr_entry = size;
    pt_que->pt_head= &(pt_que->p_at_entries[0]);
    pt_que->pt_tail= &(pt_que->p_at_entries[size-1]);
    pt_que->pt_p_ptr= pt_que->pt_head;
    pt_que->pt_c_ptr= pt_que->pt_head;

    EXIT:
        return pt_que;
}

void* pc_que_outq_try(t_pc_que *pt_q, int32_t   *para_int, void **para)
{
    void *ret = NULL;
    
    if (!(pt_q->pt_c_ptr->valid))
        goto EXIT;

    ret = pt_q->pt_c_ptr->data;
    if (para_int)
        *para_int = pt_q->pt_c_ptr->para_int;
    if (para)
        *para = pt_q->pt_c_ptr->para;
    
    pt_q->pt_c_ptr->valid = 0;
    if (pt_q->pt_c_ptr==pt_q->pt_tail)
        pt_q->pt_c_ptr = pt_q->pt_head;
    else
        pt_q->pt_c_ptr++;

EXIT:
    return ret;
    
}

int   pc_que_enq_try(t_pc_que *pt_q, void *data, int32_t   para_int, void *para)
{
    t_pc_que_entry *pt_entry = NULL;
    
    if (pt_q->pt_p_ptr->valid)
        return 1;

    pt_entry = pt_q->pt_p_ptr;

    pt_entry->data = data;
    pt_entry->para_int = para_int;
    pt_entry->para = para;
    pt_entry->valid = 1;

    if (pt_q->pt_p_ptr==pt_q->pt_tail)
        pt_q->pt_p_ptr = pt_q->pt_head;
    else
        pt_q->pt_p_ptr++;

    return 0;
    
}