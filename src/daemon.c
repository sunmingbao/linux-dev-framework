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
#include <linux/reboot.h>
#include <sys/reboot.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include <syslog.h>
#include <fcntl.h>
#include <sys/resource.h>
#include <signal.h>
#include <sys/wait.h>
#include "debug.h"

struct sigaction    sa;
void signal_config()
{
    int i;
    /*
     * Ensure future opens won't allocate controlling TTYs.
     */
    sa.sa_handler = SIG_IGN;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags =0; 
    
    if (sigaction(SIGHUP, &sa, NULL) < 0)
    {
        ERR_DBG_PRINT_QUIT("can't ignore SIGHUP");
    }

#if 1
    /* SIGRTMIN（通常大于30，具体见kill -l）以上的信号全部忽略 */
    for (i = SIGRTMIN; i <= SIGRTMAX; i++)
    {
        if (sigaction(i, &sa, NULL) < 0)
        {
            ERR_DBG_PRINT_QUIT("can't ignore signal %d", i );
        }
    }
#endif

}


void daemonize(int redirect_std_io, const char *log_id)
{
    int                 i, fd0, fd1, fd2;
    pid_t               pid;
    struct rlimit       rl;

    /*
     * Clear file creation mask.
     */
    umask(0);

    /*
     * Become a session leader to lose controlling TTY.
     */
    if ((pid = fork()) < 0)
    {
        ERR_DBG_PRINT_QUIT("can't fork");
    }
    
    if (pid != 0) /* parent */
    {
        exit(0);
    }

    if ((pid_t)-1 == setsid())
    {
        ERR_DBG_PRINT_QUIT("setsid() failed");
    }

    signal_config();
    
    
    if ((pid = fork()) < 0)
    {
        ERR_DBG_PRINT_QUIT("can't fork");
    }
    
    if (pid != 0) /* parent */
    {
        exit(0);
    }
    
#if 0
    /*
     * Change the current working directory to the root so
     * we won't prevent file systems from being unmounted.
     */
    if (chdir("/") < 0)
        err_quit("can't change directory to /");
#endif
    /*
     * Close all opened file.
     */
    if (getrlimit(RLIMIT_NOFILE, &rl) < 0)
    {
        ERR_DBG_PRINT("getrlimit failed");
        rl.rlim_max = 1024;
    }

    /*
     * Close all open file descriptors.
     */
    if (rl.rlim_max == RLIM_INFINITY)
        rl.rlim_max = 1024;
    for (i = 3; i < rl.rlim_max; i++)
        close(i);
    
if (0==redirect_std_io) goto FD_PREPARE_OVER;
    
    for (i = 0; i < 3; i++)
        close(i);

    /*
     * Attach file descriptors 0, 1, and 2 to /dev/null.
     */
    fd0 = open("/dev/null", O_RDWR);
    fd1 = dup(fd0);
    fd2 = dup(fd0);
    if (fd0 != 0 || fd1 != 1 || fd2 != 2) 
    {
        ERR_DBG_PRINT("unexpected file descriptors %d %d %d",
          fd0, fd1, fd2);
    }
FD_PREPARE_OVER:
    /*
     * Initialize the log file.
     */
     if (log_id != NULL)
        openlog(log_id, LOG_CONS, LOG_DAEMON | LOG_NDELAY);
}

