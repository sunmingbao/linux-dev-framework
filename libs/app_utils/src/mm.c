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
#include <pthread.h>
#include <stdlib.h>
#include "list.h"
#include "mm.h"

typedef struct
{
    uint32_t buffer_num_total;
    uint32_t buffer_num_free;
    
    struct   list_head free_pool;
    struct   list_head all;
    pthread_mutex_t the_mutex_free_pool;
    pthread_mutex_t the_mutex_all;

    void    *buffer_mem_start;
    uint32_t buffer_mem_len;
} t_mm_manager;

static inline void lock_free_pool(t_mm_manager  *pt_manager)
{
    pthread_mutex_lock(&(pt_manager->the_mutex_free_pool));
}

static inline void unlock_free_pool(t_mm_manager  *pt_manager)
{
    pthread_mutex_unlock(&(pt_manager->the_mutex_free_pool));
}

static inline void lock_all_link(t_mm_manager  *pt_manager)
{
    pthread_mutex_lock(&(pt_manager->the_mutex_all));
}

static inline void unlock_all_link(t_mm_manager  *pt_manager)
{
    pthread_mutex_unlock(&(pt_manager->the_mutex_all));
}

uint32_t free_buffer_num(MM_HANDLE hd)
{
    t_mm_manager  *pt_manager = hd;
    return pt_manager->buffer_num_free;
}

typedef struct
{
    struct list_head link_node_free_alloc;
    struct list_head link_node_all;
    char   buffer[0];
} t_buffer;

int  MM_HANDLE_IS_VALID(MM_HANDLE hd)
{
    t_mm_manager  *pt_manager = hd;
    return (NULL!=pt_manager);
}

void free_only(t_mm_manager  *pt_manager, void *buf)
{
    t_buffer *pt_buffer=buf;
    list_add_tail(&(pt_buffer->link_node_free_alloc), &(pt_manager->free_pool));
    pt_manager->buffer_num_free++;
}

void free_buffer(MM_HANDLE hd, void *buf)
{
    t_mm_manager  *pt_manager = hd;
    t_buffer *pt_buffer = buf-sizeof(t_buffer);
    lock_free_pool(pt_manager);
    free_only(pt_manager, pt_buffer);
    unlock_free_pool(pt_manager);
}

void *alloc_buffer(MM_HANDLE hd)
{
    t_mm_manager  *pt_manager = hd;
    t_buffer *pt_buffer;
    void *ret = NULL;
    struct list_head *pt_list=pt_manager->free_pool.next;

    lock_free_pool(pt_manager);
    if (0==pt_manager->buffer_num_free) goto EXIT;

    list_del(pt_list);
    pt_manager->buffer_num_free--;
    pt_buffer=list_entry(pt_list, t_buffer, link_node_free_alloc);
    ret = pt_buffer->buffer;

EXIT:
    unlock_free_pool(pt_manager);
    return ret;
}

void add_to_all_link(MM_HANDLE hd, void *buf)
{
    t_mm_manager  *pt_manager = hd;
    t_buffer *pt_buffer = buf-sizeof(t_buffer);
    lock_all_link(pt_manager);
    list_add_tail(&(pt_buffer->link_node_all), &(pt_manager->all));
    unlock_all_link(pt_manager);
}

void delete_from_all_link(MM_HANDLE hd, void *buf)
{
    t_mm_manager  *pt_manager = hd;
    t_buffer *pt_buffer = buf-sizeof(t_buffer);
    lock_all_link(pt_manager);
    list_del(&(pt_buffer->link_node_all));
    unlock_all_link(pt_manager);
}

void for_each_buf_in_all_link(MM_HANDLE hd, p_func usr_func)
{
    t_mm_manager  *pt_manager = hd;
    t_buffer *pt_buffer;
    struct list_head *pos, *head = &(pt_manager->all);
    
    list_for_each(pos, head)
    {
        pt_buffer = list_entry(pos, t_buffer, link_node_all);
        if (usr_func(pt_buffer->buffer))
        {
            break;
        }

    }

}

MM_HANDLE create_buffer_manager(int buf_size, int num)
{
    int i;
    t_mm_manager  *pt_manager=malloc(sizeof(t_mm_manager));
    t_buffer *pt_buffer;
    int actual_buf_size=sizeof(t_buffer)+buf_size;
    int total_mem_len=actual_buf_size*num;
    void *all_mem=malloc(total_mem_len);

    if (NULL==all_mem || NULL==pt_manager)
    {
        goto FAIL_EXIT;
    }

    pt_manager->buffer_num_total=num;
    pt_manager->buffer_mem_start=all_mem;
    pt_manager->buffer_mem_len=total_mem_len;
    INIT_LIST_HEAD(&(pt_manager->free_pool));
    INIT_LIST_HEAD(&(pt_manager->all));
    pthread_mutex_init(&(pt_manager->the_mutex_free_pool), NULL);
    pthread_mutex_init(&(pt_manager->the_mutex_all), NULL);
    
    for (i=0; i<num; i++)
    {
        pt_buffer=all_mem+i*actual_buf_size;
        INIT_LIST_HEAD(&(pt_buffer->link_node_all));
        free_only(pt_manager, pt_buffer);
    }
    
    return pt_manager;

FAIL_EXIT:
    if (pt_manager) free(pt_manager);
    if (all_mem)    free(all_mem);
    return NULL;
}

