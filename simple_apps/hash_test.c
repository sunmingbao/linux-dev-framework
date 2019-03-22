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

#ifdef _USE_OPENSSL
#include <openssl/md5.h> /* link with -lcrypto not -lcrypt , see man 3 crypto and man 3 crypt */
#else
#include "md5.h"
#endif

#include "debug.h"

#ifdef _USE_OPENSSL
void md5_test(const char *str)
{
  MD5_CTX context;
  unsigned char digest[16];
  unsigned int len = strlen (str);

  MD5_Init (&context);
  MD5_Update (&context, str, len);
  MD5_Final (digest, &context);

  print_mem(digest,16);
}
#else
void md5_test(const char *str)
{
    ;
}
#endif


int main(int argc, char *argv[])
{
	DBG_PRINT_S("hello");
	md5_test("abc");

	return 0;
}

