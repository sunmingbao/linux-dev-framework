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
 * 本程序实现了向tap口发包的功能，支持速率控制。
 * 同时，本程序可以将tap口加入到ovs网桥中，
 * 从而可以向网桥灌包。
 *
 * 本程序使用了pcap库。若没有，需要先安装此库。
 * 本程序的编译及运行命令如下：
 * gcc tap_writer.c -lpcap -lpthread
 * ./a.out  --input-file=test.pcap --tap-name=smbtest --add-tap-to-ovs-br=ovsbr
 *
 */

#define _GNU_SOURCE
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <errno.h>
#include <assert.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <getopt.h>
#include <signal.h>
#include <ucontext.h>
#include <net/if.h>
#include <linux/if_tun.h>
#include <stdlib.h>
#include <pcap.h>


static const char *version = "1.0.2";
static const char *app_name = "tap_writer";


#define    DBG_PRINT(fmt, args...) \
    do \
    { \
        printf("DBG:%s(%d)-%s:\n"fmt"\n", __FILE__,__LINE__,__FUNCTION__,##args); \
    } while (0)


#define    ERR_DBG_PRINT(fmt, args...) \
    do \
    { \
        printf("ERR_DBG:%s(%d)-%s:\n"fmt": %s\n", __FILE__,__LINE__,__FUNCTION__,##args, strerror(errno)); \
    } while (0)
    
#define    ARRAY_SIZE(array_name)    (sizeof(array_name)/sizeof(array_name[0]))


#define    MAX_PACKET_NUM    (128)


#define    MAX_ETH_DATA_LEN    (8192)
#define    MAX_ETH_PKT_LEN     (MAX_ETH_DATA_LEN+14)
typedef struct
{
    uint32_t           pkt_len;
    unsigned char      dst_mac[6];
    unsigned char      src_mac[6];
    unsigned short     eth_type;
    uint32_t           data[MAX_ETH_DATA_LEN];
} __attribute__((packed)) t_ether_packet;


t_ether_packet  gat_pkts[MAX_PACKET_NUM];
uint32_t        g_pkts_num;


void print_mem(void *start_addr, uint32_t length)
{
    char str_addr[32];
    char str_data[64];
    char str_readable[32];
    unsigned char *cur_pos = start_addr;
    int i;


    while (length >= 16)
    {
        sprintf(str_addr, "%-16lx", (unsigned long)(void *)cur_pos);
        for (i = 0; i < 16; i++)
        {
            sprintf(str_data + i*3, "%02hhx ", cur_pos[i]);
            if (cur_pos[i] > 31 &&  cur_pos[i] < 127)
            sprintf(str_readable + i, "%c", (char)(cur_pos[i]));
            else
            sprintf(str_readable + i, "%c", '*');
        }
        length -= 16;
        cur_pos += 16;
        printf("%s: %s: %s\n", str_addr, str_data, str_readable);
    }


    if (length > 0)
    {
        sprintf(str_addr, "%-16lx", (unsigned long)(void *)cur_pos);
        for (i = 0; i < length; i++)
        {
            sprintf(str_data + i*3, "%02hhx ", cur_pos[i]);
            if (cur_pos[i] > 31 &&  cur_pos[i] < 127)
            sprintf(str_readable + i, "%c", (char)(cur_pos[i]));
            else
            sprintf(str_readable + i, "%c", '*');
        }
        for (i = length; i < 16; i++)
        {
            sprintf(str_data + i*3, "%s", "   ");
            sprintf(str_readable + i, "%c", ' ');
        }
        printf("%s: %s: %s\n", str_addr, str_data, str_readable);
    }




}




static inline void nano_sleep(long sec, long nsec)
{
    struct timespec remain  = (struct timespec) {sec, nsec};
    struct timespec tmp = (struct timespec) {0, 0};
    while (remain.tv_sec || remain.tv_nsec)
    {
        if (0==nanosleep(&remain, &tmp)) return;
        remain = tmp;
        tmp = (struct timespec) {0, 0};
    }
}




int good_write(int fd, void *src, int len)
{
    int ret = 0, finished = 0;


    while (finished < len)
    {
        ret = write(fd, src+finished, len-finished);
        if (ret<0)
        {
            ERR_DBG_PRINT("could not configure");
            return -1;
        }
        finished += ret;
    }


    return len;


}






static int   enable_max_speed;
static char *input_file, tap_name[64], *add_tap_to_ovs_br;
static int   burst_num;
static int64_t   snd_interval = 1000000;
static int64_t   frequency = 1;




#define    LINE_HDR    "        "
void report_working_paras()
{
    struct timeval intvl = {0};


    intvl.tv_sec = snd_interval/1000000;
    intvl.tv_usec = snd_interval%1000000;
    
    printf("\n[working paras]\n");
    
    if (enable_max_speed)
        printf("%sSPEED: MAX_SPEED\n", LINE_HDR);
    else
        printf("%sSPEED: interval=%lus %luus <==> (frequency=%lu)\n"
        , LINE_HDR
        , intvl.tv_sec, intvl.tv_usec, frequency);


    if (burst_num>0)
        printf("%sMODE: burst %d times\n", LINE_HDR, burst_num);
    else
        printf("%sMODE: continue\n", LINE_HDR);


    if (tap_name[0])
        printf("%stap interface name: %s\n", LINE_HDR, tap_name);
    else
        printf("%stap interface name: not provided, use tapx instead, x is a dynamic number\n", LINE_HDR);


        if (add_tap_to_ovs_br)
            printf("%sadd tap to ovs bridge: yes (bridge name %s)\n", LINE_HDR, add_tap_to_ovs_br);
        else
            printf("%sadd tap to ovs bridge: no\n", LINE_HDR);
}


void report_tap_info()
{
        printf("\n[actual tap info]\n");
        printf("%sname: %s\n", LINE_HDR, tap_name);


        if (add_tap_to_ovs_br)
            printf("%sadded to ovs bridge: yes (bridge name %s)\n", LINE_HDR, add_tap_to_ovs_br);
        else
            printf("%sadded to ovs bridge: no\n", LINE_HDR);


}


void report_pkt_load_info(int not_loaded_all)
{
        printf("\n[packet load info]\n");
   
        printf("%s%d packets loaded\n", LINE_HDR, g_pkts_num);


        if (not_loaded_all)
            printf("%sdump file has many packets, but we only load first %d packets\n"
            , LINE_HDR, MAX_PACKET_NUM);


}


int load_from_dump_file(char *file_path)
{
    char errbuf[PCAP_ERRBUF_SIZE];
    struct pcap_pkthdr *header;
    uint32_t got_pkt = 0;
    const u_char *pkt_data;
    int ret = 0;
    pcap_t *handle  = pcap_open_offline(file_path , errbuf);
    if (NULL == handle) 
    {  
        printf("Error: open file %s failed,  %s\n", file_path, errbuf);  
        ret = -1;
        return ret;
    }  




        while ((got_pkt=pcap_next_ex(handle, &header, &pkt_data))==1 && g_pkts_num<MAX_PACKET_NUM)
        {
            t_ether_packet  *pt_pkt = &gat_pkts[g_pkts_num];
            pt_pkt->pkt_len = header->caplen;
            if(pt_pkt->pkt_len>=MAX_ETH_PKT_LEN)
            {
                printf("packet %d length abnormal %d\n", g_pkts_num+1, pt_pkt->pkt_len); 
                ret = -1;
                goto EXIT;
            }
            
            memcpy(pt_pkt->dst_mac, pkt_data, header->caplen);
            g_pkts_num++;


            //DBG_PRINT("pkt %d:", g_pkts_num);
            //print_mem(pt_pkt->dst_mac, pt_pkt->pkt_len);
        }


    report_pkt_load_info(got_pkt && g_pkts_num==MAX_PACKET_NUM);




EXIT:
    pcap_close(handle);
    return 0;
}






#define    ENABLE_MAX_SPEED    (1000)
#define    INPUT_FILE          (1001)
#define    BURST_NUM           (1002)
#define    INTERVAL            (1003)
#define    FREQUENCY           (1004)
#define    TAP_NAME            (1005)
#define    ADD_TAP_TO_OVS_BR   (1006)


struct option my_options[] =
    {
        {"input-file",        required_argument, NULL, INPUT_FILE},
        {"enable-max-speed",  no_argument,       NULL, ENABLE_MAX_SPEED},
        {"burst-num",         required_argument, NULL, BURST_NUM},
        {"interval",          required_argument, NULL, INTERVAL},
        {"frequency",         required_argument, NULL, FREQUENCY},
        {"tap-name",          required_argument, NULL, TAP_NAME},
        {"add-tap-to-ovs-br", required_argument, NULL, ADD_TAP_TO_OVS_BR},
        {0},
    };


const char *opt_remark[][2] = {
    {"(-f or -F for short)","specify tcpdump file"}, 
    {"(-m or -M for short)", "send packet with max speed"}, 
    {"", "how many times to send each packet. If not specified, always sending until press CTRL+C"}, 
    {"(-i or -I for short)", "how long to delay (in us) between two sending"},
    {"", "how many times to send each packet in one second"},
    {"", "name of the new created tap interface. if not provided, use tapx. x is a dynamic number"},
    {"", "if provided, new created tap interface will be added to the given ovs bridge"},
};


void print_usage()
{
    int i, opt_num = ARRAY_SIZE(my_options);
    char *arg;
    printf("%s(v%s) \nUsage example:\n"
        "  ./tap_writer --input-file=my.pcap --enable-max-speed --frequency=3 --burst-num=5\n",
                       app_name, version);




    printf("\n\noptions:\n");
    for (i=0; i<opt_num; i++)
    {
        arg = my_options[i].has_arg?"":"no argument";
        if (my_options[i].name==NULL) break;
        printf("--%s %s :  %s\n      %s\n\n"
            , my_options[i].name
            , opt_remark[i][0]
            , arg
            , opt_remark[i][1]);


    }
}


int check_args()
{


    if (input_file==NULL)
    {
        printf("you must provide a input pcap file\n");
        goto FAIL_EXIT;
    }


    if (enable_max_speed) goto SUCC_EXIT;
    if (snd_interval<=0 || frequency<=0)
    {
        printf("interval or pps must larger than 0\n");
        goto FAIL_EXIT;
    }


SUCC_EXIT:


    report_working_paras();
    return 0;


FAIL_EXIT:
        print_usage();
        return -1;


}




int parse_and_check_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt_long(argc, argv, "mMi:I:f:F:", my_options, NULL)) != -1)
    {
       switch (opt)
       {


           case 'f':
           case 'F':
           case INPUT_FILE: 
               input_file = optarg;
               break;


           case 'i':
           case 'I':
           case INTERVAL:
               snd_interval = strtol(optarg, NULL, 0);
               frequency = 1000000/snd_interval;
               break;


           case BURST_NUM:
               burst_num = strtol(optarg, NULL, 0);
               break;


           case FREQUENCY:
               frequency  = strtol(optarg, NULL, 0);
               snd_interval = 1000000/frequency;
               break;


           case 'm':
           case 'M':
           case ENABLE_MAX_SPEED:
               enable_max_speed = 1;
               break;




           case TAP_NAME:
               strncpy(tap_name, optarg, sizeof(tap_name));
               break;


           case ADD_TAP_TO_OVS_BR:
               add_tap_to_ovs_br = optarg;
               break;


           default: /* '?' */
               print_usage();
       }
    }


    return check_args();
}


