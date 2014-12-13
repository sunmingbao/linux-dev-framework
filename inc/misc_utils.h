/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __MISC_UTILS_H__
#define  __MISC_UTILS_H__

#include   "defs.h"

void nano_sleep(long sec, long nsec);
int genPIDfile(char *szPidFile);
int register_sighandler(int signum, void (*handler)(int));
int get_cmd_result_int(const char *cmd, int *result_code);
int get_cmd_result(char *buf, int buflen, const char *cmd);
int mem_addr_vir2phy(unsigned long vir, unsigned long *phy);
int fd_readable(int fd, int usec);
void itimer_init(int s, int us);
#endif

