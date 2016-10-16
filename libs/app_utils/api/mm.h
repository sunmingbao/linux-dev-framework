/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __MM_H__
#define  __MM_H__
#include <stdint.h>
typedef    void *    MM_HANDLE;
typedef int (*p_func)(void *buf);
MM_HANDLE create_buffer_manager(int buf_size, int num);
int  MM_HANDLE_IS_VALID(MM_HANDLE hd);
void free_buffer(MM_HANDLE hd, void *buf);
void *alloc_buffer(MM_HANDLE hd);
void add_to_all_link(MM_HANDLE hd, void *buf);
void delete_from_all_link(MM_HANDLE hd, void *buf);
void for_each_buf_in_all_link(MM_HANDLE hd, p_func usr_func);
uint32_t free_buffer_num(MM_HANDLE hd);
#endif

