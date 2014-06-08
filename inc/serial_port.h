/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __SERIAL_PORT_H__
#define  __SERIAL_PORT_H__

int serial_init(const char *serial_dev_name, int *p_fd, int RTSCTS, int speed, int need_line_input);
int serial_write(int fd_serial, void *src, int len);
int serial_read(int fd_serial, char *buf, int len);

#endif

