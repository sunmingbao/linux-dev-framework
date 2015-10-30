/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include "symbol_utils.h"
#include <pthread.h>
#include <pty.h>
#include <arpa/telnet.h>
#include <ctype.h>
#include <string.h>
#include "telnetd4dbg.h"
#include "socket.h"
#include "misc_utils.h"
#include "io_utils.h"
#include "string_utils.h"
#include "debug.h"

#define    DEBUG_SHELL_HINT    "[debug_shell]#"
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

int shell_thread_should_exit;
int shell_quit_occurred;
void term_session()
{
    if (fd_conn==0) return;
    
    restore_ori_io();
    shell_thread_should_exit = 1;

    while (!shell_quit_occurred) nano_sleep(0, 10000000);

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

static char shell_buf[512];
static int shell_buf_cur_len;

#define    MAX_HISTORY_CMD_NUM    (128)
typedef struct
{
    char name[512];
    int  len;
} t_history_cmd;
static t_history_cmd histotry_cmds[MAX_HISTORY_CMD_NUM];
static int cur_histotry_cmds_num;
int cur_histotry_cmds_roll_idx;


void update_histotry_cmds(char *cmd)
{
    if (cur_histotry_cmds_num==MAX_HISTORY_CMD_NUM)
    {
        memmove(histotry_cmds, &(histotry_cmds[1])
            ,sizeof(histotry_cmds[1])*(MAX_HISTORY_CMD_NUM-1));

        cur_histotry_cmds_num--;

    }

    histotry_cmds[cur_histotry_cmds_num].len = 
    sprintf(histotry_cmds[cur_histotry_cmds_num].name, "%s", cmd);
    cur_histotry_cmds_num++;
    cur_histotry_cmds_roll_idx = cur_histotry_cmds_num;

}

static void refresh_shell_buf_display()
{
    printf_to_fd(1, "\r%s", DEBUG_SHELL_HINT);
    write_certain_bytes(1, shell_buf, shell_buf_cur_len);
}

static void back_input(int nr_back_step)
{
    int i;
    printf_to_fd(1, "\r%s", DEBUG_SHELL_HINT);
    for (i=0;i<shell_buf_cur_len;i++)
        printf_to_fd(1, " ");

    shell_buf_cur_len -= nr_back_step;
    refresh_shell_buf_display();
}

void history_cmd_roll_prev()
{
    if (cur_histotry_cmds_num>0 && cur_histotry_cmds_roll_idx>0)
    {
        cur_histotry_cmds_roll_idx--;
        strcpy(shell_buf, histotry_cmds[cur_histotry_cmds_roll_idx].name);
        if (histotry_cmds[cur_histotry_cmds_roll_idx].len<shell_buf_cur_len)
            back_input(shell_buf_cur_len -
            histotry_cmds[cur_histotry_cmds_roll_idx].len);

         shell_buf_cur_len = histotry_cmds[cur_histotry_cmds_roll_idx].len;
         refresh_shell_buf_display();
    }
}

void history_cmd_roll_next()
{
    if (cur_histotry_cmds_num>0 && cur_histotry_cmds_roll_idx<(cur_histotry_cmds_num-1))
    {
        cur_histotry_cmds_roll_idx++;
        strcpy(shell_buf, histotry_cmds[cur_histotry_cmds_roll_idx].name);
        if (histotry_cmds[cur_histotry_cmds_roll_idx].len<shell_buf_cur_len)
            back_input(shell_buf_cur_len -
            histotry_cmds[cur_histotry_cmds_roll_idx].len);

         shell_buf_cur_len = histotry_cmds[cur_histotry_cmds_roll_idx].len;
         refresh_shell_buf_display();
    }
}

int read_input(int is_password)
{
    char c, two_byes[2];
    int ret;

READ_BYTE:
    if (!fd_readable(fd_pty_slave, 0, 50000))
            return 0;
    ret=read_reliable(fd_pty_slave, &c, 1);
    if (ret<0) return -1;
    if (ret!=1) goto READ_BYTE;
    if (c==0x00 || c==0x0a) goto READ_BYTE;
    if (c==0x08 && shell_buf_cur_len>0)
    {
        back_input(1);
        goto READ_BYTE;
    }

    if (c==0x1b)
    {
        ret=read_certain_bytes(fd_pty_slave, two_byes, 2);
        if (ret<0) return -1;
        if (two_byes[0]==0x5b && two_byes[1]==0x41)
            history_cmd_roll_prev();
        else if (two_byes[0]==0x5b && two_byes[1]==0x42)
            history_cmd_roll_next();

        goto READ_BYTE;

    }

    if (c==0x0d) goto EXIT;

    if (c!=' ' && !isgraph(c)) goto READ_BYTE;

        shell_buf[shell_buf_cur_len++]=c;
        if (is_password) c='*';
        printf_to_fd(fd_pty_slave, "%c", c);
        goto READ_BYTE;


EXIT:
    printf_to_fd(fd_pty_slave, "\n");
    shell_buf[shell_buf_cur_len] = 0;
    return 1;
}

void print_hint()
{
    printf_to_fd(fd_pty_slave, DEBUG_SHELL_HINT, strlen(DEBUG_SHELL_HINT));
}

static void print_intro()
{
    printf_to_fd(fd_pty_slave, "\n ****debug_shell started****\n"
        "you can input var names to see var info\n"
        "you can input d(addrress, len) to see memory contents\n"
        "you can input xxx(1, 0x2, \"abc\") to execute function xxx\n"
        "caution: every args's size of function xxx must == sizeof(long)\n");
}

static pthread_t the_shell_thread;
int login_auth()
{
    char user_name[16]={0};
    char password[16]={0};
    int ret;

    printf_to_fd(fd_pty_slave, "login:");
READ_USER_NAME:
    
    ret=read_input(0);
    if (ret<0) return -1;
    if (ret!=1) goto READ_USER_NAME;
    memcpy(user_name, shell_buf, 15);
    shell_buf_cur_len = 0;

    printf_to_fd(fd_pty_slave, "password:");
READ_PASSWORD:
    ret=read_input(1);
    if (ret<0) return -1;
    if (ret!=1) goto READ_PASSWORD;
    memcpy(password, shell_buf, 15);
    shell_buf_cur_len = 0;

    if (strcmp(user_name, "admin")==0 &&
        strcmp(user_name, "admin")==0)
    {
        printf_to_fd(fd_pty_slave, "login success\n");
        return 0;
    }

          printf_to_fd(fd_pty_slave, "username or password wrong\n");
        return 1;
}
static void *the_shell_thread_func(void *arg)
{
    int ret;
    char clean_cmd_line[512];
    cur_histotry_cmds_num = 0;
    shell_buf_cur_len=0;

    print_intro();

    if (login_auth()) goto EXIT;
    
    redirect_io(fd_pty_slave);
    print_hint();

    while (!shell_thread_should_exit)
    {
        ret=read_input(0);
        if (ret<0) goto EXIT;
        if (ret==1)
        {
            if (shell_buf_cur_len==0)
                goto CMD_OVER;
            
            update_histotry_cmds(shell_buf);
            shell_buf_cur_len=0;
            str_trim_all(clean_cmd_line, shell_buf);
            //printf_to_fd(ori_std_output, "== ret=%s\n", shell_buf);
            if (strlen(clean_cmd_line)==0)
                goto CMD_OVER;

            if (strcmp(clean_cmd_line,"quit")==0)
                goto EXIT;

            proccess_cmd(clean_cmd_line);
            printf_to_fd(1, "\n");
CMD_OVER:
            print_hint();
        }
    }

EXIT:
    shell_quit_occurred = 1;
    return NULL;
}

static char pty2sock_cache[512], *cur_snd_ptr;
static int pty2sock_cache_len;

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
        return ret;
    }

    term_session();

    pty2sock_cache_len = 0;

    fd_conn = new_sock_fd;
    set_fd_nonblock(fd_conn);
    fd_pty_master = sv[0];
    fd_pty_slave  = sv[1];

    shell_thread_should_exit = 0;
    pthread_create(&the_shell_thread, NULL, the_shell_thread_func, NULL);
    return 0;
}

