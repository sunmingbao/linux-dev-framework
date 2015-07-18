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
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include "log.h"
#include "debug.h"

char *log_file_name;
char *old_log_file_name;
int max_log_file_size;

int log_inited = 0;
int fd_log_file=-1;

int get_log_file_fd()
{
    return fd_log_file;
}

int get_file_size(int fd)
{
    struct stat buf;
    int ret = fstat(fd, &buf);
    if (ret)
    {
        ERR_DBG_PRINT("stat %s failed", log_file_name);
        exit(1);
        return -1;

    }

    return buf.st_size;
}

void new_log_file_name()
{
    time_t log_time = time(NULL);
    sprintf(log_file_name, "%s", old_log_file_name);
    strftime(log_file_name+strlen(log_file_name),  32, "(%Y%m%d_%H%M%S).log", localtime(&log_time));


}

int init_log(char *file_name, int file_size)
{
    int file_name_buf_len;
    if (log_inited) return 0;
    max_log_file_size = file_size;
    file_name_buf_len = strlen(file_name)+32;
    log_file_name     = malloc(file_name_buf_len);
    old_log_file_name = malloc(file_name_buf_len);
    if ((NULL==log_file_name)||(NULL==old_log_file_name))
    {
        ERR_DBG_PRINT("malloc failed");
        exit(1);
        return -1;

    }
    
    sprintf(old_log_file_name, "%s", file_name);
    new_log_file_name();

    fd_log_file=open(log_file_name
            , O_CREAT|O_WRONLY|O_APPEND|O_TRUNC
            , S_IRUSR | S_IWUSR | S_IRGRP);

    if (fd_log_file < 0)
    {
        ERR_DBG_PRINT("open %s failed", log_file_name);
        exit(1);
        return -1;

    }

    log_inited = 1;
    return 0;

}

void close_file()
{
    if (fd_log_file>0) close(fd_log_file);
    fd_log_file = -1;
}

int write_file(void *buf, int len)
{
    int file_size;
    int ret;

    if (!log_inited) return -1;
    
    file_size = get_file_size(fd_log_file);
    
    if (file_size>=max_log_file_size)
    {
        close_file();
        new_log_file_name();
        fd_log_file=open(log_file_name
            , O_CREAT|O_WRONLY|O_APPEND|O_TRUNC
            , S_IRUSR | S_IWUSR | S_IRGRP);

        if (fd_log_file < 0)
        {
            ERR_DBG_PRINT("open %s failed", log_file_name);
            exit(1);
            return -1;

        }

    }
    
    ret = write(fd_log_file, buf, len);
    if (ret < 0)
    {
        ERR_DBG_PRINT("write %s failed", log_file_name);
        exit(1);
        return -1;

    }

    return 0;

}

int write_log(const char *fmt, ...)
{
    time_t log_time = time(NULL);
    int len1, len2;
    char buf[288];
    va_list ap;

    len1 = strftime(buf,  sizeof(buf), "[%Y%m%d_%H%M%S]:", localtime(&log_time));
    va_start(ap, fmt);
    len2 = vsnprintf(buf+len1, sizeof(buf)-len1, fmt, ap);
    va_end(ap);

    buf[len1 + len2] = '\n';
    buf[len1 + len2 + 1] = '\0';
    return write_file(buf, len1 + len2 + 1);
}



