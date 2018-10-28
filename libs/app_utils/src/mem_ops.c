/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>

#include "debug.h"

int map_memory(unsigned long long phy_addr, unsigned len, void **result)
{

	int ret = 0;
	void *vaddr;

	int fd;

	fd = open("dev/mem", O_RDWR);
#ifdef __LP64__
	vaddr = mmap(NULL, (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr);
#else
	vaddr = mmap64(NULL, (size_t)len, PROT_READ|PROT_WRITE, MAP_SHARED, fd, phy_addr);
#endif
	*result = vaddr;

	return ret;

}
