/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */


/* 
    代码编译后,通过如下三条命令,
    即可启动p2p_server及两个p2p_client进行p2p通信
   
 ./target/p2p_server.exe
 ./target/p2p_client.exe   --from aaa --to bbb
 ./target/p2p_client.exe   --from bbb --to aaa
 
*/

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <stdlib.h>
#include <signal.h>
#include <sys/time.h>

#include "p2p.h"
#include "socket.h"
#include "common.h"
#include "io_utils.h"
#include "misc_utils.h"
#include "timer_utils.h"
#include "debug.h"
#include "trace_exception.h"

#define    TIMER_GAP_IN_SEC    (3)

static char     server_ip[40] = "127.0.0.1";
static uint16_t server_port   = 8888;

static char usr_src[32];
static char usr_dst[32];
static char  password[32] = "95555";

static int   connected;
static int   heart_beat_miss_cnt;
static char heart_beat_data[64];

static int   sock_fd;

static struct sockaddr_in  sockaddr_server, sockaddr_peer;
static inline int addr_is_server(struct sockaddr_in *addr)
{
    return sockaddr_equal(addr, &sockaddr_server);
}

static inline int addr_is_peer(struct sockaddr_in *addr)
{
    return sockaddr_equal(addr, &sockaddr_peer);
}


static inline int send_to_peer(void *buf, int buf_size)
{
    if (connected)
    return udp_socket_sendto(sock_fd, buf, buf_size, &sockaddr_peer);

    return -1;
}

static inline int send_to_server(void *buf, int buf_size)
{
    return udp_socket_sendto(sock_fd, buf, buf_size, &sockaddr_server);
}

void  snd_heart_beat()
{
    /* we just send a string as heart beat :) */
    static uint32_t heart_beat_seq;

    if (connected)
    {
        DBG_PRINT("snd_heart_beat : %u", heart_beat_seq);
        send_to_peer(heart_beat_data, sizeof(heart_beat_data));
        heart_beat_seq++;
    }
}

void timer_proc()
{
    static int timer_cnt;

    snd_heart_beat();
    
    if (!connected) goto HEART_BEAT_OVER;
        
    heart_beat_miss_cnt++;
    if (heart_beat_miss_cnt>=5)
    {
        connected = 0;
    }

HEART_BEAT_OVER:
    
    timer_cnt+=TIMER_GAP_IN_SEC;
    if (timer_cnt>=60*60*24)
    {
        timer_cnt = 0;
        // we can do some long periords work here;
    }
}

static void sig_handler(int signo)
{
    if (SIGALRM==signo)
    {
        timer_proc();
        return;
    }
}


#define    LONG_OPT_ID_SERVER_IP      (1001)
#define    LONG_OPT_ID_SERVER_PORT    (1002)
#define    LONG_OPT_ID_FROM           (1003)
#define    LONG_OPT_ID_TO             (1004)

struct option our_options[] =
    {
        {"server-ip",    required_argument, NULL, LONG_OPT_ID_SERVER_IP},
        {"server-port",  required_argument, NULL, LONG_OPT_ID_SERVER_PORT},
        {"from",         required_argument, NULL, LONG_OPT_ID_FROM},
        {"to",           required_argument, NULL, LONG_OPT_ID_TO},

        {0},
    };

static void parse_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt_long(argc, argv, "", our_options, NULL)) != -1)
    {
       switch (opt)
       {
           case LONG_OPT_ID_SERVER_IP:
               strcpy(server_ip, optarg);
               break;

           case LONG_OPT_ID_SERVER_PORT:
               server_port = atoi(optarg);
               break;

           case LONG_OPT_ID_FROM:
               strcpy(usr_src, optarg);
               break;

           case LONG_OPT_ID_TO:
               strcpy(usr_dst, optarg);
               break;

           default:
               goto PRINT_USAGE_AND_QUIT;
       }
    }

    if (usr_src==NULL || usr_dst==NULL)  
        goto PRINT_USAGE_AND_QUIT;
    
    DBG_PRINT("%s connect to %s through server %s:%d", usr_src, usr_dst, server_ip, (int)server_port);
    return;

PRINT_USAGE_AND_QUIT:
    DBG_PRINT("Usage: %s [--server-ip ip_addr] [--server-port port] <--from lucy> <--to jim>\n",
                       argv[0]);
   exit(1);

}

