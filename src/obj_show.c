/*
CmdShowObjV                      cseg     00005DD3 00000024
CmdShowObj                       cseg     00005DF7 0000001C
subShowObj                       cseg     00005E13 00000152
*/

#include "agi.h"
#include "obj_show.h"

#include "res/res.h"
#include "sys/mem_wrap.h"
#include "view/obj_picbuff.h"
#include "view/obj_base.h"
#include "view/view_base.h"
#include "sys/endian.h"
#include "sys/script.h"
#include "view/obj_blit.h"
#include "ui/msg.h"

u8 *cmd_show_obj_v(u8 *c)
{
	obj_show(state.var[*(c++)]);
	return c;
}

u8 *cmd_show_obj(u8 *c)
{
	obj_show(*(c++));
	return c;
}

void obj_show(u16 view_num)
{
	u16 obj_displayed;
	u16 view_loaded;	// 1=view exists
	
	VIEW obj_view;
	//u16 temp2f;	// 0-1	// steptime
	//u8 temp2d;		// 2	// num
	//u16 temp2c;	// 3	// x
	//u16 temp2a;	// 5	// y
	//u8 temp25;		// a	// loop_cur
	//u8 temp21;		// e	// cel_cur
	//u16 temp1f;	// 10	// cel_cur
	//u16 temp1d;	// 12	// cel_data_prev
	//u16 temp19;	// 16	// x_prev
	//u16 temp17;	// 18	// y_prev
	//u16 temp15;	// 1a	// x_size
	//u16 temp13;	// 1c	// y_size
	//u8 tempb;		// 24	// priority
	//u16 tempa;	// 25	// flags
	
	u16 obj_size;	
	BLIT *obj_bg_area;
	VIEW_NODE *si;
	
	script_block();
	obj_displayed = 0;
	view_loaded = (view_find(view_num) != 0);
	
	free_mem_check = 1;
	if ( view_load(view_num, 0) == 0)
	{
		free_mem_check = 0;
		message_box("Not now.");
	}
	else
	{
		free_mem_check = 0;
		
		obj_view.cel_cur = 0;
		obj_view.loop_cur = 0;
		obj_view_set(&obj_view, view_num);
		
		//obj_view.cel_data_prev = obj_view.cel_data;  // removed because of kq4 bug
		obj_view.cel_prev_height = obj_view.cel_data[1];
		obj_view.cel_prev_width = obj_view.cel_data[0];
		obj_view.x_prev = (159-obj_view.x_size)/2;
		obj_view.x = obj_view.x_prev;
		obj_view.y_prev = 167;
		obj_view.y = 167;
		obj_view.priority = 0xF;
		obj_view.flags = obj_view.flags | O_PRIFIXED;
		obj_view.num = 0xFF;
		
		obj_size = obj_view.y_size * obj_view.x_size + sizeof(BLIT);
		//if ( update_var8() > obj_size)
		{
			obj_displayed = 1;
			obj_bg_area = blit_new(&obj_view);
			blit_save(obj_bg_area);
			obj_blit(&obj_view);
			obj_cel_update(&obj_view);
		}
		
		si = view_find(view_num);
		message_box(si->data + load_le_16(si->data + 3));
		
		if (obj_displayed != 0)
		{
			blit_restore(obj_bg_area);
			obj_cel_update(&obj_view);
			if (obj_bg_area->buffer != 0)
			{
				a_free(obj_bg_area->buffer);
				obj_bg_area->buffer = 0;
			}
			a_free(obj_bg_area);
			//set_mem_ptr2(obj_bg_area);
		}
		if (view_loaded == 0)
			view_discard(view_num);
		
		script_allow();
	}
}