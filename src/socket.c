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
#include <errno.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

#include "debug.h"
#include "log.h"
#include "socket.h"

char * get_ipstr(struct sockaddr_in *sock_addr, char *ip)
{
    char *tmp_ipstr=(char *)inet_ntoa(sock_addr->sin_addr);
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

int sockaddr_equal(struct sockaddr_in *sock_addr1, struct sockaddr_in *sock_addr2)
{
    return (sock_addr1->sin_family == sock_addr2->sin_family) &&
        (sock_addr1->sin_port == sock_addr2->sin_port) &&
        (sock_addr1->sin_addr.s_addr == sock_addr2->sin_addr.s_addr);
}


void make_sockaddr(struct sockaddr_in *sock_addr, uint32_t ip, uint16_t port)
{
    memset(sock_addr, 0, sizeof(struct sockaddr_in));
    sock_addr->sin_family = AF_INET;
    sock_addr->sin_addr.s_addr = ip;
    sock_addr->sin_port = port;

}

int socket_init_2(int type, struct sockaddr_in *sock_addr)
{
    int ret, sockfd;
    
    sockfd = socket(AF_INET, type, 0);

    if (sockfd<0)
    {
        ErrSysLog("create socket failed");
        return -1;
    }
    
    set_useful_sock_opt(sockfd);
    
    ret=bind(sockfd, (struct sockaddr *)sock_addr, sizeof(struct sockaddr_in));
    if (ret<0)
    {
        ErrSysLog("bind socket to %s:%d failed", get_ipstr(sock_addr, NULL), (int)get_port(sock_addr, NULL));
        return -1;
    }

    SysLog("bind socket to %s:%d succeed", get_ipstr(sock_addr, NULL), (int)get_port(sock_addr, NULL));
    return sockfd;

}

int socket_init(int type, const char *ipstr, uint16_t port)
{
    uint32_t ip = htonl(INADDR_ANY);
    struct sockaddr_in sock_addr;
    
    if (ipstr != NULL && strcmp(ipstr, "0.0.0.0")) ip = inet_addr(ipstr);
    make_sockaddr(&sock_addr, ip, htons(port));

    return socket_init_2(type, &sock_addr);

}

int udp_socket_init(const char *ipstr, uint16_t port)
{
    return socket_init(SOCK_DGRAM, ipstr, port);
}

int tcp_socket_init(const char *ipstr, uint16_t port)
{
    return socket_init(SOCK_STREAM, ipstr, port);
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

int udp_socket_sendto(int sockfd, void *buf, int buf_size, struct sockaddr_in *peer_addr)
{
    socklen_t addr_len = sizeof(struct sockaddr_in);
    int ret=sendto(sockfd, buf, buf_size
                     , 0
                     , (struct sockaddr *)peer_addr, addr_len);
    if (ret<0)
    {
        if (EINTR==errno) return 0;
        
        ErrSysLog("udp socket snd failed");
        return -1;
    }

    return ret;
}

void set_socket_timeout(int sockfd, int snd, int rcv)
{
    struct timeval timeout; 

    if (snd != -1)
    {
        timeout.tv_sec = snd;
        timeout.tv_usec = 0;

        setsockopt (sockfd, SOL_SOCKET, SO_SNDTIMEO, (char *)&timeout,  sizeof(timeout));
    }

    if (rcv != -1)
    {
        timeout.tv_sec = rcv;
        timeout.tv_usec = 0;

        setsockopt (sockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout,  sizeof(timeout));
    }

}


void set_useful_sock_opt(int sockfd)
{
    int nRecvBuf=256*1024;
    int nSendBuf=256*1024;
    int reuse_addr = 1;
    socklen_t optlen = sizeof(int);
    setsockopt(sockfd,SOL_SOCKET,SO_RCVBUF,&nRecvBuf,optlen); 
    setsockopt(sockfd,SOL_SOCKET,SO_SNDBUF,&nSendBuf,optlen);
    setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&reuse_addr,optlen);
}
