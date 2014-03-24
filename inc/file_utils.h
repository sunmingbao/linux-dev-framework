/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __UTILS_H__
#define  __UTILS_H__

#include   "defs.h"

#define    MAX_FILE_NAME_LEN    (128)
#define    MAX_ARGS_LEN         (128)
#define    MAX_VAR_NAME_LEN     (128)

#define    MAX_VAR_NUM          (50)
#define    MAX_VAR_VALUE_LEN    (MAX_PATH_LEN)

int parse_config_file(char *path_to_config_file, int need_update);
char * get_config_var(char *var_name);
#endif

