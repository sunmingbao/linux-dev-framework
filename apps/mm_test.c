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
#include <string.h>
#include "mm.h"

int print_buf(void *buf)
{
    printf("buf contents: %s\n", (char *)buf);
    return 0;
}

int main(int argc, char *argv[])
{
    MM_HANDLE hd=create_buffer_manager(128, 3);
    char *buf1, *buf2, *buf3;
    assert(MM_HANDLE_IS_VALID(hd));
    assert(3==free_buffer_num(hd));

    buf1 = alloc_buffer(hd);
    assert(2==free_buffer_num(hd));
    strcpy(buf1, "buf1");
    printf("buf1=%p : %s\n", buf1, buf1);
    
    buf2 = alloc_buffer(hd);
    assert(1==free_buffer_num(hd));
    strcpy(buf2, "buf2");
    printf("buf2=%p : %s\n", buf2, buf2);

    buf3 = alloc_buffer(hd);
    assert(0==free_buffer_num(hd));
    strcpy(buf3, "buf3");
    printf("buf3=%p : %s\n", buf3, buf3);

    for_each_buf_in_all_link(hd, print_buf);
    add_to_all_link(hd, buf3);
    add_to_all_link(hd, buf2);
    add_to_all_link(hd, buf1);
    for_each_buf_in_all_link(hd, print_buf);
    delete_from_all_link(hd, buf3);
    delete_from_all_link(hd, buf2);
    delete_from_all_link(hd, buf1);
    for_each_buf_in_all_link(hd, print_buf);

    free_buffer(hd, buf1);
    free_buffer(hd, buf2);
    free_buffer(hd, buf3);

    buf1 = alloc_buffer(hd);
    assert(2==free_buffer_num(hd));
    strcpy(buf1, "buf1");
    printf("buf1=%p : %s\n", buf1, buf1);
    
    buf2 = alloc_buffer(hd);
    assert(1==free_buffer_num(hd));
    strcpy(buf2, "buf2");
    printf("buf2=%p : %s\n", buf2, buf2);
    
    buf3 = alloc_buffer(hd);
    assert(0==free_buffer_num(hd));
    strcpy(buf3, "buf3");
    printf("buf3=%p : %s\n", buf3, buf3);
    
    return 0;
}

