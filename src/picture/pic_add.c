/*
CmdAddToPic                      cseg     00002C7A 00000050
CmdAddToPicV                     cseg     00002CCA 00000088
_AddToPic                        cseg     00002D52 000000E0
*/

#include "../agi.h"

#include "../view/view_base.h"
#include "../view/obj_update.h"
#include "../picture/pic_add.h"
#include "../sys/script.h"
#include "../view/obj_picbuff.h"
#include "../ui/events.h"
#include "../sys/video_misc.h"

u8 add_num = 0;
u8 add_loop = 0;
u8 add_cel = 0;
u8 add_x = 0;
u8 add_y = 0;
u8 add_pri = 0;

VIEW view_pic_add;

u8 *cmd_add_to_pic(u8 *c)
{
	add_num = *(c++);
	add_loop = *(c++);
	add_cel = *(c++);
	add_x = *(c++);
	add_y = *(c++);
	add_pri = *(c++);
	add_pri = add_pri | ( *(c++) << 4 );
	add_to_pic();
	return c;
}

u8 *cmd_add_to_pic_v(u8 *c)
{
	add_num = state.var[*(c++)];
	add_loop = state.var[*(c++)];
	add_cel = state.var[*(c++)];
	add_x = state.var[*(c++)];
	add_y = state.var[*(c++)];
	add_pri = state.var[*(c++)];
	add_pri = add_pri | ( state.var[*(c++)] << 4 );
	add_to_pic();
	return c;
}

void add_to_pic()
{
	script_write(5, 0);
	script_write(add_num, add_loop);
	script_write(add_cel, add_x);
	script_write(add_y, add_pri);
	
	obj_view_set(&view_pic_add, add_num);
	obj_loop_set(&view_pic_add, add_loop);
	obj_cel_set(&view_pic_add, add_cel);
	
	
	view_pic_add.cel_prev_height =view_pic_add.cel_data[1];
	view_pic_add.cel_prev_width = view_pic_add.cel_data[0];
	// kq4 bug
	//view_pic_add.cel_data_prev = view_pic_add.cel_data;
	view_pic_add.x_prev = add_x;
	view_pic_add.x = add_x;
	view_pic_add.y_prev = add_y; 
	view_pic_add.y = add_y;
	view_pic_add.flags = O_OBJIGNORE|O_HORIZONIGNORE|O_PRIFIXED;	// 9 3 2
	view_pic_add.priority = 0xF;
	obj_pos_shuffle(&view_pic_add);
	if ( (add_pri & 0xF) == 0)
		view_pic_add.flags = 0;
	view_pic_add.priority = add_pri;
	blists_erase();
	obj_add_pic_pri(&view_pic_add);
	blists_draw();
	obj_cel_update(&view_pic_add);
}

u8 *cmd_show_pri(u8 *c)
{
	pic_buff_rotate = 1;
	pic_buff_update();
	user_bolean_poll();
	pic_buff_update();
	pic_buff_rotate =  0;
	return c;
}