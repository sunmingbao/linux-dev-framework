/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <errno.h>
#include <syslog.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "debug.h"
#include "log.h"

char * get_ipstr(struct sockaddr_in *sock_addr, char *ip)
{
    char *tmp_ipstr=inet_ntoa(sock_addr->sin_addr);
    if (ip != NULL)
        strcpy(ip, tmp_ipstr);

    return tmp_ipstr;

}

uint16_t get_port(struct sockaddr_in *sock_addr, uint16_t *port)
{
    uint16_t tmp_port=ntohs(sock_addr->sin_port);
    if (port != NULL)
    *port=tmp_port;

    return tmp_port;
}


void make_sockaddr(struct sockaddr_in *sock_addr, uint32_t ip, uint16_t port)
{
    memset(sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr->sin_family = AF_INET;
    sock_addr->sin_addr.s_addr = ip;
    sock_addr->sin_port = port;

}

int udp_socket_init_2(struct sockaddr_in *sock_addr)
{
    int ret, sockfd;
    
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);

    if (sockfd<0)
    {
        ErrSysLog("create udp socket failed");
        return -1;
    }

    ret=bind(sockfd, (struct sockaddr *)sock_addr, sizeof(struct sockaddr_in));
    if (ret<0)
    {
        ErrSysLog("bind udp socket to %s:%d failed", get_ipstr(sock_addr, NULL), (int)get_port(sock_addr, NULL));
        return -1;
    }

    //DBG_PRINT("bind udp socket to %s:%d succeed", get_ipstr(sock_addr, NULL), (int)get_port(sock_addr, NULL));
    return sockfd;

}

int udp_socket_init(const char *ipstr, uint16_t port)
{
    uint32_t ip = htonl(INADDR_ANY);
    struct sockaddr_in sock_addr;
    
    if (ipstr != NULL) ip = inet_addr(ipstr);
    make_sockaddr(&sock_addr, ip, htons(port));

    return udp_socket_init_2(&sock_addr);

}


int udp_socket_recvfrom(int sockfd, void *buf, int buf_size, struct sockaddr_in *peer_addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int ret=recvfrom(sockfd, buf, buf_size
                     , 0
                     , (struct sockaddr *)peer_addr, peer_addr!=NULL?&addr_len:NULL);
    if (ret<0)
    {
        if (EINTR==errno) return 0;
        
        ErrSysLog("udp socket rcv failed");
        return -1;
    }

    return ret;
}


