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
#include <errno.h>

#include "measure.h"

void f1()
{
    printf("f1 \n");
}

void f2()
{
    printf("f2 \n");
}

void test_perform()
{
    int i;
    DECLAIRE_TIME_MEASURE(f1);
    DECLAIRE_TIME_MEASURE(f2);

    INIT_TIME_MEASURE(f1);
    INIT_TIME_MEASURE(f2);

    for (i=0; i<5; i++)
    {
        TIME_MEASURE_BEGIN(f1);
        f1();
        TIME_MEASURE_END(f1);

    }

    for (i=0; i<3; i++)
    {
        TIME_MEASURE_BEGIN(f2);
        f2();
        TIME_MEASURE_END(f2);

    }


}

int main(int argc, char *argv[])
{
    init_measure(2);
    test_perform();
    report_measure();
    return 0;
}

