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
#include "debug.h"


typedef struct
{
	int no_verbose;
	uint8_t cpu_idx;
	const char *src_ip;
	uint16_t    src_port;
	const char *dst_ip;
	uint16_t    dst_port;
	struct sockaddr_in  dst_sock_addr;
	int enable_perf;
	int no_prt_pkt;
	int snd_interval;
	int snd_data_len;
	int need_exit;
	int sockfd;
    char buf[65536];
	char snd_buf[65536];

} t_working_paras;

#define    MAX_SND_DATA_LEN    (65535-20-8)
static t_working_paras the_working_paras = 
{
	.cpu_idx = -1,
	.snd_interval = 1,
	.snd_data_len = 8,
	.snd_buf = {'1', '2', '3', '4', '5', 'a', 'b', 'c'},
	
		
};

#define    LONG_OPT_ID_HELP                  (1000)
#define    LONG_OPT_ID_NO_VERBOSE            (1001)
#define    LONG_OPT_ID_BIND_TO_CPU           (1002)
#define    LONG_OPT_ID_SRC_IP                (1003)
#define    LONG_OPT_ID_SRC_PORT              (1004)
#define    LONG_OPT_ID_SND_INTERVAL          (1005)
#define    LONG_OPT_ID_ENABLE_PERF           (1006)
#define    LONG_OPT_ID_NO_PRINT_PKT          (1007)
#define    LONG_OPT_ID_DST_IP                (1008)
#define    LONG_OPT_ID_DST_PORT              (1009)
#define    LONG_OPT_ID_SND_DATA_LEN          (1010)


typedef struct
{
    uint64_t rx_times_total;
    uint64_t rx_times_fail;
    uint64_t rx_times_succ;
    uint64_t rx_bytes;
	

	uint64_t tx_pkts_total;
	uint64_t tx_bytes_total;
	uint64_t tx_pkts_fail;
	uint64_t tx_bytes_fail;
	uint64_t tx_pkts_succ;
	uint64_t tx_bytes_succ;

} t_rxtx_stat;

static const char *stat_item_names[] = 
{
	"rx_times_total",
	"rx_times_fail",
	"rx_times_succ",
	"rx_bytes",
	
	"tx_pkts_total",
	"tx_bytes_total",
	"tx_pkts_fail",
	"tx_bytes_fail",
	"tx_pkts_succ",
	"tx_bytes_succ",

};

#define    INC_STAT(x)   ((x)++)
#define    INC_STAT_VALUE(x, v)   ((x)+=(v))

static t_rxtx_stat the_stat_data;

static struct option my_options[] =
{
    {"help",         no_argument,       NULL, LONG_OPT_ID_HELP},
    {"no-verbose",   no_argument,       NULL, LONG_OPT_ID_NO_VERBOSE},
    {"bind-to-cpu",  required_argument, NULL, LONG_OPT_ID_BIND_TO_CPU},
    {"src-ip",       required_argument, NULL, LONG_OPT_ID_SRC_IP},
    {"src-port",     required_argument, NULL, LONG_OPT_ID_SRC_PORT},
    {"snd-interval", required_argument, NULL, LONG_OPT_ID_SND_INTERVAL},
    {"enable-perf",  no_argument,       NULL, LONG_OPT_ID_ENABLE_PERF},
    {"no-print-pkt", no_argument,       NULL, LONG_OPT_ID_NO_PRINT_PKT},
	{"dst-ip",		 required_argument, NULL, LONG_OPT_ID_DST_IP},
	{"dst-port",	 required_argument, NULL, LONG_OPT_ID_DST_PORT},
	{"snd-data-len", required_argument, NULL, LONG_OPT_ID_SND_DATA_LEN},

    {0},
};


static void args_post_proc()
{
    if (the_working_paras.enable_perf)
	{
		DBG_PRINT("Perf enabled. All printf in rxtx will be shutdown");
		the_working_paras.no_verbose = 1;
		the_working_paras.no_prt_pkt = 1;

	}

	if (!the_working_paras.dst_ip || !the_working_paras.dst_port)
	{

	    DBG_PRINT_QUIT("--dst-ip  and --dst-port must be provided.");

	}

}

static void parse_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt_long(argc, argv, "hH", my_options, NULL)) != -1)
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
               break;
               
           case LONG_OPT_ID_DST_IP: 
               the_working_paras.dst_ip = optarg;
               break;

           case LONG_OPT_ID_NO_VERBOSE:
               the_working_paras.no_verbose = 1;
			   DBG_PRINT("no verbose enabled");
               break;

           case LONG_OPT_ID_NO_PRINT_PKT:
               the_working_paras.no_prt_pkt = 1;
			   DBG_PRINT("packet contents will not be printed");
               break;

           case LONG_OPT_ID_SND_INTERVAL:
               the_working_paras.snd_interval = strtoul(optarg, NULL, 0);
			   DBG_PRINT("send interval %d seconds", the_working_paras.snd_interval);
               break;

           case LONG_OPT_ID_SND_DATA_LEN:
               the_working_paras.snd_data_len = strtoul(optarg, NULL, 0);
			   DBG_PRINT("send data len %d bytes", the_working_paras.snd_data_len);
               break;

           case LONG_OPT_ID_ENABLE_PERF:
               the_working_paras.enable_perf = 1;
               break;
			   
		   case LONG_OPT_ID_BIND_TO_CPU: 
			   the_working_paras.cpu_idx = strtoul(optarg, NULL, 0);
			   DBG_PRINT("we will run on cpu %d", (int)the_working_paras.cpu_idx);
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


