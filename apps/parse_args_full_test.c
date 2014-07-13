/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#define _GNU_SOURCE

#include <stdio.h>
#include <unistd.h>
#include <getopt.h>

int enable_opt_i;
int enable_opt_d;
int int_para_n;
char *str_para_s;

#define    LONG_OPT_A_ID    (1000)
#define    LONG_OPT_B_ID    (1001)
#define    LONG_OPT_C_ID    (1002)

int enable_long_opt_a;
int long_int_para_b;
char *long_str_para_c;

struct option my_options[] =
    {
        {"opt_a",  no_argument,       NULL, LONG_OPT_A_ID},
        {"opt_b",  required_argument, NULL, LONG_OPT_B_ID},
        {"opt_c",  required_argument, NULL, LONG_OPT_C_ID},
        {0},
    };

void parse_args(int argc, char *argv[])
{
   int opt;
    while ((opt = getopt_long(argc, argv, "iIn:s:d", my_options, NULL)) != -1)
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

           case LONG_OPT_A_ID:
               enable_long_opt_a = 1;
               break;

           case LONG_OPT_B_ID:
               long_int_para_b = atoi(optarg);
               break;

           case LONG_OPT_C_ID:
               long_str_para_c = optarg;
               break;


           default: /* '?' */
               printf("Usage: %s [-<i/I>] [-n number] [-s string] [-d] [--opt_a] [--opt_b=number] [--opt_c=string]\n",
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

    printf("enable_long_opt_a = %d\n",enable_long_opt_a);
    printf("long_int_para_b   = %d\n",long_int_para_b  );
    printf("long_str_para_c   = %s\n",long_str_para_c  );

    return 0;
}

