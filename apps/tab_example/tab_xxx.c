#include <errno.h>
#include "tab_xxx.h"
#include "debug.h"
#include "mm.h"

#define    MAX_HASH_LINE_NUM    (1024)

typedef struct
{
#if 1
    /* we use mm to manage free entry */
    MM_HANDLE mem_pool;
#else
	uint32_t		   free_entry_num;
	struct list_head   link_free;
	pthread_spinlock_t link_lock_free;
#endif

	pthread_spinlock_t add_del_lock;

	uint32_t           entry_num_in_use;
	
    struct list_head   link_all;
	pthread_spinlock_t link_lock_all;
	
	struct list_head  hash_tab_by_id[MAX_HASH_LINE_NUM];
	pthread_spinlock_t hash_line_lock_by_id[MAX_HASH_LINE_NUM];
	
	struct list_head  hash_tab_by_key[MAX_HASH_LINE_NUM];
	pthread_spinlock_t hash_line_lock_by_key[MAX_HASH_LINE_NUM];
} t_xxx_tab;


static t_xxx_tab the_tab_xxx;

static uint32_t id_hash(uint32_t id)
{
    return id % MAX_HASH_LINE_NUM;
}

static uint32_t key_hash(uint32_t a, uint32_t b, uint32_t c)
{
    return (a*b*c) % MAX_HASH_LINE_NUM;
}

static void *search_entry_by_id_in_link(struct list_head   *head, uint32_t id)
{
	t_tab_xxx_entry *pt_entry;
	struct list_head *pos;
	
	list_for_each(pos, head)
	{
		pt_entry = list_entry(pos, t_tab_xxx_entry, by_id);
		if (id == pt_entry->id)
		{
			return pt_entry;
		}

	}

    return NULL;
}

static void *search_entry_by_id(uint32_t id)
{
	int line = id_hash(id);
	
	t_tab_xxx_entry *pt_entry;

	pthread_spin_lock(&(the_tab_xxx.hash_line_lock_by_id[line]));
	pt_entry = search_entry_by_id_in_link(&(the_tab_xxx.hash_tab_by_id[line]), id);
	pthread_spin_unlock(&(the_tab_xxx.hash_line_lock_by_id[line]));

    return pt_entry;
}

static uint32_t gen_id()
{
    static     uint32_t     id_src = 1;
	
	while (search_entry_by_id(id_src)!=NULL)
	{
		id_src++;
	}

	return id_src;
}

static void *search_entry_by_key_in_link(struct list_head   *head, uint32_t a, uint32_t b, uint32_t c)
{
	t_tab_xxx_entry *pt_entry;
	struct list_head *pos;
	
	list_for_each(pos, head)
	{
		pt_entry = list_entry(pos, t_tab_xxx_entry, by_key);
		if ((a == pt_entry->a) &&
			(b == pt_entry->b) &&
			(c == pt_entry->c))
		{
			return pt_entry;
		}

	}

    return NULL;
}

static void *search_entry_by_key(uint32_t a, uint32_t b, uint32_t c)
{
	int line = key_hash(a, b, c);
	
	t_tab_xxx_entry *pt_entry;

	pthread_spin_lock(&(the_tab_xxx.hash_line_lock_by_key[line]));
	pt_entry = search_entry_by_key_in_link(&(the_tab_xxx.hash_tab_by_key[line]), a, b, c);
	pthread_spin_unlock(&(the_tab_xxx.hash_line_lock_by_key[line]));

    return pt_entry;
}

static void do_add_entry_to_all(t_tab_xxx_entry *pt_entry)
{
	pthread_spin_lock(&(the_tab_xxx.link_lock_all));
	list_add(&(pt_entry->all), &(the_tab_xxx.link_all));
	pthread_spin_unlock(&(the_tab_xxx.link_lock_all));
}

