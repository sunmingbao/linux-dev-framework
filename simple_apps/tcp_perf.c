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
#include <stdint.h>
#include <getopt.h>
#include <stdlib.h>
#include "common.h"
#include "sys_utils.h"
#include "io_utils.h"
#include "signal_utils.h"
#include "socket.h"
#include "screen_ops.h"
#include "stat.h"
#include "debug.h"


typedef struct
{
	volatile int need_exit;

	int is_server_mode;

	int cpu_idx;
	int cpu_idx_r;
	int cpu_idx_s;
pthread_t rx_thread;
pthread_t tx_thread;

	const char *src_ip;
	uint16_t    src_port;
	const char *dst_ip;
	uint16_t    dst_port;
	struct sockaddr_in  dst_sock_addr;
	int snd_data;
	int snd_data_len;

	int rcv_data;
	int rcv_data_len;
	int sockfd_listen;
	int sockfd;
    char buf[65536];
	char snd_buf[65536];

} t_working_paras;

#define    DFT_RX_TX_DATA_LEN     (1500-20-20)
#define    STAT_PRINT_INTERVAL    (1)

static t_working_paras the_working_paras = 
{
	.is_server_mode = -1,
	.cpu_idx = -1,
	.cpu_idx_r = -1,
	.cpu_idx_s = -1,
	.sockfd = -1,
	.snd_data_len = DFT_RX_TX_DATA_LEN,
	.rcv_data_len = DFT_RX_TX_DATA_LEN,
	.snd_buf = {'1', '2', '3', '4', '5', 'a', 'b', 'c'},
};

#define    LONG_OPT_ID_HELP                  (1000)
#define    LONG_OPT_ID_BIND_TO_CPU           (1002)

#define    LONG_OPT_ID_SRC_IP                (1003)
#define    LONG_OPT_ID_SRC_PORT              (1004)

#define    LONG_OPT_ID_BIND_RX_TO_CPU           (1005)
#define    LONG_OPT_ID_BIND_TX_TO_CPU           (1006)

#define    LONG_OPT_ID_SND_DATA              (1007)
#define    LONG_OPT_ID_RCV_DATA              (1012)
#define    LONG_OPT_ID_DST_IP                (1008)
#define    LONG_OPT_ID_DST_PORT              (1009)
#define    LONG_OPT_ID_SND_DATA_LEN          (1010)
#define    LONG_OPT_ID_RCV_DATA_LEN          (1011)

typedef struct
{
	uint64_t tx_total;
	uint64_t tx_bytes_total;
	uint64_t tx_fail;
	uint64_t tx_bytes_fail;
	uint64_t tx_succ;
	uint64_t tx_bytes_succ;
	uint64_t tx_succ_part;
	uint64_t tx_bytes_succ_part;
	uint64_t tx_succ_sum;
	uint64_t tx_bytes_succ_sum;

	uint64_t rx_total;
	uint64_t rx_bytes_total;
	uint64_t rx_fail;
	uint64_t rx_bytes_fail;
	uint64_t rx_succ;
	uint64_t rx_bytes_succ;
	uint64_t rx_succ_part;
	uint64_t rx_bytes_succ_part;
	uint64_t rx_succ_sum;
	uint64_t rx_bytes_succ_sum;
} t_rxtx_stat;

static const char *stat_item_names[] = 
{
	
	"tx_total",
	"tx_bytes_total",
	"tx_fail",
	"tx_bytes_fail",
	"tx_succ",
	"tx_bytes_succ",
	"tx_succ_part",
	"tx_bytes_succ_part",
	"tx_succ_sum",
	"tx_bytes_succ_sum",

	"rx_total",
	"rx_bytes_total",
	"rx_fail",
	"rx_bytes_fail",
	"rx_succ",
	"rx_bytes_succ",
	"rx_succ_part",
	"rx_bytes_succ_part",
	"rx_succ_sum",
	"rx_bytes_succ_sum",
};

#define    INC_STAT(x)   ((x)++)
#define    INC_STAT_VALUE(x, v)   ((x)+=(v))

static t_rxtx_stat the_stat_data[4];