int start_up_tap_if(const char *tap)
{
    char cmd_str[128];


    sprintf(cmd_str, "ifconfig %s up", tap);
    if (0!=system(cmd_str))
    {
        //ERR_DBG_PRINT("%s failed", cmd_str);
        //return -1;
    }


    return 0;
}


int add_tap_if_to_ovs_br(const char *br, const char *tap)
{
    char cmd_str[128];


    sprintf(cmd_str, "ovs-vsctl  del-port %s %s", br, tap);
    if (0!=system(cmd_str))
    {
        //ERR_DBG_PRINT("%s failed", cmd_str);
        //return -1;
    }


    sprintf(cmd_str, "ovs-vsctl  add-port %s %s", br, tap);
    if (0!=system(cmd_str))
    {
        //ERR_DBG_PRINT("%s failed", cmd_str);
        //return -1;
    }


    return 0;
}


int prepare_tap_if()
{
    struct ifreq ifr;
    int fd, ret;
    unsigned int features;
    
    fd = open("/dev/net/tun", O_RDWR);
    if (fd < 0) {
        ERR_DBG_PRINT("could not open %s", "/dev/net/tun");
        return -1;
    }
    memset(&ifr, 0, sizeof(ifr));
    ifr.ifr_flags = IFF_TAP | IFF_NO_PI;


    if (ioctl(fd, TUNGETFEATURES, &features) == -1) {
        ERR_DBG_PRINT("warning: TUNGETFEATURES failed:");
        features = 0;
    }


//    if (features & IFF_ONE_QUEUE) {
//        ifr.ifr_flags |= IFF_ONE_QUEUE;
//    }




    if (tap_name[0]==0)
        strcpy(tap_name, "tapsmb%d");


    strcpy(ifr.ifr_name, tap_name);
    
    ret = ioctl(fd, TUNSETIFF, (void *) &ifr);
    if (ret != 0) 
    {


        ERR_DBG_PRINT("could not configure tap interface %s", tap_name);
        close(fd);
        return -1;
    }
    
    strcpy(tap_name, ifr.ifr_name);
    start_up_tap_if(tap_name);


    if (add_tap_to_ovs_br)
        add_tap_if_to_ovs_br(add_tap_to_ovs_br, tap_name);
    
    fcntl(fd, F_SETFL, O_NONBLOCK);


    report_tap_info();
    return fd;
}