static void do_add_entry_to_id_hash(t_tab_xxx_entry *pt_entry)
{
	int line = id_hash(pt_entry->id);

	pthread_spin_lock(&(the_tab_xxx.hash_line_lock_by_id[line]));
	list_add(&(pt_entry->by_id), &(the_tab_xxx.hash_tab_by_id[line]));
	pthread_spin_unlock(&(the_tab_xxx.hash_line_lock_by_id[line]));

}
static void do_add_entry_to_key_hash(t_tab_xxx_entry *pt_entry)
{
	int line = key_hash(pt_entry->a, pt_entry->b, pt_entry->c);

	pthread_spin_lock(&(the_tab_xxx.hash_line_lock_by_key[line]));
	list_add(&(pt_entry->by_key), &(the_tab_xxx.hash_tab_by_key[line]));
	pthread_spin_unlock(&(the_tab_xxx.hash_line_lock_by_key[line]));

}


static void add_entry_into_tab(t_tab_xxx_entry *pt_entry)
{

    do_add_entry_to_id_hash(pt_entry);
	do_add_entry_to_key_hash(pt_entry);
	do_add_entry_to_all(pt_entry);
    the_tab_xxx.entry_num_in_use++;
}

static void do_del_entry_from_all(t_tab_xxx_entry *pt_entry)
{
	pthread_spin_lock(&(the_tab_xxx.link_lock_all));
	list_del(&(pt_entry->all));
	pthread_spin_unlock(&(the_tab_xxx.link_lock_all));
}

static void do_del_entry_from_id_hash(t_tab_xxx_entry *pt_entry)
{
	int line = id_hash(pt_entry->id);

	pthread_spin_lock(&(the_tab_xxx.hash_line_lock_by_id[line]));
	list_del(&(pt_entry->by_id));
	pthread_spin_unlock(&(the_tab_xxx.hash_line_lock_by_id[line]));

}
static void do_del_entry_from_key_hash(t_tab_xxx_entry *pt_entry)
{
	int line = key_hash(pt_entry->a, pt_entry->b, pt_entry->c);

	pthread_spin_lock(&(the_tab_xxx.hash_line_lock_by_key[line]));
	list_del(&(pt_entry->by_key));
	pthread_spin_unlock(&(the_tab_xxx.hash_line_lock_by_key[line]));

}

static void del_entry_from_tab(t_tab_xxx_entry *pt_entry)
{

    do_del_entry_from_id_hash(pt_entry);
	do_del_entry_from_key_hash(pt_entry);
	do_del_entry_from_all(pt_entry);
    the_tab_xxx.entry_num_in_use--;
}

int tab_xxx_init(int max_entry_num)
{
    int i;
	
    the_tab_xxx.mem_pool = create_buffer_manager(sizeof(t_tab_xxx_entry), max_entry_num);
	if (NULL == the_tab_xxx.mem_pool)
	{
        errno = ENOMEM;
		goto FAIL_EXIT;
	}

	pthread_spin_init(&(the_tab_xxx.add_del_lock), PTHREAD_PROCESS_PRIVATE);

	the_tab_xxx.entry_num_in_use = 0;
	INIT_LIST_HEAD(&(the_tab_xxx.link_all));
	pthread_spin_init(&(the_tab_xxx.link_lock_all), PTHREAD_PROCESS_PRIVATE);

	for (i=0; i<MAX_HASH_LINE_NUM; i++)
	{
		INIT_LIST_HEAD(&(the_tab_xxx.hash_tab_by_id[i]));
		pthread_spin_init(&(the_tab_xxx.hash_line_lock_by_id[i]), PTHREAD_PROCESS_PRIVATE);

		INIT_LIST_HEAD(&(the_tab_xxx.hash_tab_by_key[i]));
		pthread_spin_init(&(the_tab_xxx.hash_line_lock_by_key[i]), PTHREAD_PROCESS_PRIVATE);

	}
	
    return 0;
	
FAIL_EXIT:
	return -1;
}

static void build_new_entry(t_tab_xxx_entry *pt_entry, uint32_t a, uint32_t b, uint32_t c, uint32_t priv_data)
{
	pt_entry->id = gen_id();
	pt_entry->a = a;
	pt_entry->b = b;
	pt_entry->c = c;
	pt_entry->priv_data = priv_data;
	
	pt_entry->ref_cnt=1;
	pthread_spin_init(&(pt_entry->lock), PTHREAD_PROCESS_PRIVATE);

}

