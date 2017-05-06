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
#include <sys/types.h>
#include <unistd.h>

#include <sys/socket.h>
#include <linux/netlink.h>
#include <linux/rtnetlink.h>
#include "io_utils.h"
#include "debug.h"

#define    INPUT_BUF_LEN  (1024*1024)

static void net_link_msg_proc(void *data, int len)
{
	struct nlmsghdr *nlh = data;
	
/* 内核中的函数rtmsg_ifinfo用于通告网口状态的变化 
   消息类型的定义在C库头文件 <linux/rtnetlink.h> 中 

   例如: 
	RTM_NEWLINK     = 16,
#define RTM_NEWLINK     RTM_NEWLINK

	RTM_NEWROUTE    = 24,
#define RTM_NEWROUTE    RTM_NEWROUTE

*/

    DBG_PRINT("got a msg, len=%d\n"
		"nlmsg_len = %d\n"
		"nlmsg_type = %d\n"
		"nlmsg_flags = %d\n"
		"nlmsg_seq = %d\n"
		"nlmsg_pid = %d\n"
    ,len
	,(int)nlh->nlmsg_len 
	,(int)nlh->nlmsg_type
	,(int)nlh->nlmsg_flags
	,(int)nlh->nlmsg_seq 
	,(int)nlh->nlmsg_pid);
	//print_mem(data, len);
}

static void event_proc_loop(int fd)
{
	int ret;
	uint8_t  input[INPUT_BUF_LEN];
	
    while (1)
	{
	    ret=fd_readable(fd, 1, 0);
		if (!ret)
			continue;
		
		ret=read(fd, input, sizeof(input));
		if (ret<0)
		{
		    ERR_DBG_PRINT("read failed");
			continue;
		}

		if (ret>0)
			net_link_msg_proc(input, ret);

	}
}

int create_netlink_socket()
{
    int ret;
    struct sockaddr_nl the_sock_addr = 
		{
		    .nl_family = AF_NETLINK,
			.nl_pid    = getpid(),
			.nl_groups = 0xffffffff,

		};

	int fd = socket(AF_NETLINK, SOCK_DGRAM, NETLINK_ROUTE);

	if (fd<0)
		ERR_DBG_PRINT_QUIT("create netlink socket failed:");

	ret = bind(fd, (void *)&the_sock_addr, sizeof(the_sock_addr));
	if (ret<0)
		ERR_DBG_PRINT_QUIT("bind socket failed:");

	return fd;
}

int main(int argc, char *argv[])
{
	int fd = create_netlink_socket();
	
	event_proc_loop(fd);
	
    return 0;
}

