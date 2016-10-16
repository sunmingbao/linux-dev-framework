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
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "io_utils.h"
#include "debug.h"
#include "log.h"

int cmd2file(const char *cmd, char *file_path)
{
    char cmd_str[512];
    int ret = get_temp_file(file_path);
    
    if (ret<0) return ret;

    sprintf(cmd_str
            , "%s  > %s"
            , cmd
            , file_path);
        
    system(cmd_str);
    return 0;
}

int get_cmd_result(char *buf, int buflen, const char *cmd)
{
    int fd;
    int ret;
    char output_file[64];

    cmd2file(cmd, output_file);
    fd = open(output_file, O_RDONLY);
    if (fd == -1 )
    {
        ErrSysLog("open %s failed", output_file);
        unlink(output_file);
        return -1;
    }

    ret=read(fd, buf, buflen);
    close(fd);
    unlink(output_file);

    if (ret>0)
    {
        buf[ret] = 0;
        return 0;
    }

    ErrSysLog("read %s failed", output_file);
    return -1;
}

int get_cmd_result_int(const char *cmd, int *result_code)
{
    char buf[64] = {0};
    int ret = get_cmd_result(buf, sizeof(buf), cmd);

    if (result_code != NULL) *result_code = ret;
    return atoi(buf);
}