static void remove_iacs(unsigned char *ptr0, int len, int *pnum_totty)
{
    unsigned char *ptr = ptr0;
    unsigned char *totty = ptr;
    unsigned char *end = ptr + len;
     
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
    int ret;
    
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

static int trans_data_pty2sock()
{
    char buf[256];
    int ret;
    static int send_fail_cnt;

    if (pty2sock_cache_len>0) goto DO_SEND_DATA;

    cur_snd_ptr = pty2sock_cache;
    pty2sock_cache_len = 0;

    ret=read_reliable(fd_pty_master, buf, sizeof(buf));
    if (ret<0)
    {
        return ret;
    }

    buf[ret]=0;
    //printf_to_fd(ori_std_output, "== ret=%d\n", ret);
    ret=str_replace_substr(pty2sock_cache, buf, "\n", "\r\n");
    //printf_to_fd(ori_std_output, "77 ret=%d\n", ret);
    cur_snd_ptr = pty2sock_cache;
    pty2sock_cache_len = ret;
    if (ret==0) return 0;

DO_SEND_DATA:
    ret=write_reliable(fd_conn, pty2sock_cache, pty2sock_cache_len);
    //printf_to_fd(ori_std_output, "write ret=%d\n", ret);
    if (ret<0)
    {
        //printf_to_fd(ori_std_output, "%s", strerror(errno));
        return ret;
    }
    if (ret==0)
    {
        send_fail_cnt++;
        if (send_fail_cnt>=100)
        {
            send_fail_cnt = 0;
            return -1;
        }

        return 0;

    }

    send_fail_cnt = 0;
    pty2sock_cache_len -= ret;
    cur_snd_ptr+=ret;

    return 0;
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
        if (shell_quit_occurred)
            term_session();

        FD_ZERO(&r_fds);
        FD_ZERO(&w_fds);
        FD_ZERO(&except_fds);

        FD_SET(fd_server, &r_fds);
        max_fd=fd_server;



        if (fd_conn>0)
        {
            FD_SET(fd_conn, &r_fds);
            FD_SET(fd_conn, &except_fds);
            max_fd=(fd_conn>max_fd)?fd_conn:max_fd;

            FD_SET(fd_pty_master, &r_fds);
            max_fd=(fd_pty_master>max_fd)?fd_pty_master:max_fd;
        }

        tv.tv_sec = 0;
        tv.tv_usec = 200000;

        retval = select(max_fd + 1, &r_fds, &w_fds, &except_fds, &tv);
        if (retval <= 0)
        {
            continue;
        }

        if (FD_ISSET(fd_server, &r_fds))
        {
            tmp_fd=accept(fd_server, NULL, NULL);

            if (tmp_fd<0)
            {
                continue;
            }

            if (make_new_session(tmp_fd))
            {
                close(tmp_fd);
            }
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

        if (FD_ISSET(fd_pty_master, &r_fds)|| (pty2sock_cache_len>0))
        {
            if (trans_data_pty2sock())
            {
                    term_session();
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