static int be_sending, need_stop;
static int  fd;


static inline int time_a_smaller_than_b(struct timeval *a, struct timeval *b)
{
    if (a->tv_sec < b->tv_sec) return 1;
    if (a->tv_sec > b->tv_sec) return 0;


    return a->tv_usec < b->tv_usec;
}


typedef struct
{
    uint64_t send_total, send_fail, rcv_total;
    uint64_t send_total_bytes, send_fail_bytes, rcv_total_bytes;
} t_pkt_stat;


static t_pkt_stat gt_pkt_stat;
uint64_t send_times_cnt;
static struct timeval first_snd_tv, last_snd_tv;


struct timeval time_a_between_b2(struct timeval a, struct timeval b)
{
    struct timeval ret;
    if (b.tv_usec<a.tv_usec)
    {
        b.tv_usec+=1000000;
        b.tv_sec-=1;
    }
    ret.tv_sec = b.tv_sec-a.tv_sec;
    ret.tv_usec = b.tv_usec-a.tv_usec;
    return ret;
}


void report_snd_summary()
{
    struct timeval  tmp = time_a_between_b2(first_snd_tv, last_snd_tv);
    uint64_t pps = gt_pkt_stat.send_total*1000000/(tmp.tv_sec*1000000 + tmp.tv_usec);
    uint64_t bps = gt_pkt_stat.send_total_bytes*1000000/(tmp.tv_sec*1000000 + tmp.tv_usec);;
    
    printf("\n[packet send summary]\n"
        "%stime: %lu sec %lu us\n"
        "%ssucc traffic : %lu packets %lu bytes\n"
        "%ssucc performence : pps %lu; bps %lu\n\n"
        ,LINE_HDR, tmp.tv_sec, tmp.tv_usec
        ,LINE_HDR, gt_pkt_stat.send_total, gt_pkt_stat.send_total_bytes
        ,LINE_HDR, pps, bps);


    if (gt_pkt_stat.send_fail)
    printf("%sfail traffic : %lu packets %lu bytes\n"
        ,LINE_HDR, gt_pkt_stat.send_fail, gt_pkt_stat.send_fail_bytes);


}


