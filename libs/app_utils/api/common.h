/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __COMMON_H__
#define  __COMMON_H__

#include <stdio.h>
#include <getopt.h>
#include <inttypes.h>

#define    ARRAY_SIZE(array_name)    (sizeof(array_name)/sizeof(array_name[0]))
#define    MEMBER_OFFSET(type_name, member_name)    ((unsigned long)(void *)(&(((type_name *)NULL)->member_name)))

static inline void print_usage(const char *prog, const char *syntax, const struct option *opts, int nr_opt)
{
    int i;
    printf("usage:\n"
		  "\t%s %s\n\n"
		  ,prog
		  ,syntax);

	printf("full option list:\n"
		"%-32s    %-12s\n"
		,"<name>"
		,"<has arg>");

	for (i=0; i<nr_opt; i++)
		if (opts[i].name)
			printf("--%-30s    %-12s\n"
			,opts[i].name
			,(opts[i].has_arg==required_argument)?"yes":"no");
	
}



#endif

