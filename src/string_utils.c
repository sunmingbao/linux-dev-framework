/* 
 * ������Ϊ��ѡ���Դ������
 * �������İ�Ȩ(����Դ�뼰�����Ʒ����汾)��һ�й������С�
 * ����������ʹ�á�������������
 * ��Ҳ�������κ���ʽ���κ�Ŀ��ʹ�ñ�����(����Դ�뼰�����Ʒ����汾)���������κΰ�Ȩ���ơ�
 * =====================
 * ����: ������
 * ����: sunmingbao@126.com
 */

#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "string_utils.h"
#include "debug.h"
#include "log.h"

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