static struct option my_options[] =
{
	{"help",         no_argument,       NULL, LONG_OPT_ID_HELP},
	{"bind-to-cpu",  required_argument, NULL, LONG_OPT_ID_BIND_TO_CPU},
	{"bind-rx-to-cpu",  required_argument, NULL, LONG_OPT_ID_BIND_RX_TO_CPU},
	{"bind-tx-to-cpu",  required_argument, NULL, LONG_OPT_ID_BIND_TX_TO_CPU},
	{"src-ip",       required_argument, NULL, LONG_OPT_ID_SRC_IP},
	{"src-port",     required_argument, NULL, LONG_OPT_ID_SRC_PORT},
	{"snd-data", no_argument,       NULL, LONG_OPT_ID_SND_DATA},
	{"rcv-data", no_argument,       NULL, LONG_OPT_ID_RCV_DATA},
	{"dst-ip",		 required_argument, NULL, LONG_OPT_ID_DST_IP},
	{"dst-port",	 required_argument, NULL, LONG_OPT_ID_DST_PORT},
	{"snd-data-len", required_argument, NULL, LONG_OPT_ID_SND_DATA_LEN},
	{"rcv-data-len", required_argument, NULL, LONG_OPT_ID_RCV_DATA_LEN},
	{0},
};


static void args_post_proc()
{
	if (the_working_paras.is_server_mode == -1)
	{

	    DBG_PRINT_QUIT("server mode(-S or -s) or client mode(-C or -c) must be specified.");

	}

	if (!the_working_paras.is_server_mode && (!the_working_paras.dst_ip || !the_working_paras.dst_port))
	{

	    DBG_PRINT_QUIT("--dst-ip  and --dst-port must be provided in client mode.");

	}

	if (the_working_paras.is_server_mode && (!the_working_paras.src_port))
	{

	    DBG_PRINT_QUIT("--src-port must be provided in server mode.");

	}
}

static void parse_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt_long(argc, argv, "hHSsCc", my_options, NULL)) != -1)
    {
       switch (opt)
       {
               
           case LONG_OPT_ID_SRC_PORT: 
               the_working_paras.src_port = strtoul(optarg, NULL, 0);
               break;
               
           case LONG_OPT_ID_SRC_IP: 
               the_working_paras.src_ip = optarg;
               break;

           case LONG_OPT_ID_DST_PORT: 
               the_working_paras.dst_port = strtoul(optarg, NULL, 0);
DBG_PRINT("--dst-port = %d",  (int)the_working_paras.dst_port);
               break;
               
           case LONG_OPT_ID_DST_IP: 
               the_working_paras.dst_ip = optarg;
	DBG_PRINT("--dst-ip = %s",  the_working_paras.dst_ip);
               break;


           case LONG_OPT_ID_SND_DATA:
               the_working_paras.snd_data= 1;
			   DBG_PRINT("send data enabled");
               break;

           case LONG_OPT_ID_RCV_DATA:
               the_working_paras.rcv_data= 1;
			   DBG_PRINT("send data enabled");
               break;

           case LONG_OPT_ID_SND_DATA_LEN:
               the_working_paras.snd_data_len = strtoul(optarg, NULL, 0);
			   DBG_PRINT("send data block len %d bytes", the_working_paras.snd_data_len);
               break;

           case LONG_OPT_ID_RCV_DATA_LEN:
               the_working_paras.rcv_data_len = strtoul(optarg, NULL, 0);
			   DBG_PRINT("rcv data block len %d bytes", the_working_paras.snd_data_len);
               break;
	   
	   case LONG_OPT_ID_BIND_TO_CPU: 
		   the_working_paras.cpu_idx = strtoul(optarg, NULL, 0);
		   DBG_PRINT("we will run on cpu %d", (int)the_working_paras.cpu_idx);
		   break;

	   case LONG_OPT_ID_BIND_RX_TO_CPU: 
		   the_working_paras.cpu_idx_r= strtoul(optarg, NULL, 0);
		   DBG_PRINT("rx run on cpu %d", (int)the_working_paras.cpu_idx_r);
		   break;

	   case LONG_OPT_ID_BIND_TX_TO_CPU: 
		   the_working_paras.cpu_idx_s= strtoul(optarg, NULL, 0);
		   DBG_PRINT("tx run on cpu %d", (int)the_working_paras.cpu_idx_s);
		   break;

	   case 'S':
	   case 's':
		the_working_paras.is_server_mode = 1;
		break;

	   case 'C':
	   case 'c':
		the_working_paras.is_server_mode = 0;
		break;

	   case 'h':
	   case 'H':
	   case LONG_OPT_ID_HELP:
           default: /* '?' */
               print_usage(argv[0]
			   	, "<--dst-ip=xxx>  <--dst-port=xxx> [--no-verbose] [--bind-to-cpu=x]"
			   	, my_options
			   	, ARRAY_SIZE(my_options)-1);
			   exit(0);
       }
    }

    args_post_proc();
}



