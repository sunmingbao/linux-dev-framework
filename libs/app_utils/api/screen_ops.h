/* 
 * 本软件为免费、开源软件。
 * 本软件的版权(包括源码及二进制发布版本)归一切公众所有。
 * 您可以自由使用、传播本软件。
 * 您也可以以任何形式、任何目的使用本软件(包括源码及二进制发布版本)，而不受任何版权限制。
 * =====================
 * 作者: 孙明保
 * 邮箱: sunmingbao@126.com
 */

#ifndef  __SCREEN_OPS_H__
#define  __SCREEN_OPS_H__

#include <stdio.h>

// 清除屏幕 
#define CLEAR_SCREEN() do { printf("\033[2J"); fflush(stdout); } while (0)

// 光标移动 
#define MOVE_CURSOR_UP(x) do { printf("\033[%dA", (x)) ; fflush(stdout); } while (0)

#define MOVE_CURSOR_DOWN(x) do { printf("\033[%dB", (x)) ; fflush(stdout); } while (0)

#define MOVE_CURSOR_LEFT(y) do { printf("\033[%dD", (y)) ; fflush(stdout); } while (0)

#define MOVE_CURSOR_RIGHT(y) do { printf("\033[%dC",(y)) ; fflush(stdout); } while (0)

#define MOVE_CURSOR_TO(x,y) do { printf("\033[%d;%dH", (x), (y)) ; fflush(stdout); } while (0)

// 其他操作 
#define RESET_CURSOR() do { printf("\033[H") ; fflush(stdout); } while (0)
#define HIDE_CURSOR() do { printf("\033[?25l") ; fflush(stdout); } while (0)
#define SHOW_CURSOR() do { printf("\033[?25h") ; fflush(stdout); } while (0)
#define HIGHT_LIGHT() do { printf("\033[7m")
#define UN_HIGHT_LIGHT() do { printf("\033[27m"); fflush(stdout); } while (0)
#define PUSH_SCREEN() do { printf("\033[?1049h\033[H"); fflush(stdout); } while (0)
#define POP_SCREEN() do { printf("\033[?1049l"); fflush(stdout); } while (0)

/* 下面是更多的信息 */
#if 0
printf("\033[47;31mhello world\033[5m");

47是字背景颜色, 31是字体的颜色, hello world是字符串. 后面的\033[5m是控制码.
颜色代码:
QUOTE:
字背景颜色范围: 40--49 字颜色: 30--39
40: 黑 30: 黑
41: 红 31: 红
42: 绿 32: 绿
43: 黄 33: 黄
44: 蓝 34: 蓝
45: 紫 35: 紫
46: 深绿 36: 深绿
47: 白色 37: 白色
ANSI控制码:
QUOTE:
\033[0m 关闭所有属性 
\033[1m 设置高亮度 
\03[4m 下划线 
\033[5m 闪烁 
\033[7m 反显 
\033[8m 消隐 
\033[30m -- \033[37m 设置前景色 
\033[40m -- \033[47m 设置背景色 
\033[nA 光标上移n行 
\03[nB 光标下移n行 
\033[nC 光标右移n行 
\033[nD 光标左移n行 
\033[y;xH设置光标位置 
\033[2J 清屏 
\033[K 清除从光标到行尾的内容 
\033[s 保存光标位置 
\033[u 恢复光标位置 
\033[?25l 隐藏光标 
\33[?25h 显示光标
这样, 在某些时候就可以实现动态的输出

#endif

#endif