int send_pkt(void *arg)
{
int i;
struct timeval cur_tv, next_snd_tv;


    gettimeofday(&next_snd_tv, NULL);


/* Send down the packet */
while (!need_stop)
{
   if (enable_max_speed)
        {
            gettimeofday(&cur_tv, NULL);
             goto SND_PKT;
        }
        
   do
        {   
       gettimeofday(&cur_tv, NULL);
            //nano_sleep(0, 330000000);
        } while (!need_stop && time_a_smaller_than_b(&cur_tv, &next_snd_tv));


        if (need_stop) goto exit;


SND_PKT:


        if (first_snd_tv.tv_sec==0)
            first_snd_tv = cur_tv;
        
        last_snd_tv = cur_tv;
        
    for(i=0;i<g_pkts_num;i++)
        {   
       if (write(fd, gat_pkts[i].dst_mac, gat_pkts[i].pkt_len) != gat_pkts[i].pkt_len)
       {
            gt_pkt_stat.send_fail++;
                gt_pkt_stat.send_fail_bytes+=gat_pkts[i].pkt_len;
        //sys_log(TEXT("Error sending the packet: %s"), pcap_geterr(fp));
            }
            else
            {
                gt_pkt_stat.send_total++;
                gt_pkt_stat.send_total_bytes+=gat_pkts[i].pkt_len;
            }
            
       }


        send_times_cnt++;
        if (burst_num>0 && send_times_cnt>=burst_num)
        {
            goto exit;
        }


        if (enable_max_speed)  continue;


        next_snd_tv.tv_usec+=snd_interval;
        next_snd_tv.tv_sec+=(next_snd_tv.tv_usec/1000000);
        next_snd_tv.tv_usec%=1000000;
    }
    
exit:
    report_snd_summary();
    be_sending = 0;
return 0;
}


