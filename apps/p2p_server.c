/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <getopt.h>
#include <stdlib.h>

#include "p2p.h"
#include "socket.h"
#include "common.h"
#include "daemon.h"
#include "log.h"
#include "mm.h"
#include "list.h"
#include "debug.h"

static char     ip[40] = "0.0.0.0";
static uint16_t port   = 8888;

typedef struct
{
    struct list_head list;
    struct list_head hash;
    
    time_t timestamp;
    
    t_P2P_CONN_REQ  req;  
    
    struct sockaddr_in  sockaddr_src;
} t_conn_req;

#define    MAX_REQ_NUM     (256)
#define    REQ_LIFE_TIME   (30)
static int req_num;
static struct list_head all_reqs;
static struct list_head hash_reqs[256];

static int req_equal(t_P2P_CONN_REQ  *pt_req1, t_P2P_CONN_REQ  *pt_req2)
{
    if (strcmp(pt_req1->usr_src, pt_req2->usr_src))
        return 0;

    if (strcmp(pt_req1->usr_dst, pt_req2->usr_dst))
        return 0;
    
    return 1;
}

static int req_match(t_P2P_CONN_REQ  *pt_req1, t_P2P_CONN_REQ  *pt_req2)
{
    if (strcmp(pt_req1->usr_src, pt_req2->usr_dst))
        return 0;

    if (strcmp(pt_req1->usr_dst, pt_req2->usr_src))
        return 0;

    if (strcmp(pt_req1->pass_word, pt_req2->pass_word))
        return 0;

    return 1;
}

static int hash_index(const char *usr_name)
{
    int idx = usr_name[0]*usr_name[1];
    idx = idx%ARRAY_SIZE(hash_reqs);
    return idx;
}

static void *find_req(t_P2P_CONN_REQ  *pt_req2find)
{
    t_conn_req  *pt_req;
    struct list_head *pos, *hash_head;
    int idx=hash_index(pt_req2find->usr_src);
    
    hash_head = &(hash_reqs[idx]);
    list_for_each(pos, hash_head)
    {
        pt_req = list_entry(pos, t_conn_req, hash);
        if (req_equal(&(pt_req->req), pt_req2find))
        {
            return pt_req;
        }

    }

    return NULL;
}

static void *find_match_req(t_P2P_CONN_REQ  *pt_req)
{
    t_conn_req  *pt_match_req;
    struct list_head *pos, *hash_head;
    int idx=hash_index(pt_req->usr_dst);
    
    hash_head = &(hash_reqs[idx]);
    list_for_each(pos, hash_head)
    {
        pt_match_req = list_entry(pos, t_conn_req, hash);
        if (req_match(&(pt_match_req->req), pt_req))
        {
            return pt_match_req;
        }

    }

    return NULL;
}

int req_expired(t_conn_req  *pt_req)
{
    time_t time_now=time(NULL);
    return ((time_now - pt_req->timestamp)>REQ_LIFE_TIME);
}

static void delete_req(t_conn_req  *pt_req)
{
    list_del(&(pt_req->list));
    list_del(&(pt_req->hash));
    req_num--;
}

static void release_req(t_conn_req  *pt_req)
{
    delete_req(pt_req);
    free_buffer(pt_req);
}

static void add_req(t_conn_req  *pt_req)
{
    struct list_head *hash_head;
    int idx=hash_index(pt_req->req.usr_src);
    hash_head = &(hash_reqs[idx]);

    list_add_tail(&(pt_req->list), &all_reqs);
    list_add_tail(&(pt_req->hash), hash_head);
    req_num++;
}


static void release_oldest_req()
{
    struct list_head *pos = all_reqs.next;
    t_conn_req    *pt_req = list_entry(pos, t_conn_req, list);
    release_req(pt_req);
}

static int      gap=60;

static int sock_fd;

static int need_exit;


