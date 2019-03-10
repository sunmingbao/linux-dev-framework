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
#include "stat.h"
#include "debug.h"


typedef struct
{
	int no_verbose;
	uint8_t cpu_idx;
	const char *src_ip;
	uint16_t    src_port;
	int enable_perf;
	int no_prt_pkt;
	int only_recv;
	int need_exit;
	int sockfd;
    char buf[65536];

} t_working_paras;

static t_working_paras the_working_paras = 
{
	.cpu_idx = -1,
	.src_ip  = "0.0.0.0",
	.src_port= 5555,
	
		
};

#define    LONG_OPT_ID_HELP                  (1000)
#define    LONG_OPT_ID_NO_VERBOSE            (1001)
#define    LONG_OPT_ID_BIND_TO_CPU           (1002)
#define    LONG_OPT_ID_SRC_IP                (1003)
#define    LONG_OPT_ID_SRC_PORT              (1004)
#define    LONG_OPT_ID_ONLY_RECV             (1005)
#define    LONG_OPT_ID_ENABLE_PERF           (1006)
#define    LONG_OPT_ID_NO_PRINT_PKT          (1007)


typedef struct
{
    uint64_t rx_times_total;
    uint64_t rx_times_fail;
    uint64_t rx_pkts;
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
	"rx_pkts",
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
    {"only-recv",    no_argument,       NULL, LONG_OPT_ID_ONLY_RECV},
    {"enable-perf",  no_argument,       NULL, LONG_OPT_ID_ENABLE_PERF},
    {"no-print-pkt", no_argument,       NULL, LONG_OPT_ID_NO_PRINT_PKT},
    {0},
};


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

           case LONG_OPT_ID_NO_VERBOSE:
               the_working_paras.no_verbose = 1;
			   DBG_PRINT("no verbose enabled");
               break;

           case LONG_OPT_ID_NO_PRINT_PKT:
               the_working_paras.no_prt_pkt = 1;
			   DBG_PRINT("packet contents will not be printed");
               break;

           case LONG_OPT_ID_ONLY_RECV:
               the_working_paras.only_recv= 1;
			   DBG_PRINT("only receive enabled");
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
			   	, "[--src-ip=xxx]  [--src-port=xxx] [--no-verbose] [--bind-to-cpu=x]"
			   	, my_options
			   	, ARRAY_SIZE(my_options)-1);
			   exit(0);
       }
    }

    if (the_working_paras.enable_perf)
	{
		DBG_PRINT("Perf enabled. All printf in rxtx will be shutdown");
		the_working_paras.no_verbose = 1;
		the_working_paras.no_prt_pkt = 1;

	}
}


static void rxtx_loop()
{
    int ret, send_ret;
	struct sockaddr_in  peer_addr;
    char peer_ip[32];
	uint16_t peer_port;
	while (!the_working_paras.need_exit)
	{
	    ret=fd_readable(the_working_paras.sockfd, 0, 10000);
	    if (ret<=0)
    	{
    	    if (ret<0 && !the_working_paras.no_verbose)
    	        ERR_DBG_PRINT("test fd failed");

			continue;
    	}

        ret=udp_socket_recvfrom(the_working_paras.sockfd
			,the_working_paras.buf
			,sizeof(the_working_paras.buf)
			,&peer_addr);
		INC_STAT(the_stat_data.rx_times_total);

		if (ret>0)
		{
			INC_STAT(the_stat_data.rx_pkts);
			INC_STAT_VALUE(the_stat_data.rx_bytes, ret);

			if (!the_working_paras.no_verbose)
			{
			    resolve_sockaddr(&peer_addr, peer_ip, sizeof(peer_ip), &peer_port);
			    printf("[%"PRIu64"]got %d bytes from %s:%d\n"
					,the_stat_data.rx_pkts
					,ret
					,peer_ip
					,(int)peer_port);

                if (!the_working_paras.no_prt_pkt)
            	{
					printf("the data contents is:\n");
					print_mem(the_working_paras.buf, ret);
            	}
			}
			
			if (!the_working_paras.only_recv)
			{
				if (!the_working_paras.no_verbose)
				{
				    printf("try send back %d bytes to %s:%d\n"
						,ret
						,peer_ip
						,(int)peer_port);

				}
			    send_ret=udp_socket_sendto(the_working_paras.sockfd, the_working_paras.buf, ret, &peer_addr);
				INC_STAT(the_stat_data.tx_pkts_total);
				INC_STAT_VALUE(the_stat_data.tx_bytes_total, ret);

				if (send_ret==ret)
				{
				    INC_STAT(the_stat_data.tx_pkts_succ);
			        INC_STAT_VALUE(the_stat_data.tx_bytes_succ, ret);
				}
				else if (send_ret<=0)
				{
				    INC_STAT(the_stat_data.tx_pkts_fail);
			        INC_STAT_VALUE(the_stat_data.tx_bytes_fail, ret);

					if (!the_working_paras.no_verbose)
					    ERR_DBG_PRINT("tx failed");

				}
				else
					DBG_PRINT_QUIT("abnormal event: udp packet was partly sent!");
			}

		}
		else
		{
			INC_STAT(the_stat_data.rx_times_fail);

			if (!the_working_paras.no_verbose)
			    ERR_DBG_PRINT("rx failed");
		}

		printf("\n\n");
		continue;

	}
}


static void sig_handler(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{
    if (SIGINT==sig_no)
		the_working_paras.need_exit = 1;
}


static void show_stats()
{
	print_stats(1, (void *)(&the_stat_data), stat_item_names, ARRAY_SIZE(stat_item_names));
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
    the_working_paras.sockfd = udp_socket_init(the_working_paras.src_ip, the_working_paras.src_port);
	if (the_working_paras.sockfd<0)
	{
	    ERR_DBG_PRINT_QUIT("create socket on %s:%d failed ", the_working_paras.src_ip, (int)the_working_paras.src_port);
	}

	set_fd_nonblock(the_working_paras.sockfd);
	rxtx_loop();
	show_stats();
    return 0;
}

