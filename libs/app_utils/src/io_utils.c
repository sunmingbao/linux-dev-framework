/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/select.h>
#include <stdarg.h>
#include <errno.h>

int fd_read_or_write_able(int fd, int sec, int usec, int test_read)
{
    fd_set fds, fds_except;
    struct timeval tv;
    int retval;

    FD_ZERO(&fds);
    FD_SET(fd, &fds);

    FD_ZERO(&fds_except);
    FD_SET(fd, &fds_except);

    tv.tv_sec = sec;
    tv.tv_usec = usec;

    if (test_read)
        retval = select(fd + 1, &fds, NULL, &fds_except, &tv);
    else
        retval = select(fd + 1, NULL, &fds, &fds_except, &tv);

    if (retval == -1)
    {
       if (EINTR==errno) return 0;
       return retval;
    }
    
    return retval>0;
}

int fd_readable(int fd, int sec, int usec)
{
    return fd_read_or_write_able(fd, sec, usec, 1);
}

int fd_writeable(int fd, int sec, int usec)
{
    return fd_read_or_write_able(fd, sec, usec, 0);
}

int set_fd_nonblock(int fd)
{
    int ret;
    if((ret = fcntl(fd, F_GETFL,0))==-1)
    {   
        return ret;
    }

    ret |= O_NONBLOCK;
    return fcntl(fd, F_SETFL, ret);
}

int write_reliable(int fd, const void *buf, size_t count)
{
    int ret;

TRY_AGAIN:
    ret = write(fd, buf, count);
    if (ret<0 && (EINTR==errno || EAGAIN==errno || EWOULDBLOCK==errno))
        goto TRY_AGAIN;

    return ret;
}

int write_certain_bytes(int fd, const void *buf, size_t count, int *written)
{
    int ret, left=count, finished=0;

    while (left>0)
    {
        ret = write_reliable(fd, buf+finished, left);
        if (ret<0)
            goto EXIT;

        finished+=ret;
        left-=ret;
    }

EXIT:
    if (written)
        (*written) = finished;

    return (left==0)?0:ret;
}

int read_reliable(int fd, void *buf, size_t count)
{
    int ret;

TRY_AGAIN:
    ret = read(fd, buf, count);
    if (ret<0 && (EINTR==errno || EAGAIN==errno || EWOULDBLOCK==errno))
        goto TRY_AGAIN;

    return ret;
}

int read_certain_bytes(int fd, void *buf, size_t count, int *actual_got)
{
    int ret, left=count, finished=0;

    while (left>0)
    {
        ret = read_reliable(fd, buf+finished, left);
        if (ret<=0)
            goto EXIT;

        finished+=ret;
        left-=ret;
    }

EXIT:
    if (actual_got)
        (*actual_got) = finished;

    return (left==0) ? 0 : (ret<0?ret:-1);
}

int printf_to_fd(int fd, const char *fmt, ...)
{
    int len;
    char buf[512];
    va_list ap;

    va_start(ap, fmt);
    len = vsnprintf(buf, sizeof(buf), fmt, ap);
    va_end(ap);

    return write_certain_bytes(fd, buf, len, NULL);
}

int get_temp_file(char *path)
{
    char file_name_template[] = "temp_XXXXXX";
    int ret;
TRY_AGAIN:
    ret=mkstemp(file_name_template);
    if (ret<0)
    {
        if (errno==EINTR)
            goto TRY_AGAIN;

        return ret;
    }

    close(ret);
    strcpy(path, file_name_template);
    return 0;
}
