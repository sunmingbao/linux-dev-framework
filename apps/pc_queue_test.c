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
#include <errno.h>
#include <assert.h>
#include "debug.h"
#include "pc_queue.h"


int main(int argc, char *argv[])
{
    int a[] = {0,1,2,3,4};
    int value, i;
    t_pc_que *pt_q = create_pc_que(16);
    assert(pt_q);
    for (i=0;i<5;i++)
    assert(0==pc_que_enq_try(pt_q, &a[i], 0, NULL));
for (i=0;i<5;i++)
{
    value = *(int *)pc_que_outq_try(pt_q, NULL, NULL);
    assert(i==value);
    DBG_PRINT("value=%d", value);
}
    return 0;
}

