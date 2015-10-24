/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */


#include <pthread.h>
#include <pty.h>
#include <arpa/telnet.h>
#include <ctype.h>

#include "telnetd4dbg.h"
#include "socket.h"
#include "misc_utils.h"
#include "io_utils.h"
#include "debug.h"

#define    DEBUG_SHELL_HINT    "[debug_shell]"
static uint16_t server_port;
static pthread_t misc_thread;
static int ori_std_input, ori_std_output, ori_std_err;
static int fd_server, fd_conn, fd_pty_master, fd_pty_slave;
static void save_ori_io()
{
    ori_std_input =dup(0);
    ori_std_output=dup(1);
    ori_std_err   =dup(2);
}

static void restore_ori_io()
{
    dup2(ori_std_input, 0);
    dup2(ori_std_output,1);
    dup2(ori_std_err,2);
}

static void redirect_io(int fd)
{
    dup2(fd, 0);
    dup2(fd,1);
    dup2(fd,2);
}

int shell_thread_exited = 1;
int shell_quit_occurred;
void term_session()
{
    if (fd_conn==0) return;
    
    restore_ori_io();
    while (!shell_thread_exited) nano_sleep(0, 10000000);
    close(fd_conn);
    close(fd_pty_slave);
    close(fd_pty_master);
    fd_conn=0;
    fd_pty_slave=0;
    fd_pty_master=0;
    shell_quit_occurred = 0;
}

void snd_iac(int sockfd, char cmd, char opt)
{
    char data[3] = {IAC, cmd, opt};
    write_certain_bytes(sockfd, data, sizeof(data));
}

static int tty_cfg(int fd)
{ 
    struct termios options;
    
    if  ( tcgetattr( fd,&options)  !=  0) { 
        DBG_PRINT("get tty cfg fail");     
        return -1;  
    }


    options.c_cc[VERASE] =  '\b';
    tcflush(fd,TCIFLUSH);
    options.c_cc[VMIN] = 0; /* Update the options and do it NOW */
    if (tcsetattr(fd,TCSANOW,&options) != 0)   
    { 
        DBG_PRINT("set tty fail");   
        return -1;  
    } 
    
    return 0;  
}

static pthread_t hehe;
static void back_input(char *buf, int cur_len, int nr_back_step)
{
    int i;
    printf_to_fd(1, "\r%s", DEBUG_SHELL_HINT);
    for (i=0;i<cur_len;i++)
        printf_to_fd(1, " ");

    printf_to_fd(1, "\r%s", DEBUG_SHELL_HINT);
    write_certain_bytes(1, buf, cur_len-nr_back_step);
}

int read_input(char *buf, int size, int is_password)
{
    char c, two_byes[2];
    int ret, cur_len=0;

READ_BYTE:
    ret=read_reliable(0, &c, 1);
    if (ret!=1) goto READ_BYTE;
    if (c==0x00 || c==0x0a) goto READ_BYTE;
    if (c==0x08 && cur_len>0)
    {
        back_input(buf, cur_len, 1);
        cur_len--;
        goto READ_BYTE;
    }

    if (c==0x1b)
    {
        read_certain_bytes(0, two_byes, 2);
        if (two_byes[0]==0x5b && two_byes[1]==0x41)
            ;//history_cmd_prev
        else if (two_byes[0]==0x5b && two_byes[1]==0x42)
            ;//history_cmd_next

        goto READ_BYTE;

    }

    if (c==0x0d) goto EXIT;

    if (c!=' ' && !isgraph(c)) goto READ_BYTE;

        buf[cur_len++]=c;
        printf_to_fd(1, "%c", c);
        goto READ_BYTE;


EXIT:
    printf_to_fd(1, "\n");
    return cur_len;
}

void print_hint()
{
    printf_to_fd(1, DEBUG_SHELL_HINT, strlen(DEBUG_SHELL_HINT));
}

static void *the_shell_thread(void *arg)
{
    int ret;
    char buf[512];
    
    while (1)
    {
        print_hint();
        if ((ret=read_input(buf, sizeof(buf), 0))>0)
        {
            printf_to_fd(1, "got %d bytes", ret);
        }
        printf_to_fd(1, "\n");
    }


    return NULL;
}

