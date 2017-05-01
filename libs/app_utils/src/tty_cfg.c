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
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/ioctl.h>
#include <termios.h>
#include "debug.h"

static struct termios	 old_tty_cfg;

int set_tty_input_to_raw_mode()
{
    struct termios   new_tty_cfg;

   
   tcgetattr(0, &old_tty_cfg); 
   new_tty_cfg = old_tty_cfg;
   new_tty_cfg.c_lflag &= ((~ICANON)); 
   new_tty_cfg.c_lflag &= (~(ECHO|ECHOE|ECHOK|ECHONL)); 
      tcflush(0, TCIOFLUSH);     
      if  (tcsetattr(0, TCSANOW, &new_tty_cfg)) {        
        ERR_DBG_PRINT("tcsetattr failed");  
        return -1;     
      }    
      tcflush(0,TCIOFLUSH);   



  return 0;
}


int restore_tty_input_mode()
{
      tcflush(0, TCIOFLUSH);     
      if  (tcsetattr(0, TCSANOW, &old_tty_cfg)) {        
        ERR_DBG_PRINT("tcsetattr failed");  
        return -1;     
      }    
      tcflush(0,TCIOFLUSH);   



  return 0;
}

