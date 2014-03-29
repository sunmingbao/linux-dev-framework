/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __STRING_UTILS_H__
#define  __STRING_UTILS_H__

#include <string.h>
#include   "defs.h"

int str_trim(char *output, char * input);
int str_trim_len(char *output, char * input, int len);
int str_trim_all_len(char *output, char * input, int len);
int str_trim_all(char *output, char * input);
#endif

