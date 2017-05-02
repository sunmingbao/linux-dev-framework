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
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/vfs.h>
#include <time.h>
#include "defs.h"
#include "debug.h"
#include "misc_utils.h"
#include "log.h"
#include "string_utils.h"

#define  PROG_NAME    "disk_monitor"
#define CONFIG_FILE_NAME   PROG_NAME".conf"

#define    DFT_FS_SPACE_THRESHOLD (500)  /* MBytes */

uint64_t g_fs_space_threshold = DFT_FS_SPACE_THRESHOLD;
uint64_t g_space_test = 0;


uint64_t fs_free_space(char *dir)
{
    uint64_t blocks, block_size, ret;
    struct statfs s;
    if (statfs(dir, &s) != 0)
    {
        ErrSysLogQuit("statfs %s failed", dir);
        return 0;
    }
    
    blocks = s.f_bavail;
    block_size = s.f_bsize;
    ret = (blocks)*(block_size)/(1024*1024);
    return ret;
}

uint64_t fs_total_space(char *dir)
{
    uint64_t blocks, block_size, ret;
    struct statfs s;
    if (statfs(dir, &s) != 0)
    {
        ErrSysLogQuit("statfs %s failed", dir);
    }
    blocks = s.f_blocks;
    block_size = s.f_bsize;
    ret = (blocks)*(block_size)/(1024*1024);
    return ret;
}



void get_the_oldest_file(char *dir_path)
{
    char cmd[MAX_PATH_LEN];

    snprintf(cmd
             ,sizeof(cmd) 
             ,"ls -1tr %s | grep .log | sed -n '1p'| sed \"s:^:%s/:\" >./tmp_dir/old_file_name.txt"
             , dir_path
             , dir_path);

    system(cmd);
}

int delete_the_oldest_file()
{
    int ret = 0;
	int len;
    char *s;
    char line[MAX_PATH_LEN];
    char trimed_line[128];
    FILE *file;
    get_the_oldest_file("./");
        
    file = fopen("./tmp_dir/old_file_name.txt", "r");

    if (NULL == file)
    {
        ErrSysLog("open old_file_name file failed.");
        return 1;
    }
    s = fgets(line, sizeof(line), file);
    fclose(file);
    if (NULL == s)  goto EXIT;
    len = strlen(line);
    if (str_trim_len(trimed_line, line, len)<=0)
        goto EXIT;
    unlink(trimed_line);
    DBG_PRINT("delete %s", trimed_line);
    ret = 0;

EXIT:
    return ret;
}

#define  STORAGE_SIZE_UNIT     (1024*1024)


int NeedExit = 0;
void sig_handler(int signo)
{
    SysLog("signal %d received", signo);
	if (signo == SIGINT) NeedExit = 1;
}

void parse_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt(argc, argv, "n:t:")) != -1)
    {
       switch (opt)
       {
              
           case 'n': 
               g_fs_space_threshold = atoi(optarg);
               DBG_PRINT("==%lld", g_fs_space_threshold);
               break;
               
           case 't':
               g_space_test = atoi(optarg);
               DBG_PRINT("==%lld", g_space_test);
               break;


           default: /* '?' */
               DBG_PRINT("Usage: \n");
       }
    }


}

int space_under_water_level()
{
    uint64_t free_space=fs_free_space("./");
    return (free_space>0) && (free_space<g_fs_space_threshold);
}

int main(int argc, char * argv[])
{
    uint64_t sleep_cnt = 0;

    init_log(PROG_NAME, DFT_LOG_FILE_SIZE);
    SysLog("*** %s launching *** ", PROG_NAME);

    parse_args(argc, argv);

    if (register_sighandler(SIGINT, sig_handler)<0)
    {
        ErrSysLogQuit("register_sighandler failed");
    }

    
    SysLog("*** %s launch succeed *** ", PROG_NAME);

    if (g_space_test)
        g_fs_space_threshold = fs_free_space("./") - g_space_test;
DBG_PRINT("==%lld", g_fs_space_threshold);
    while (!NeedExit)
    {
        nano_sleep(1, 0);
        sleep_cnt++;

        if (sleep_cnt % 10)
            goto NEED_NOT_DEL_FILE;

        while (space_under_water_level())
        {
            if (delete_the_oldest_file())
            {
                ErrSysLog("delete_an_oldest_file failed");

            }
        
        }
        
NEED_NOT_DEL_FILE:

        ;

        
    }

    SysLog(PROG_NAME" quit");
    return 0;

    
}
    

    

