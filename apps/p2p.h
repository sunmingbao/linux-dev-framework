/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef __P2P_H_
#define __P2P_H_
#include <stdint.h>

#define    P2P_MSG_CODE_BEGIN       (100)

#define    MSG_P2P_CONN_REQ                (P2P_MSG_CODE_BEGIN+1)
#define    MSG_P2P_CONN_RESP_DST_OFFLINE   (P2P_MSG_CODE_BEGIN+2)
#define    MSG_P2P_CONN_RESP_OK            (P2P_MSG_CODE_BEGIN+3)


typedef struct
{
    uint32_t    msg_id;
    char        usr_src[32];
    char        usr_dst[32];
    char        pass_word[32];
} __attribute__ ((packed)) t_P2P_CONN_REQ;


typedef struct
{
    uint32_t    msg_id;
    char        usr_src[32];
    char        usr_dst[32];

    char         usr_src_ip[40];
    uint16_t     usr_src_port;
    char         usr_dst_ip[40];
    uint16_t     usr_dst_port;
} __attribute__ ((packed)) t_P2P_CONN_RPLY;

#endif

