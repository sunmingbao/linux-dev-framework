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

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <linux/reboot.h>
#include <sys/reboot.h>

#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/wait.h>

#include "p2p.h"
#include "socket.h"
#include "common.h"
#include "config_file_ops.h"
#include "misc_utils.h"
#include "log.h"
#include "debug.h"
#include "daemon.h"

int daemonize_process=0;
int redirect_std_io=0;

int use_dft_config_file=1;

#define  INVALID_PID     ((pid_t)-1)

#define CONFIG_FILE_NAME   "guard.conf"
char config_file_path[MAX_PATH_LEN];
typedef struct
{
    char   bin_path[MAX_PATH_LEN];
    char   args[MAX_ARGS_LEN];
    char   *argv[16];
    int    restore_mode;
        
    pid_t  pid;
    int    status; /* reserved; do not use */
    int    flags;  /* reserved; do not use */


}t_process_item;

#define RESTORE_MODE_MACHINE_STR    "machine"
#define RESTORE_MODE_PROCESS_STR    "process"

#define RESTORE_MODE_MACHINE    (0)
#define RESTORE_MODE_PROCESS    (1)

t_process_item  *g_pt_process_list;
int              g_process_num;

#define    MAX_PROCESS_NUM    (128)

int g_need_exit = 0;
static void sig_handler(int signo)
{
    SysLog("signal %d received", signo);
	if (signo == SIGINT) g_need_exit = 1;
}

void kill_process(pid_t pid, char *name)
{
    SysLog("kill process: %s ", name);

    kill(pid, SIGINT);
    nano_sleep(1, 0);
    kill(pid, SIGINT);
    nano_sleep(1, 0);

    kill(pid, SIGKILL);

}

void start_process(t_process_item  * pt_process)
{
    pid_t               pid;
    
    SysLog("start process: %s ", pt_process->bin_path);
    if ((pid = fork()) < 0)
    {
        ErrSysLog("fork failed!");
        return;
    }
    
    if (pid != 0) /* parent */
    {
        pt_process->pid = pid;
    }
    else
    {
        execv(pt_process->bin_path, 
               pt_process->argv);
        ErrSysLog("execlp %s %s failed!", pt_process->bin_path, pt_process->args);
        exit(1);
    }

}


int process_exited(t_process_item  * pt_process)
{
    int status;
    pid_t  pid = waitpid(pt_process->pid, &status, WNOHANG | WUNTRACED);

    if ((pid_t)-1 == pid)
    {
        ErrSysLog("waitpid failed");
        return 0;
    }

    if ((pid_t)0 == pid)
    {
        return 0;
    }

    if (pid == pt_process->pid)
    {
        SysLog("opoos~~~ child  %s (pid:%u) down", pt_process->bin_path, (uint32_t)pid);
        if (!(WIFEXITED(status)))
        {
            SysLog("opoos, unknown exception happened");
            return 1;
        }
        
        SysLog("child exit with code %d", WEXITSTATUS(status));
        return 1;
    }

    SysLog("unknown child  %u down", (uint32_t)pid);
    return 0;
}

void kill_process_list()
{
    int i;
    t_process_item  * pt_process;
    SysLog("kill all process");
    for (i = 0; i<g_process_num; i++)
    {
        pt_process = &(g_pt_process_list[i]);
        if (pt_process->pid != INVALID_PID)
        {
            kill_process(pt_process->pid, pt_process->bin_path);
            pt_process->pid = INVALID_PID;
        }

    }
}

void restart_system()
{
        SysLog("restart system ");
        kill_process_list();
        sync(); /* flush all data to disks*/
        //reboot(LINUX_REBOOT_CMD_RESTART);
        system("reboot");

}

