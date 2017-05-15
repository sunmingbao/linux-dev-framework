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
#include "tab_xxx.h"

#include "debug.h"

#define    MAX_ADD_ENTRY_NUM    (1000)

void static add_test()
{
    int a=0, b=0, c=0;
	int i;
	for (i=0; i<MAX_ADD_ENTRY_NUM+2; i++)
	{
	    if (tab_xxx_add_entry(a, b, c, i))
    	{
    	    ERR_DBG_PRINT("a=%d; b=%d; c=%d; i=%d", a, b, c, i);

    	}
		a++;
		b++;
		c++;

	}
	
}

void static del_test()
{
    int a=0, b=0, c=0;
	int i;
	for (i=0; i<MAX_ADD_ENTRY_NUM+2; i++)
	{
	    if (tab_xxx_del_entry_by_key(a, b, c))
    	{
    	    ERR_DBG_PRINT("delete fail a=%d; b=%d; c=%d", a, b, c);

    	}
		a++;
		b++;
		c++;

	}
	
}


static void search_test()
{
	int a=0, b=0, c=0;
	int i;
	t_tab_xxx_entry *pt_entry;
	for (i=0; i<MAX_ADD_ENTRY_NUM+2; i++)
	{
	    pt_entry=tab_xxx_get_entry_by_key(a,b,c);
		if (!pt_entry)
		{
			DBG_PRINT("not found: a=%d; b=%d; c=%d; i=%d", a, b, c, i);
			goto    LOOP_BOTTOM;

		}
		
		if((pt_entry->a != a) ||
			(pt_entry->a != a) ||
			(pt_entry->c != c) ||
			(pt_entry->priv_data != i))
		{
			DBG_PRINT("wrong result");
			DBG_PRINT(" wanna : a=%d; b=%d; c=%d; i=%d"	, a, b, c, i);
			DBG_PRINT(" found : a=%d; b=%d; c=%d; i=%d"	, pt_entry->a, pt_entry->b, pt_entry->c, pt_entry->priv_data);

		}
		if (pt_entry->ref_cnt != 2)
			DBG_PRINT("wrong ref cnt = %u", pt_entry->ref_cnt);
		tab_xxx_put(pt_entry);
		if (pt_entry->ref_cnt != 1)
			DBG_PRINT("wrong ref cnt = %u", pt_entry->ref_cnt);

LOOP_BOTTOM:
		a++;
		b++;
		c++;

	}
	
}


int main(int argc, char *argv[])
{

    int ret;

	ret = tab_xxx_init(MAX_ADD_ENTRY_NUM);
	

	if (ret<0)
        ERR_DBG_PRINT_QUIT("==");
	
	DBG_PRINT("entry num = %u",tab_xxx_entry_num());

	add_test();
	DBG_PRINT("entry num = %u",tab_xxx_entry_num());
	search_test();
	DBG_PRINT("entry num = %u",tab_xxx_entry_num());
	//tab_xxx_dump_all_entry();
	del_test();
	DBG_PRINT("entry num = %u",tab_xxx_entry_num());

	while (1)
	{

        sleep(1);
	}
    return 0;
}

