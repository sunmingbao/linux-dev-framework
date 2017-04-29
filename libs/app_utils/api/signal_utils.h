/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __SIGNAL_UTILS_H__
#define  __SIGNAL_UTILS_H__
#include <signal.h>

#define     RT_SIG_FOR_APP_MIN    (SIGRTMIN+10)
#define     RT_SIG_FOR_APP_MAX    (SIGRTMAX-10)
int get_a_free_sig_and_register_proc(void *sig_handler);
int restore_sig_default_proc(int sig);
int ignore_sig(int sig);
int register_sig_proc(int sig, void *sig_handler);

#endif

