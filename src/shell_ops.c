#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include "debug.h"
#include "log.h"

int get_cmd_result(char *buf, int buflen, const char *cmd)
{
    char cmd_str[128];
    int fd;
    int ret;

    char output_file[64];

    get_tmp_file_name(output_file, sizeof(output_file));

        sprintf(cmd_str
            , "%s  > %s"
            , cmd
            , output_file);
        
        system(cmd_str);

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


