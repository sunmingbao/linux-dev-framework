/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef __TAB_XXX_H_
#define __TAB_XXX_H_

#include <stdint.h>
#include <pthread.h>
#include "list.h"

typedef struct
{
    struct list_head  all;
	struct list_head  by_id;
	struct list_head  by_key;

	uint32_t ref_cnt;
    pthread_spinlock_t lock;

    uint16_t id;
	uint32_t a;
	uint32_t b;
	uint32_t c;
	uint32_t priv_data;
} t_tab_xxx_entry;

int tab_xxx_init(int max_entry_num);

int tab_xxx_add_entry(uint32_t a, uint32_t b, uint32_t c, uint32_t priv_data);
int tab_xxx_del_entry_by_key(uint32_t a, uint32_t b, uint32_t c);
void * tab_xxx_get_entry_by_key(uint32_t a, uint32_t b, uint32_t c);
void * tab_xxx_get_entry_by_id(uint16_t id);
void   tab_xxx_put(t_tab_xxx_entry *pt_entry);
uint32_t tab_xxx_entry_num();
void tab_xxx_dump_all_entry();
#endif
