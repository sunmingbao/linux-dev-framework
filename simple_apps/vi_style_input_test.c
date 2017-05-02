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
#include <stdint.h>
#include "io_utils.h"
#include "tty_cfg.h"
#include "screen_ops.h"
#include "string_utils.h"
#include "debug.h"

#define MAX_LINE_LEN    (60)
#define MAX_LINE_NR    (10)

static char empty_line[MAX_LINE_LEN+1] = { [0 ... MAX_LINE_LEN-1] =  ' ',};
typedef struct
{
    char some_data_for_edit[MAX_LINE_NR][MAX_LINE_LEN+1];
	int cur_line;
	int cur_col;
	int line_data_len[MAX_LINE_NR];
} t_edit_ctx;

static t_edit_ctx the_edit_ctx;

static void adjust_cursor()
{
	int cur_line = the_edit_ctx.cur_line;
	int cur_line_len = the_edit_ctx.line_data_len[cur_line];
	int cur_col = the_edit_ctx.cur_col;

	if (cur_col<=cur_line_len)
		return;

	cur_col=cur_line_len;
	printf("\r");
	fflush(stdout); 

	if (cur_col>0)
	    MOVE_CURSOR_RIGHT(cur_col);

	the_edit_ctx.cur_col = cur_col;

}


static void move_cursor_up()
{

	if (the_edit_ctx.cur_line<1)
		return;

	
	MOVE_CURSOR_UP(1);
	the_edit_ctx.cur_line--;
	adjust_cursor();

}


static void move_cursor_down()
{

	if (the_edit_ctx.cur_line>=(MAX_LINE_NR-1))
		return;


	
	MOVE_CURSOR_DOWN(1);
	the_edit_ctx.cur_line++;
	adjust_cursor();

}

static void move_cursor_left()
{
    int cur_col = the_edit_ctx.cur_col;

	if (cur_col<=0)
		return;
	
	
	MOVE_CURSOR_LEFT(1);
	the_edit_ctx.cur_col--;
}

static void move_cursor_right()
{
    int cur_line = the_edit_ctx.cur_line;
	int cur_line_len = the_edit_ctx.line_data_len[cur_line];
    int cur_col = the_edit_ctx.cur_col;

	if (cur_col>=cur_line_len)
		return;
	
	
	MOVE_CURSOR_RIGHT(1);
	the_edit_ctx.cur_col++;
}

static void backspace_proc()
{
	int cur_line = the_edit_ctx.cur_line;
	char *line = the_edit_ctx.some_data_for_edit[cur_line];
	int cur_line_len = the_edit_ctx.line_data_len[cur_line];
	int cur_col = the_edit_ctx.cur_col;

	if (cur_line_len<=0 || cur_col<1)
		return;
	
	delete_char(line, cur_col-1, cur_line_len);
	cur_line_len--;
	cur_col--;
	printf("\r%s\r%s\r", empty_line, line);
	fflush(stdout); 

	if (cur_col>0)
	    MOVE_CURSOR_RIGHT(cur_col);

	the_edit_ctx.line_data_len[cur_line]=cur_line_len;
	the_edit_ctx.cur_col = cur_col;

}


static void delete_proc()
{
	int cur_line = the_edit_ctx.cur_line;
	char *line = the_edit_ctx.some_data_for_edit[cur_line];
	int cur_line_len = the_edit_ctx.line_data_len[cur_line];
	int cur_col = the_edit_ctx.cur_col;

	if (cur_line_len<=0 || cur_col>=cur_line_len)
		return;
	
	delete_char(line, cur_col, cur_line_len);
	cur_line_len--;
	printf("\r%s\r%s\r", empty_line, line);
	fflush(stdout); 

	if (cur_col>0)
	    MOVE_CURSOR_RIGHT(cur_col);

	the_edit_ctx.line_data_len[cur_line]=cur_line_len;
	the_edit_ctx.cur_col = cur_col;

}

static void virtual_key_input_proc(uint8_t *input, int len)
{
	if (0x1b != input[0])
	    ERR_DBG_PRINT_QUIT("what's going on? virtual key input[0]==0x%02hhx", input[0]);

    if (0x5b==input[1])
	{
	    switch (input[2])
    	{
    	    case 0x41:  /* up */
				
				move_cursor_up();
				break;

			case 0x42:	/* down */
				move_cursor_down();
				break;

		    case 0x44:  /* left*/
				move_cursor_left();
				break;

		    case 0x43:  /* right */
				move_cursor_right();
				break;

			case 0x33:
				if (0x7e==input[3]) //delete pressed in gui
					delete_proc();
				break;

    	}

	}
	else if (0x4f==input[1])
	{
		switch (input[2])
		{
			case 0x50:	/* f1 */
				break;

			case 0x51:	/* f2 */
				break;

			case 0x52:	/* f3*/
				break;

			case 0x53:	/* f4 */
				break;

		}
	
	}

	
}




static void special_key_input_proc(uint8_t c)
{

    switch (c)
	{

	    case 0x1b:  //esc pressed
			break;
			
		case 0x0a:	//enter pressed
		    move_cursor_down();
			break;

		case 0x08:	//backspace pressed
		    backspace_proc();
			break;

		case 0x7f:	//delete pressed or backspace pressed in gui
		    //delete_proc();
			backspace_proc();
			break;

	}
	
}

static void normal_key_input_proc(char c)
{
    int cur_line = the_edit_ctx.cur_line;
    char *line = the_edit_ctx.some_data_for_edit[cur_line];
	int cur_line_len = the_edit_ctx.line_data_len[cur_line];
    int cur_col = the_edit_ctx.cur_col;

	if (cur_line_len>=MAX_LINE_LEN)
		return;
	
	insert_char(line, c, cur_col, cur_line_len);
	cur_line_len++;
	cur_col++;
	printf("\r%s\r%s\r", empty_line, line);
	MOVE_CURSOR_RIGHT(cur_col);

    the_edit_ctx.line_data_len[cur_line]=cur_line_len;
	the_edit_ctx.cur_col = cur_col;

}

static void input_proc(uint8_t *input, int len)
{
# if 0
    int i;

    DBG_PRINT("len=%d", len);
	for (i=0; i<len; i++)
	    DBG_PRINT("%02hhx", input[i]);

	return;
#endif

    if (len>=3)
		goto VIRTUAL_KEY_PROC;

	if (1 != len)
		ERR_DBG_PRINT_QUIT("what's going on? input len=%d", len);


    if (input[0] > 31 &&  input[0] < 127)
        normal_key_input_proc((char)input[0]);
	else
		special_key_input_proc(input[0]);
	goto exit;


VIRTUAL_KEY_PROC:
	virtual_key_input_proc(input, len);

exit:
    return;
}

static void input_proc_loop()
{
	int ret;
	uint8_t input[4];
    while (1)
	{
	    ret=read_reliable(0, input, sizeof(input));
		if (ret<0)
			ERR_DBG_PRINT_QUIT("read input failed");

		if (ret>0)
			input_proc(input, ret);

	}
}

int main(int argc, char *argv[])
{
    set_tty_input_to_raw_mode();
	CLEAR_SCREEN();
	MOVE_CURSOR_TO(0,0);
	input_proc_loop();
    return 0;
}