static void main_loop()
{
    t_P2P_CONN_REQ  conn_req;
    struct sockaddr_in  sockaddr;
    t_conn_req *pt_req, *pt_req_to_find;
    int ret;

    t_P2P_CONN_RPLY reply;
    while (!need_exit)
    {
        ret = udp_socket_recvfrom(sock_fd, &conn_req, sizeof(t_conn_req), &sockaddr);

        if (ret<=0) continue;
        
        SysLog("recved %d bytes from %s:%d", ret, get_ipstr(&sockaddr, NULL), (int)get_port(&sockaddr, NULL));
        //print_mem(&conn_req, ret);
        conn_req.msg_id=ntohl(conn_req.msg_id);
        SysLog("%s wants to connect %s", conn_req.usr_src, conn_req.usr_dst);
        
        pt_req=find_req(&conn_req);
        if (pt_req == NULL)
        {
            if (req_num>=MAX_REQ_NUM)
            {
                release_oldest_req();
            }
            pt_req=alloc_buffer();
        }
        else
        {
            delete_req(pt_req);
        }

        memcpy(&(pt_req->req), &conn_req, sizeof(conn_req));
        pt_req->sockaddr_src=sockaddr;
        pt_req->timestamp=time(NULL);
        add_req(pt_req);

        pt_req_to_find = find_match_req(&conn_req);
        if (pt_req_to_find != NULL)
        {
            if ((int)(difftime(pt_req->timestamp, pt_req_to_find->timestamp))>gap)
            {
                release_req(pt_req_to_find);
                pt_req_to_find = NULL;
            }

        }

        reply.msg_id=htonl(MSG_P2P_CONN_RESP_DST_OFFLINE);
        strcpy(reply.usr_src, conn_req.usr_src);
        strcpy(reply.usr_dst, conn_req.usr_dst);
        strcpy(reply.usr_src_ip, get_ipstr(&sockaddr, NULL));
        reply.usr_src_port = htons(get_port(&sockaddr, NULL));

        if (pt_req_to_find==NULL)
        {
            SysLog("no match usr %s found for %s", conn_req.usr_dst, conn_req.usr_src);
            udp_socket_sendto(sock_fd, &reply, sizeof(reply), &sockaddr);
        }
        else
        {
            SysLog("usr %s is online", conn_req.usr_dst);
            reply.msg_id=htonl(MSG_P2P_CONN_RESP_OK);

            SysLog("send replay to %s", conn_req.usr_src);
            strcpy(reply.usr_dst_ip, get_ipstr(&(pt_req_to_find->sockaddr_src), NULL));
            reply.usr_dst_port = htons(get_port(&(pt_req_to_find->sockaddr_src), NULL));
            udp_socket_sendto(sock_fd, &reply, sizeof(reply), &sockaddr);
        }
            
    }

}

#define    LONG_OPT_ID_IP      (1001)
#define    LONG_OPT_ID_PORT    (1002)

struct option our_options[] =
    {
        {"ip",    required_argument, NULL, LONG_OPT_ID_IP},
        {"port",  required_argument, NULL, LONG_OPT_ID_PORT},
        {0},
    };

static void parse_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt_long(argc, argv, "i:p:", our_options, NULL)) != -1)
    {
       switch (opt)
       {
           case 'i':
           case LONG_OPT_ID_IP:
               strcpy(ip, optarg);
               SysLog("==ip addr :%s", ip);
               break;

           case 'p':
           case LONG_OPT_ID_PORT:
               port = atoi(optarg);
               SysLog("==port :%d", (int)port);
               break;
               
           default: /* '?' */
               SysLog("Usage: %s [--ip ip_addr] [--port port]\n",
                       argv[0]);
               exit(1);
       }
    }

    SysLog("server will bind to  %s:%d", ip, (int)port);
}

static void init_internal_struct()
{
    int i;
    
    init_buffer(sizeof(t_conn_req), MAX_REQ_NUM);
    INIT_LIST_HEAD(&all_reqs);

    for (i=0;i<ARRAY_SIZE(hash_reqs);i++)
    {
        INIT_LIST_HEAD(&(hash_reqs[i]));

    }

}

int main(int argc, char *argv[])
{
    init_log("p2p_server.log", DFT_LOG_FILE_SIZE);

    init_internal_struct();
    
    parse_args(argc, argv);
    sock_fd=udp_socket_init(ip, port);

    main_loop();

    return 0;
}

