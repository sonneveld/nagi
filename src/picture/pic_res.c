// _InitPicList                     cseg     000049CD 0000000F
// _FreePicList                     cseg     000049DC 0000000C
// _FindPic                         cseg     000049E8 0000002E
// CmdLoadPic                       cseg     00004A16 00000025
// _LoadPic                         cseg     00004A3B 0000006F
// CmdDrawPic                       cseg     00004AAA 00000025
// _DrawPic                         cseg     00004ACF 00000048
// CmdOverlayPic                    cseg     00004B17 00000024
// _OverlayPic                      cseg     00004B3B 00000047
// CmdShowPic                       cseg     00004B82 00000028
// CmdDiscardPic                    cseg     00004BAA 00000024
// _DiscardPic                      cseg     00004BCE 00000047
// CmdSetUpperLeft                  cseg     00004C15 0000000E


#include <stdlib.h>
#include "../agi.h"


#include "../picture/sbuf_util.h"
#include "../sys/drv_video.h"
#include "../sys/gfx.h"

#include "../ui/msg.h"

#include "../picture/pic_res.h"
#include "../picture/pic_render.h"

#include "../res/res.h"

// writescript
#include "../sys/script.h"

// blit/erase both
#include "../view/obj_update.h"

// agi error
#include <setjmp.h>
#include "../sys/error.h"

// flags
#include "../flags.h"

// for update_var8
#include "../sys/memory.h"

#include "../sys/mem_wrap.h"



PIC pic_head = {0,0,0};	// the pic head struct
PIC *last_pic;
u16 pic_visible = 0;


void pic_list_init()
{
	PIC *next, *cur;
		
	cur = pic_head.next;
	
	while (cur != 0)
	{
		next = cur->next;
		if (cur->data != 0)
			a_free(cur->data);
		a_free(cur);
		cur = next;
	}
	
	pic_head.next = 0;
} 

void pic_list_new_room()
{
	pic_list_init();
}

// finds the pointer to a pic struct
// passes 0 if nothing found.
PIC *pic_find(u16 pic_num)
{
	PIC *next, *cur;
	
	next = pic_head.next;
	cur = &pic_head;
	goto loop;
	
next_pic:
	cur = next;
	next = next->next;
loop:
	if (next != 0)
		if (next->num != pic_num)
			goto next_pic;
	
	last_pic = cur;	// used if reached zero
	return (next);
}

u8 *cmd_load_pic(u8 *c)
{
	pic_load(state.var[*(c++)]);
	return c;
}

PIC *pic_load(u16 pic_num)
{
	PIC *n=0, *prev=0;
	
	n = pic_find(pic_num);
	
	if (n == 0)
	{
		blists_erase();
		script_write(2, pic_num);

		if (last_pic == 0)
			n = &pic_head;	// this should never happen
		else
		{
			n = (PIC *)a_malloc(sizeof(PIC));
			prev = last_pic;
			prev->next = n;
			n->next = 0;	// next_struct
		}

		n->num = pic_num;	// pic_num
		n->data = vol_res_load( dir_picture(pic_num), 0);
		
		if (n->data == 0) return 0;
	
		blists_draw();
	}

	return(n);
}


u8 *cmd_draw_pic(u8 *c)
{
	pic_draw(state.var[*(c++)]);
	return c;
}


void pic_draw(u16 pic_num)
{
	PIC *cur;
	
	state.pic_num = pic_num;
	cur = pic_find(pic_num);
	
	if (cur == 0)
		set_agi_error(0x12, pic_num);

	script_write(4, pic_num);
	given_pic_data = cur->data;
	
	blists_erase();
	render_pic(0);
	blists_draw();
	
	pic_visible = 0;
}

u8 *cmd_overlay_pic(u8 *c)
{
	pic_overlay(state.var[*(c++)]);
	return c;
}

void pic_overlay(u16 pic_num)
{
	PIC *cur;
	
	state.pic_num = pic_num;
	cur = pic_find(pic_num);
	
	if (cur == 0)
		set_agi_error(0x12, pic_num);
	
	script_write(8, pic_num);
	given_pic_data = cur->data;
	
	blists_erase();
	render_overlay();	// no clearing
	blists_draw();
	blists_update();
	
	pic_visible = 0;
}

u8 *cmd_show_pic(u8 *c)
{
	flag_reset(F15_PRINTMODE);
	cmd_close_window(0);
	gfx_picbuff_update();
	pic_visible = 1;
	return c;
}


u8 *cmd_discard_pic(u8 *c)
{
	pic_discard(state.var[*(c++)]);
	return c;

}

void pic_discard(u16 pic_num)
{
	PIC *cur;
	
	cur = pic_find(pic_num);

	if (cur == 0)
		set_agi_error(0x15, pic_num);

	script_write(6, pic_num);

	last_pic->next = 0;
	
	blists_erase();
	
	if (cur->data != 0)
		a_free(cur->data);
	a_free(cur);
	
	blists_draw();
	update_var8();
}


u8 *cmd_set_upper_left(u8 *c)
{
	printf("cmd.set.upper.left(%d, %d) was called\n", c[0], c[1]);
	return c+2;
}
