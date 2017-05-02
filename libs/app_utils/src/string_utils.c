/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "string_utils.h"

int str_trim_len(char *output, char * input, int len)
{
    char str_temp[len+1];
    char *head=str_temp;
    char *tail=str_temp + (len - 1);

    if (len <=0) return 0;

    strcpy(str_temp, input);

    while (len >0 && !(isgraph(*head)))
    {
        head++;
        len--;

    }
    if (len <=0) return len;
    
    while (len >0 && !(isgraph(*tail)))
    {
        *tail='\0';
        tail--;
        len--;

    }
    if (len <=0) return len;

    strcpy(output, str_temp);
    return len;

}

int str_trim(char *output, char * input)
{
    return str_trim_len(output, input, strlen(input));
}

int str_trim_all_len(char *output, char * input, int len)
{
    char str_temp[len+1];
    int i, cnt=0;

    if (len <=0) return 0;

    for (i=0; i<len; i++)
    {
        if (isgraph(input[i]))
        {
            str_temp[cnt] = input[i];
            cnt++;
        }

    }

    str_temp[cnt] = 0;

    strcpy(output, str_temp);
    return cnt;

}

int str_trim_all(char *output, char * input)
{
    return str_trim_all_len(output, input, strlen(input));
}

int str_replace_substr(char *output, char *input, char *from, char *to)
{
    int ret = 0, from_len = strlen(from);
    char *ptr1 = input, *ptr2;

    ptr2 = strstr(input, from);
    while (ptr2!=NULL)
    {
        *ptr2 = 0;
        ptr2+=from_len;
        ret+=sprintf(output+ret, "%s%s", ptr1, to);
        ptr1 = ptr2;

        if (*ptr2==0)
            goto EXIT;
        
        ptr2=strstr(ptr2, from);

    }
    ret+=sprintf(output+ret, "%s", ptr1);

EXIT:
    return ret;
}

void trim_new_line(char *str)
{
    int len = strlen(str);
    char *tail=str + (len - 1);
    while (tail>=str && (*tail=='\r' || *tail=='\n'))
        {
            *tail = 0;
            tail--;

        }
}
