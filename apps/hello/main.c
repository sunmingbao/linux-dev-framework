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

#include "debug.h"
#include "a.h"

int main(int argc, char *argv[])
{
    DBG_PRINT("2+3=%d", add_num(2, 3));

set_tty_input_to_raw_mode();
	while (1)
	{

	    DBG_PRINT("%c", getchar());

	}
    return 0;
}