void p2p_conn_loop()
{
    t_P2P_CONN_REQ  conn_req;
    struct sockaddr_in  sockaddr;
    int ret;
    int sleep_cnt = 0;
    t_P2P_CONN_RPLY reply;

    conn_req.msg_id=MSG_P2P_CONN_REQ;
    strcpy(conn_req.usr_src, usr_src);
    strcpy(conn_req.usr_dst, usr_dst);
    strcpy(conn_req.pass_word, password);

    while (!connected)
    {
        DBG_PRINT("sending connect request to p2p_server");
        send_to_server(&conn_req, sizeof(conn_req));
        sleep_cnt = 0;

WAIT_SOCK_DATA:
        DBG_PRINT("wait for p2p_server response");
        while (!fd_readable(sock_fd, 0, 0))
        {
            nano_sleep(0, 50000000);
            sleep_cnt++;
            if (sleep_cnt >= 100)
                break;
                
        }

        if (!fd_readable(sock_fd, 0, 0)) continue;
        
        ret = udp_socket_recvfrom(sock_fd, &reply, sizeof(reply), &sockaddr);
        if (ret<=0)
        {
            ERR_DBG_PRINT("udp_socket_recvfrom failed");
            goto SLEEP_FOR_NEXT;
        }
            
        DBG_PRINT_S("recved %d bytes from %s:%d", ret, get_ipstr(&sockaddr, NULL), (int)get_port(&sockaddr, NULL));

        if (!addr_is_server(&sockaddr))
        {
            DBG_PRINT_S("the data is not from p2pserver, its contens:");
            print_mem(&reply, ret);
            goto WAIT_SOCK_DATA;
        }
        
        reply.msg_id = ntohl(reply.msg_id);

        if (reply.msg_id==MSG_P2P_CONN_RESP_OK)
        {
            DBG_PRINT_S("get peer addr success =====");
            DBG_PRINT_S("our addr: %s:%d", reply.usr_src_ip, ntohs(reply.usr_src_port));
            DBG_PRINT_S("peer addr: %s:%d", reply.usr_dst_ip, ntohs(reply.usr_dst_port));
            make_sockaddr(&sockaddr_peer, inet_addr(reply.usr_dst_ip), reply.usr_dst_port);
            connected=1;
            return;
        }

        if (reply.msg_id==MSG_P2P_CONN_RESP_DST_OFFLINE)
        {
            DBG_PRINT_S("peer offline =====");
            DBG_PRINT_S("our addr: %s:%d", reply.usr_src_ip, ntohs(reply.usr_src_port));
            goto SLEEP_FOR_NEXT;
        }
        
        
        DBG_PRINT_S("unknown message format, its contens:");
        print_mem(&reply, ret);


SLEEP_FOR_NEXT:
        while (sleep_cnt < 100)
        {
            nano_sleep(0, 50000000);
            sleep_cnt++;
        }

    }

}

static void main_loop()
{
    int ret;
    struct sockaddr_in  sockaddr;
    uint8_t msg_buf[70000];

    while (1)
    {
        if (!connected)
            p2p_conn_loop();

        if (!fd_readable(sock_fd, 0, 0)) goto SLEEP_FOR_NEXT;
        
        ret = udp_socket_recvfrom(sock_fd, msg_buf,  ARRAY_SIZE(msg_buf), &sockaddr);
        if (ret<=0)
        {
            connected=0;
            continue;
        }

        if (addr_is_peer(&sockaddr)) 
        {
            heart_beat_miss_cnt=0;
            DBG_PRINT("recved %d bytes from peer", ret);
        }
        else
        {
            DBG_PRINT("recved %d bytes from unknown site:%s:%d", ret, get_ipstr(&sockaddr, NULL), (int)get_port(&sockaddr, NULL));
        }
        
        print_mem(msg_buf, ret);

SLEEP_FOR_NEXT:
        nano_sleep(0, 50000000);
    }
}

int main(int argc, char *argv[])
{
trace_exception_init();
    parse_args(argc, argv);
    
    sprintf(heart_beat_data, "heart beat: %s->%s", usr_src, usr_dst);

    make_sockaddr(&sockaddr_server, inet_addr(server_ip), htons(server_port));
    sock_fd=udp_socket_init(NULL, 0);

    if (register_sighandler(SIGALRM, sig_handler)<0)
    {
        DBG_PRINT_QUIT("register_sighandler failed");
    }

    itimer_init(TIMER_GAP_IN_SEC, 0);
    
    main_loop();
    
    return 0;
}