void create_thread(pthread_t *thread, void *fn, void *arg)
{
    pthread_create(thread, NULL, fn, arg);
}


void register_sig_act(int sig, void *sig_proc_func)
{
    struct sigaction action;
    memset(&action, 0, sizeof(action));
    action.sa_sigaction = sig_proc_func;
    action.sa_flags = SA_SIGINFO;
    sigaction(sig, &action, NULL);
}


static void  my_sig_handler(int sig_no, siginfo_t *pt_siginfo, void *p_ucontext)
{
    need_stop = 1;
}


int main(int argc, char *argv[])
{
    int  cnt = 0;
    int ret;
    pthread_t  the_thread;
    int dot_str_idx = 0;
    //const char *dots[] = {"", ".", "..", "..."};
    const char *dots[] = {"-", "\\", "|", "/"};


    if (parse_and_check_args(argc, argv))
    {
        return 0;
    }


    fd  = prepare_tap_if();
    if (fd<0) return 0;
    
    ret=load_from_dump_file(input_file);
    if (ret<0) return 0;


    printf("\n====press s begin sending packet====\n");
    while (getchar()!='s');


    need_stop = 0;
    be_sending = 1;
    create_thread(&the_thread, send_pkt, NULL);


    register_sig_act(SIGINT, my_sig_handler);


    printf("\n\n\n");
    while (be_sending)
    {
        printf("\r                  ");
        printf("\rsending packet  %s  [%lu packets, %lu bytes]"
            , dots[dot_str_idx]
            , gt_pkt_stat.send_total, gt_pkt_stat.send_total_bytes);
        fflush(stdout);
        nano_sleep(0, 330000000);
        dot_str_idx++;
        dot_str_idx = (dot_str_idx %4);
    }


    printf("\n====press q to exit====\n");
    while (getchar()!='q');


    return 0;
}
