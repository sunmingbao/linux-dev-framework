/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>

#include "debug.h"
#include "log.h"

#define    page_map_file     "/proc/self/pagemap"
#define    PFN_MASK          ((((uint64_t)1)<<55)-1)
#define    PFN_PRESENT_FLAG  (((uint64_t)1)<<63)

int mem_addr_vir2phy(unsigned long vir, unsigned long *phy)
{
    int fd;
    int page_size=getpagesize();
    unsigned long vir_page_idx = vir/page_size;
    unsigned long pfn_item_offset = vir_page_idx*sizeof(uint64_t);
    uint64_t pfn_item;
    
    fd = open(page_map_file, O_RDONLY);
    if (fd<0)
    {
        ErrSysLog("open %s failed", page_map_file);
        return -1;
    }

    if ((off_t)-1 == lseek(fd, pfn_item_offset, SEEK_SET))
    {
        ErrSysLog("lseek %s failed", page_map_file);
        return -1;
    }

    if (sizeof(uint64_t) != read(fd, &pfn_item, sizeof(uint64_t)))
    {
        ErrSysLog("read %s failed", page_map_file);
        return -1;
    }

    if (0==(pfn_item & PFN_PRESENT_FLAG))
    {
        ErrSysLog("page is not present");
        return -1;
    }

    *phy = (pfn_item & PFN_MASK)*page_size + vir % page_size;
    return 0;

}

/*
如果担心vir地址对应的页面不在内存中，可以在调用mem_addr_vir2phy之前，
先访问一下此地址。

例如， int  a=*(int *)(void *)vir;

如果担心Linux的swap功能将进程的页面交换到硬盘上从而导致页面的物理地址变化，
可以关闭swap功能。

下面两个C库函数可以阻止Linux将当前进程的部分或全部页面交换到硬盘上。

       int mlock(const void *addr, size_t len);
       int mlockall(int flags);
*/