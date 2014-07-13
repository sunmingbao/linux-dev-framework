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
#include <unistd.h>

int enable_opt_i;
int enable_opt_d;
int int_para_n;
char *str_para_s;

void parse_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt(argc, argv, "iIn:s:d")) != -1)
    {
       switch (opt)
       {
           case 'i':
           case 'I':
               enable_opt_i = 1;
               break;
               
           case 'n':    
               int_para_n = atoi(optarg);
               break;
               

           case 's':    
               str_para_s = optarg;
               break;

           case 'd':    
               enable_opt_d = 1;
               break;




           default: /* '?' */
               printf("Usage: %s [-<i/I>] [-n number] [-s string] [-d]\n",
                       argv[0]);
       }
    }

}


int main(int argc, char *argv[])
{
    parse_args(argc, argv);

    printf("enable_opt_i = %d\n",enable_opt_i);
    printf("enable_opt_d = %d\n",enable_opt_d);
    printf("int_para_n   = %d\n",int_para_n  );
    printf("str_para_s   = %s\n",str_para_s  );
    
    return 0;
}