static void rx_loop()
{
    int ret;
	
	while (!the_working_paras.need_exit)
	{
	    ret=fd_readable(the_working_paras.sockfd, 0, 10000);
	    if (ret<=0)
    	{
    	    if (ret<0 && !the_working_paras.no_verbose)
    	        ERR_DBG_PRINT("test fd failed");

			continue;
    	}

        ret=read(the_working_paras.sockfd
			,the_working_paras.buf
			,sizeof(the_working_paras.buf));
		INC_STAT(the_stat_data.rx_times_total);

		if (ret>0)
		{
			INC_STAT(the_stat_data.rx_times_succ);
			INC_STAT_VALUE(the_stat_data.rx_bytes, ret);

			if (!the_working_paras.no_verbose)
			{
			    printf("[%"PRIu64"]got %d bytes\n"
					,the_stat_data.rx_times_succ
					,ret);

                if (!the_working_paras.no_prt_pkt)
            	{
					printf("the data contents is:\n");
					print_mem(the_working_paras.buf, ret);
            	}
			}

		}
		else
		{
			INC_STAT(the_stat_data.rx_times_fail);

			if (!the_working_paras.no_verbose)
			    ERR_DBG_PRINT("rx failed");

			break;
		}
		
		printf("\n\n");
		continue;

	}
}


static void sig_handler(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{
    int ret;
    if (SIGINT==sig_no)
	{
		the_working_paras.need_exit = 1;
		alarm(0);
	}
	else if (SIGALRM==sig_no)
	{
	    alarm(the_working_paras.snd_interval);
		
		INC_STAT(the_stat_data.tx_pkts_total);
		INC_STAT_VALUE(the_stat_data.tx_bytes_total, the_working_paras.snd_data_len);

		if (!the_working_paras.no_verbose)
		{
			printf("[%"PRIu64"]try send %d bytes to %s:%d\n"
				,the_stat_data.tx_pkts_total
				,the_working_paras.snd_data_len
				,the_working_paras.dst_ip
				,(int)the_working_paras.dst_port);

		}
		ret=write_certain_bytes(the_working_paras.sockfd
			, the_working_paras.snd_buf
			, the_working_paras.snd_data_len);

		if (0==ret)
		{
			INC_STAT(the_stat_data.tx_pkts_succ);
			INC_STAT_VALUE(the_stat_data.tx_bytes_succ, the_working_paras.snd_data_len);
		}
		else
		{
			INC_STAT(the_stat_data.tx_pkts_fail);
			INC_STAT_VALUE(the_stat_data.tx_bytes_fail, the_working_paras.snd_data_len);

			if (!the_working_paras.no_verbose)
				ERR_DBG_PRINT("tx failed");
			
			the_working_paras.need_exit = 1;
			alarm(0);

		}

	}
}


static void show_stats()
{
	print_stats(1, (void *)(&the_stat_data), stat_item_names, ARRAY_SIZE(stat_item_names));
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

	
    make_sockaddr(&the_working_paras.dst_sock_addr, inet_addr(the_working_paras.dst_ip), htons(the_working_paras.dst_port));
	ret=connect(the_working_paras.sockfd
		,(void *)&the_working_paras.dst_sock_addr
		,sizeof(struct sockaddr_in));
	if (ret<0)
	{
	    ERR_DBG_PRINT_QUIT("connect to %s:%d failed ", the_working_paras.dst_ip, (int)the_working_paras.dst_port);
	}

    set_fd_nonblock(the_working_paras.sockfd);
	DBG_PRINT("connect to %s:%d succeed!", the_working_paras.dst_ip, (int)the_working_paras.dst_port);

}

int main(int argc, char *argv[])
{
    int ret;
    parse_args(argc, argv);
    if (the_working_paras.cpu_idx!=(uint8_t)-1)
	{
		ret = bind_cur_thread_to_cpu(the_working_paras.cpu_idx);
		if (0==ret)
			goto BIND_CPU_OK;
		
		errno = ret;
		ERR_DBG_PRINT_QUIT("bind to cpu %d failed", (int)the_working_paras.cpu_idx);
	}

BIND_CPU_OK:
	register_sig_proc(SIGINT, sig_handler);
	register_sig_proc(SIGALRM, sig_handler);
    connect2dst();
	alarm(the_working_paras.snd_interval);

	rx_loop();
	show_stats();
    return 0;
}