static void inc_entry_ref_cnt(t_tab_xxx_entry *pt_entry)
{
	pthread_spin_lock(&(pt_entry->lock));
	pt_entry->ref_cnt++;
	pthread_spin_unlock(&(pt_entry->lock));

}

static void dec_entry_ref_cnt(t_tab_xxx_entry *pt_entry)
{
	pthread_spin_lock(&(pt_entry->lock));
	pt_entry->ref_cnt--;
	pthread_spin_unlock(&(pt_entry->lock));

}

int tab_xxx_add_entry(uint32_t a, uint32_t b, uint32_t c, uint32_t priv_data)
{
    int ret = 0;
	t_tab_xxx_entry *pt_entry;
	
	pthread_spin_lock(&(the_tab_xxx.add_del_lock));

    if (NULL != search_entry_by_key(a, b, c))
	{
	    errno = EEXIST;
		ret = -1;
		goto EXIT;
	}

	pt_entry = alloc_buffer(the_tab_xxx.mem_pool);
    if (NULL == pt_entry)
	{
	    errno = ENOMEM;
		ret = -1;
		goto EXIT;
	}

	build_new_entry(pt_entry, a, b, c, priv_data);
    add_entry_into_tab(pt_entry);

EXIT:
	pthread_spin_unlock(&(the_tab_xxx.add_del_lock));
	return ret;
}

void * tab_xxx_get_entry_by_key(uint32_t a, uint32_t b, uint32_t c)
{
	int line = key_hash(a, b, c);
	
	t_tab_xxx_entry *pt_entry;

	pthread_spin_lock(&(the_tab_xxx.hash_line_lock_by_key[line]));
	pt_entry = search_entry_by_key_in_link(&(the_tab_xxx.hash_tab_by_key[line]), a, b, c);
	if (pt_entry)
	{
        inc_entry_ref_cnt(pt_entry);
	}
	pthread_spin_unlock(&(the_tab_xxx.hash_line_lock_by_key[line]));

	return pt_entry;
}

void   tab_xxx_put(t_tab_xxx_entry *pt_entry)
{
    dec_entry_ref_cnt(pt_entry);
	
	if (0==pt_entry->ref_cnt)
	{
	    /* everything needed for delete has been done, except free buffer */
		free_buffer(the_tab_xxx.mem_pool, pt_entry);
	}
}

int tab_xxx_del_entry_by_key(uint32_t a, uint32_t b, uint32_t c)
{
	int ret = 0;
	t_tab_xxx_entry *pt_entry;
	
	pthread_spin_lock(&(the_tab_xxx.add_del_lock));
	pt_entry = search_entry_by_key(a, b, c);

	if (!pt_entry)
	{
		errno = ESRCH;
		ret = -1;
		goto EXIT;
	}


	del_entry_from_tab(pt_entry);
	tab_xxx_put(pt_entry);

EXIT:
	pthread_spin_unlock(&(the_tab_xxx.add_del_lock));
	return ret;
}


uint32_t tab_xxx_entry_num()
{
	uint32_t   entry_num_in_use;
	
	pthread_spin_lock(&(the_tab_xxx.add_del_lock));
	entry_num_in_use = the_tab_xxx.entry_num_in_use;
	pthread_spin_unlock(&(the_tab_xxx.add_del_lock));

	return entry_num_in_use;
}

void tab_xxx_dump_all_entry()
{
	t_tab_xxx_entry *pt_entry;
	
	struct list_head *pos, *head;
	
	pthread_spin_lock(&(the_tab_xxx.add_del_lock));

	head = &(the_tab_xxx.link_all);
	
	list_for_each(pos, head)
	{
		pt_entry = list_entry(pos, t_tab_xxx_entry, all);
		DBG_PRINT("id=%u", pt_entry->id);

	}
	pthread_spin_unlock(&(the_tab_xxx.add_del_lock));
}

