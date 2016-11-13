/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/stat.h>
#include <sys/time.h>

#include "misc_utils.h"
#include "sys_utils.h"
#include "debug.h"
#include "defs.h"
#include "log.h"

void nano_sleep(long sec, long nsec)
{
    struct timespec remain  = (struct timespec) {sec, nsec};
    struct timespec tmp = (struct timespec) {0, 0};
    while (remain.tv_sec || remain.tv_nsec)
    {
        if (0==nanosleep(&remain, &tmp)) return;
        remain = tmp;
        tmp = (struct timespec) {0, 0};
    }

}

int get_self_path(char *buf, int buf_len)
{
    int ret = readlink("/proc/self/exe", buf, buf_len);
    
    if (-1==ret)      goto FAIL_EXIT;
    if (ret>=buf_len) goto FAIL_EXIT;

 

    /* readlink仅将软链接文件的内容拷贝到buf中，
       却不会追加字符串结束符，
       因此我们需要自己追加字符串结束符。*/
    buf[ret]='\0';
//printf("\"%s\"", buf);
    return ret; 

 

FAIL_EXIT:
    printf("ret = %d", ret);
    return -1;

}

int get_self_dir(char *buf, int buf_len)
{
    char self_path[MAX_FILE_PATH_LEN];
    int slash_idx;
    int path_len = get_self_path(self_path, sizeof(self_path));
    if (path_len <= 0)
    {
        printf("get_self_path failed");
        goto FAIL_EXIT;
    }

    slash_idx = path_len-1;
    while (path_len > 0 && self_path[slash_idx]!='/')
    {
        self_path[slash_idx]='\0';
        slash_idx--;
        path_len--;
    }

    if (path_len<=0)
    {
        printf("unknown error");
        goto FAIL_EXIT;
    }

    if (buf_len<=path_len)
    {
        printf("buf_len error");
        goto FAIL_EXIT;
    }

    strcpy(buf, self_path);
//printf("\"%s\"", buf);
    return path_len; 

 

FAIL_EXIT:
    return -1;

}

int set_workdir_to_self_path()
{
    char self_path[MAX_PATH_LEN];
    if (get_self_dir(self_path, sizeof(self_path))<=0) return -1;
    //SysLog("%s", self_path);
    return chdir(self_path);
}

int genPIDfile(char *szPidFile)
{
    FILE *pid_fp = fopen(szPidFile, "w");

    if (pid_fp == NULL)
    {
        ErrSysLog("Can't open %s\n", szPidFile);
        return -1;
    }

    fprintf(pid_fp, "%d\n", getpid());
    fchmod(fileno(pid_fp), S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
    fclose(pid_fp);
    return 0;
}

int file_exists(char *file_path)
{
    struct stat buf;
    int ret = stat(file_path, &buf);
    return (0==ret);
}


int wait_for_file_exists(int sec, char *file_path)
{
    while (!file_exists(file_path))
    {
        if (sec<=0)
        {
            return 0;
        }
        
        nano_sleep(1, 0);
        sec--;

    }
    return 1;
}




int register_sighandler(int signum, void (*handler)(int))
{
	struct sigaction siginst;
	siginst.sa_flags = 0;
	sigfillset(&siginst.sa_mask);
	siginst.sa_handler = handler;
	if (sigaction(signum, &siginst, NULL))
    {   
		SysLog("Failed to register signal handler");
        return -1;
    }
    return 0;
}


void print_time()
{
    struct timeval tv;
    if (gettimeofday(&tv, NULL))
        DBG_PRINT("gettimeofday failed");
    else
        DBG_PRINT("gettimeofday: sec %lu usec %lu"
        , (unsigned long)(tv.tv_sec)
        , (unsigned long)(tv.tv_usec));
}