int make_new_session(int new_sock_fd)
{
    int ret, sv[2], use_socket_pair=1;
    set_useful_sock_opt(new_sock_fd);
    snd_iac(new_sock_fd,DONT, TELOPT_ECHO);
    snd_iac(new_sock_fd,DO, TELOPT_LFLOW);
    snd_iac(new_sock_fd,WILL, TELOPT_ECHO);
    snd_iac(new_sock_fd,WILL, TELOPT_SGA);

    if (use_socket_pair)
    {
        ret=socketpair(AF_UNIX, SOCK_STREAM, 0, sv);
    }
    else
    {
        ret=openpty(&(sv[0]), &(sv[1]), NULL, NULL, NULL);
        tty_cfg(fd_pty_slave);
    }


    if (ret<0)
    {
        ERR_DBG_PRINT("make_new_session failed.");
        return ret;
    }

    term_session();
    
    fd_conn = new_sock_fd;
    fd_pty_master = sv[0];
    fd_pty_slave  = sv[1];

    redirect_io(fd_pty_slave);
    pthread_create(&hehe, NULL, the_shell_thread, NULL);
    return 0;
}

static void remove_iacs(unsigned char *ptr0, int len, int *pnum_totty)
{
    unsigned char *ptr = ptr0;
    unsigned char *totty = ptr;
    unsigned char *end = ptr + len;
    int num_totty;
     
    while (ptr < end)
    {
 
        if (*ptr != IAC) 
        {
            char c = *ptr;
            *totty++ = c;
            ptr++;
            continue;
        } 

 
        if (ptr[1] == SB && ptr[2] == TELOPT_NAWS) 
        {
 
            ptr += 9;
            continue;
 
        } 
 

        ptr += 3;
     
    }
 
    *pnum_totty = totty - ptr0;
}

static void trans_data_sock2pty()
{
    char buf[512];
    int ret, i;
    
    ret=read_reliable(fd_conn, buf, sizeof(buf));
//    DBG_PRINT("ret=%d", ret);
    if (ret<=0)
    {
        term_session();
        return;
    }

    remove_iacs((void *)buf, ret, &ret);
    if (ret>0)
        write_certain_bytes(fd_pty_master, buf, ret);
    //for (i=0;i<ret;i++)
    //    DBG_PRINT("%02hhx-%c", buf[i], buf[i]);
}

static void trans_data_pty2sock()
{
    char buf[512];
    int ret, i;
    
    ret=read_reliable(fd_pty_master, buf, sizeof(buf));
    if (ret<=0)
    {
        term_session();
        return;
    }

        write_certain_bytes(fd_conn, buf, ret);
}

static void *misc_thread_func(void *arg)
{
    fd_set r_fds,w_fds,except_fds;
    struct timeval tv;
    int retval, max_fd, tmp_fd;

    save_ori_io();
    fd_server=tcp_socket_init(NULL, server_port);
    if (fd_server<0)
    {
        ERR_DBG_PRINT_QUIT("create telnetd server socket failed.");
    }
    
    listen(fd_server,0);

    while (1)
    {

        FD_ZERO(&r_fds);
        FD_ZERO(&w_fds);
        FD_ZERO(&except_fds);

        FD_SET(fd_server, &r_fds);
        max_fd=fd_server;

        if (shell_quit_occurred)
            term_session();

        if (fd_conn>0)
        {
            FD_SET(fd_conn, &r_fds);
            FD_SET(fd_conn, &except_fds);
            max_fd=(fd_conn>max_fd)?fd_conn:max_fd;

            FD_SET(fd_pty_master, &r_fds);
            max_fd=(fd_pty_master>max_fd)?fd_pty_master:max_fd;
        }

        tv.tv_sec = 2;
        tv.tv_usec = 0;

        retval = select(max_fd + 1, &r_fds, &w_fds, &except_fds, &tv);
        if (retval <= 0)
        {
            continue;
        }

        if (FD_ISSET(fd_conn, &except_fds))
        {
            term_session();
            continue;
        }

        if (FD_ISSET(fd_conn, &r_fds))
        {
            trans_data_sock2pty();
        }

        if (FD_ISSET(fd_pty_master, &r_fds))
        {
            trans_data_pty2sock();
        
        }


        if (FD_ISSET(fd_server, &r_fds))
        {
            tmp_fd=accept(fd_server, NULL, NULL);
            if (tmp_fd<0)
            {
                DBG_PRINT("accept telnetd client failed.");
                continue;
            }

            if (make_new_session(tmp_fd))
            {
                close(tmp_fd);
                continue;
            }
        }

    }

    return NULL;
}

void telnetd4dbg_init(uint16_t port)
{
    server_port=port;
    pthread_create(&misc_thread, NULL, misc_thread_func, NULL);
}