static void show_stats(void)
{
	static t_rxtx_stat last_stat;
	t_rxtx_stat latest_stat, inc_stat;

	uint64_t * srcs[] = {
		(void *)&the_stat_data[0],
		(void *)&the_stat_data[1],
	};

	sum_stats(&latest_stat,srcs, ARRAY_SIZE(srcs) ,ARRAY_SIZE(stat_item_names));

	sub_stats(&inc_stat, &latest_stat ,&last_stat, ARRAY_SIZE(stat_item_names));
	last_stat = latest_stat;

	CLEAR_SCREEN();
	print_stats(1, (void *)(&inc_stat), stat_item_names, ARRAY_SIZE(stat_item_names));
}


static void sig_handler(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{
    int ret;
    if (SIGINT==sig_no) {
		the_working_paras.need_exit = 1;
		alarm(0);
	}
	else if (SIGALRM==sig_no) {
		alarm(STAT_PRINT_INTERVAL);
		show_stats();
		

	}
}



static void connect2dst()
{
    int ret;
	if (the_working_paras.src_ip)
        the_working_paras.sockfd = tcp_socket_init(the_working_paras.src_ip, the_working_paras.src_port);
	else
		the_working_paras.sockfd = tcp_socket_init_no_addr();
	
	if (the_working_paras.sockfd<0)
	{
	    ERR_DBG_PRINT_QUIT("create socket on %s:%d failed ", the_working_paras.src_ip, (int)the_working_paras.src_port);
	}

	
    make_sockaddr(&the_working_paras.dst_sock_addr
	, inet_addr(the_working_paras.dst_ip)
	, htons(the_working_paras.dst_port));

	ret=connect(the_working_paras.sockfd
		,(void *)&the_working_paras.dst_sock_addr
		,sizeof(struct sockaddr_in));
	if (ret<0)
	{
	    ERR_DBG_PRINT_QUIT("connect to %s:%d failed ", the_working_paras.dst_ip, (int)the_working_paras.dst_port);
	}

    //set_fd_nonblock(the_working_paras.sockfd);
	DBG_PRINT("connect to %s:%d succeed!", the_working_paras.dst_ip, (int)the_working_paras.dst_port);

}

static void snd_thread(void *para)
{
	int ret;
	t_rxtx_stat *pstat = para;
	int fd = the_working_paras.sockfd;
	void *buf = the_working_paras.snd_buf;
	int snd_len = the_working_paras.snd_data_len;

	while (!the_working_paras.need_exit) {
		
		INC_STAT(pstat->tx_total);
		INC_STAT_VALUE(pstat->tx_bytes_total, snd_len);

		ret=write(fd, buf, snd_len);

		if (ret>0) {
			INC_STAT(pstat->tx_succ_sum);
			INC_STAT_VALUE(pstat->tx_bytes_succ_sum, ret);
			if (snd_len==ret) {
				INC_STAT(pstat->tx_succ);
				INC_STAT_VALUE(pstat->tx_bytes_succ, ret);
			}
			else
			{
				INC_STAT(pstat->tx_succ_part);
				INC_STAT_VALUE(pstat->tx_bytes_succ_part, ret);
			}
		} else {
			INC_STAT(pstat->tx_fail);
			INC_STAT_VALUE(pstat->tx_bytes_fail, snd_len);

		}

	}
}

static void rcv_thread(void *para)
{
	int ret;
	t_rxtx_stat *pstat = para;
	int fd = the_working_paras.sockfd;
	void *buf = the_working_paras.buf;
	int rcv_len = the_working_paras.rcv_data_len;

	while (!the_working_paras.need_exit) {
		
		INC_STAT(pstat->rx_total);
		INC_STAT_VALUE(pstat->rx_bytes_total, rcv_len);

		ret=read(fd, buf, rcv_len);

		if (ret>0) {
			INC_STAT(pstat->rx_succ_sum);
			INC_STAT_VALUE(pstat->rx_bytes_succ_sum, ret);
			if (rcv_len==ret) {
				INC_STAT(pstat->rx_succ);
				INC_STAT_VALUE(pstat->rx_bytes_succ, ret);
			}
			else
			{
				INC_STAT(pstat->rx_succ_part);
				INC_STAT_VALUE(pstat->rx_bytes_succ_part, ret);
			}
		} else {
			INC_STAT(pstat->rx_fail);
			INC_STAT_VALUE(pstat->rx_bytes_fail, rcv_len);

		}

	}
}

static void listen_and_wait_conn(void)
{
	int ret, conn_fd;

	struct sockaddr_in  peer_addr;

	socklen_t addr_len = sizeof(peer_addr);
	const char *src_ip = the_working_paras.src_ip?the_working_paras.src_ip:"0.0.0.0";

	char peer_ip[32];
	uint16_t peer_port;

	the_working_paras.sockfd_listen = tcp_socket_init(the_working_paras.src_ip,  the_working_paras.src_port);
	if (the_working_paras.sockfd_listen<0)
	{
	    ERR_DBG_PRINT_QUIT("create server socket on %s:%d failed ", the_working_paras.src_ip, (int)the_working_paras.src_port);
	}

	set_useful_sock_opt(the_working_paras.sockfd_listen);
	ret=listen(the_working_paras.sockfd_listen, 5);
	if (ret)
		ERR_DBG_PRINT_QUIT("listen on socket failed ");
	else
		DBG_PRINT_S("start listen on fd %d", the_working_paras.sockfd_listen);

    while (-1 == the_working_paras.sockfd)
	{
	    conn_fd =  accept(the_working_paras.sockfd_listen
			, (void *)&peer_addr
			, &addr_len);
		if (conn_fd<0)
		{
		    if (EINTR==errno)
				continue;
			
			ERR_DBG_PRINT_QUIT("accept failed ");
		}

		the_working_paras.sockfd = conn_fd;

	}

	//set_fd_nonblock(the_working_paras.sockfd);
	close(the_working_paras.sockfd_listen);

resolve_sockaddr(&peer_addr, peer_ip, sizeof(peer_ip), &peer_port);
printf("client from %s:%d connected. and we only accept one client\n"
	,peer_ip
	,(int)peer_port);

}

static void start_rx_tx(void)
{
	int ret;
	int dest_cpu_r=the_working_paras.cpu_idx_r;
	int dest_cpu_s=the_working_paras.cpu_idx_s;


	if (!the_working_paras.rcv_data)
		goto rx_create_over;

	ret=create_thread_full(&the_working_paras.rx_thread, rcv_thread, &the_stat_data[0], dest_cpu_r, dest_cpu_r, SCHED_FIFO, 30);
	DBG_PRINT("==ret=%d", ret);
	if (ret)
		ERR_DBG_PRINT_QUIT("==");

rx_create_over:
	if (!the_working_paras.snd_data)
		return;

	ret=create_thread_full(&the_working_paras.tx_thread, snd_thread, &the_stat_data[1], dest_cpu_s, dest_cpu_s, SCHED_FIFO, 30);
	DBG_PRINT("==ret=%d", ret);
	if (ret)
		ERR_DBG_PRINT_QUIT("==");
}
int main(int argc, char *argv[])
{
    int ret;
    parse_args(argc, argv);
    if (the_working_paras.cpu_idx!=-1)
	{
		ret = bind_cur_thread_to_cpu(the_working_paras.cpu_idx);
		if (0==ret)
			goto BIND_CPU_OK;
		
		errno = ret;
		ERR_DBG_PRINT_QUIT("bind to cpu %d failed", (int)the_working_paras.cpu_idx);
	}

BIND_CPU_OK:
	if (the_working_paras.is_server_mode) {
		listen_and_wait_conn();
	} else {
		connect2dst();
	}

	if (!the_working_paras.snd_data && !the_working_paras.rcv_data) {
		return 0;
	}

	start_rx_tx();
	register_sig_proc(SIGINT, sig_handler);
	register_sig_proc(SIGALRM, sig_handler);
	alarm(STAT_PRINT_INTERVAL);

	while (!the_working_paras.need_exit) {
		sleep(5);
	}

	pthread_join(the_working_paras.rx_thread, NULL);

	if (the_working_paras.snd_data)
		pthread_join(the_working_paras.tx_thread, NULL);

    return 0;
}

