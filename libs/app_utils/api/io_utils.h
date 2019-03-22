/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __IO_UTILS_H__
#define  __IO_UTILS_H__

#include <unistd.h>
#include <stdio.h>

#define    RT_PRINT(fmt, args...) \
    do \
    { \
        printf(fmt, ##args); \
        fflush(stdout); \
    } while (0)

int fd_readable(int fd, int sec, int usec);
int fd_writeable(int fd, int sec, int usec);
int set_fd_nonblock(int fd);
int write_reliable(int fd, const void *buf, size_t count);
int write_certain_bytes(int fd, const void *buf, size_t count, int *written);
int read_reliable(int fd, void *buf, size_t count);
int read_certain_bytes(int fd, void *buf, size_t count, int *actual_got);
int printf_to_fd(int fd, const char *fmt, ...);
int get_temp_file(char *path);

#endif