void restart_process(t_process_item  * pt_process)
{
    SysLog("restart process: %s ", pt_process->bin_path);
    if (pt_process->pid != INVALID_PID)
    {
        kill_process(pt_process->pid, pt_process->bin_path);
        pt_process->pid = INVALID_PID;
    }

    if (RESTORE_MODE_MACHINE == pt_process->restore_mode)
    {
        restart_system();
        exit(1);

    }
    start_process(pt_process);

}
void init_one_process(int idx)
{
    char var_name[MAX_VAR_NAME_LEN];
    char *p_value;
    int arg_num = 0;
    
    t_process_item  * pt_process = &(g_pt_process_list[idx]);
    memset(pt_process, 0, sizeof(t_process_item));
    pt_process->pid = INVALID_PID;

    snprintf(var_name, MAX_VAR_NAME_LEN, "bin_path_%d", idx);
    p_value = get_config_var(var_name);
    strncpy(pt_process->bin_path, p_value, MAX_PATH_LEN);
    SysLog("%s: %s", var_name, pt_process->bin_path);


    snprintf(var_name, MAX_VAR_NAME_LEN, "args_%d", idx);
    p_value = get_config_var(var_name);
    strncpy(pt_process->args, p_value, MAX_ARGS_LEN);
    SysLog("%s: %s", var_name, pt_process->args);

    pt_process->argv[arg_num] = pt_process->bin_path;
    arg_num++;

    pt_process->argv[arg_num] = strtok(pt_process->args, " ");
    if (NULL != pt_process->argv[arg_num]) arg_num++;
    while (arg_num<15)
    {
        pt_process->argv[arg_num] = strtok(NULL, " ");
        if (NULL == pt_process->argv[arg_num]) break;
        arg_num++;
    }
    pt_process->argv[arg_num]=NULL;

    snprintf(var_name, MAX_VAR_NAME_LEN, "restore_mode_%d", idx);
    p_value = get_config_var(var_name);
    SysLog("%s: %s", var_name, p_value);

    if (0 == strcmp(RESTORE_MODE_MACHINE_STR, p_value))
    {
        pt_process->restore_mode = RESTORE_MODE_MACHINE;
    }
    else if (0 == strcmp(RESTORE_MODE_PROCESS_STR, p_value))
    {
        pt_process->restore_mode = RESTORE_MODE_PROCESS;
    }
    else
    {
        ERR_DBG_PRINT_QUIT("process %d invalid restore mode %s(should be %s or %s)", 
            idx, p_value, RESTORE_MODE_MACHINE_STR, RESTORE_MODE_PROCESS_STR);
    }



}
void start_process_list()
{
    int i;
    t_process_item  * pt_process;
    for (i = 0; i<g_process_num; i++)
    {
        pt_process = &(g_pt_process_list[i]);
        start_process(pt_process);
    }
}


void init_process_list()
{
    int i;
    
    if (parse_config_file(config_file_path, 0) < 1)
    {
        ERR_DBG_PRINT_QUIT("parse cfg file abnormoal %s", config_file_path);
    }
    
    g_process_num = atoi(get_config_var("process_num"));
    if ((g_process_num < 0) || (g_process_num > MAX_PROCESS_NUM))
    {
        ERR_DBG_PRINT_QUIT("invalid process_num %d", g_process_num);
    }
    
    g_pt_process_list = malloc(g_process_num * sizeof(t_process_item));
    if (NULL == g_pt_process_list)
    {
        ERR_DBG_PRINT_QUIT("malloc mem for process_list failed");
    }

    for (i = 0; i<g_process_num; i++)
    {
        init_one_process(i);
    }
    
    free_config();
}


void parse_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt(argc, argv, "drc:")) != -1)
    {
       switch (opt)
       {
           case 'd':
               daemonize_process= 1;
               DBG_PRINT("==daemonize_process");
               break;

           case 'r':
               redirect_std_io = 1;
               DBG_PRINT("==%d", redirect_std_io);
               break;
               
           case 'c': 
               use_dft_config_file = 0;
               strcpy(config_file_path, optarg);
               DBG_PRINT("==user provides config file :%s", config_file_path);
               break;


           default: /* '?' */
               DBG_PRINT("Usage: %s [-d [-r]]  [-c /path/to/cfg_file]\n",
                       argv[0]);
       }
    }


}

int main(int argc, char * argv[])
{
    char pid_file_path[128];
    
    parse_args(argc, argv);

    if (daemonize_process)
        daemonize(redirect_std_io, "iGuard");
    
    if (register_sighandler(SIGINT, sig_handler)<0)
    {
        ERR_DBG_PRINT_QUIT("register_sighandler failed");
    }

    init_log("iGuard.log", DFT_LOG_FILE_SIZE);

    SysLog("*** iGuard launching *** ");
    
    
    if (use_dft_config_file)
    {
        sprintf(config_file_path, "%s", CONFIG_FILE_NAME);
    }
    
    SysLog("%s\n", config_file_path);
    init_process_list();

    sprintf(pid_file_path, "/tmp/iGuard.exe.pid");
    if (genPIDfile(pid_file_path))
    {
        ErrSysLog("genPIDfile failed.");
    }

    SysLog("*** iGuard launch succeed *** ");

    start_process_list();
    


restart:
    if (g_need_exit)
    {
        int i;
        t_process_item  * pt_process;

        SysLog("it's time to quit!");
        kill(0, SIGINT);
        nano_sleep(1, 0);
        kill(0, SIGINT);
        nano_sleep(1, 0);

#if 1
        for (i = 0; i < g_process_num; i++)
        {
            pt_process = &(g_pt_process_list[i]);
            if (process_exited(pt_process))  continue;
            SysLog("now force %s (pid :%d) to quit!", pt_process->bin_path, pt_process->pid);
            kill(pt_process->pid, SIGKILL);
        }
#endif

        SysLog("iGuard quit");
        return 0;
    }
    
    {
            int i;
            t_process_item  * pt_process;
            for (i = 0; i < g_process_num; i++)
            {
                pt_process = &(g_pt_process_list[i]);
                if (process_exited(pt_process))
                {
                    restart_process(pt_process);
                    
                }

            }
    }

    nano_sleep(5, 0);
    goto restart;

    /* never return */
    return 0;

    
}
    

    


