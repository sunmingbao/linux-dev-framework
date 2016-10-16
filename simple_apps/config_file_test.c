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
#include <string.h>
#include "config_file_ops.h"

char *some_data="abc=\"123\"";
int main(int argc, char *argv[])
{
    FILE *file=fopen("test.config", "w");
    
    fwrite(some_data, 1, strlen(some_data),
                     file);
    fclose(file);

    parse_config_file("test.config", 0);

    printf("abc=%s\n", get_config_var("abc"));

    return 0;
}

