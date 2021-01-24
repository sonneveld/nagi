/*
CmdDraw                          cseg     000009EA 0000001C
_Draw                            cseg     00000A06 00000089
CmdErase                         cseg     00000A8F 0000001C
_Erase                           cseg     00000AAB 00000089
*/

#include "../agi.h"

#include "obj_blit.h"
#include "obj_base.h"
#include "obj_update.h"

#include "obj_loop.h"
#include "obj_motion.h"

#include "../view/obj_picbuff.h"
#include "obj_drawerase.h"

#include <setjmp.h>
#include "../sys/error.h"

static void obj_erase(u16 num);


u8 *cmd_draw(u8 *c)
{
	obj_draw(*(c++));
	return c;
}

void obj_draw(u16 num)
{
	VIEW *v;
	v = &objtable[num];
		
	if (objtable_tail < v)
		set_agi_error(0x13, num);
	if (v->cel_data == 0)
		set_agi_error(0x14, num);
	if ( (v->flags & O_DRAWN) == 0)
	{
		v->flags |= O_UPDATE;
		obj_pos_shuffle(v);
		v->cel_prev_height = v->cel_data[1];
		v->cel_prev_width = v->cel_data[0];
		//v->cel_data_prev = v->cel_data;
		v->x_prev = v->x;
		v->y_prev = v->y;
		blitlist_erase(&blitlist_updated);
		v->flags |= O_DRAWN;
		blitlist_draw(build_updated_list());
		obj_cel_update(v);
		v->flags &= ~O_SKIPUPDATE;
	}
}



u8 *cmd_erase(u8 *c)
{
	obj_erase(*(c++));
	return c;
}

static void obj_erase(u16 num)
{
	VIEW *v;
	u16 no_update_flag;

	v = &objtable[num];
	if ( v > objtable_tail)
		set_agi_error(0xC, num);
	if ( (v->flags & O_DRAWN) != 0)
	{
		blitlist_erase(&blitlist_updated);
		if ( (v->flags & O_UPDATE) == 0)
			no_update_flag = 1;
		else
			no_update_flag = 0;
		
		if (no_update_flag == 1 )
			blitlist_erase(&blitlist_static);
		
		v->flags &= ~O_DRAWN;
		
		if (no_update_flag == 1)
			blitlist_draw(build_static_list());
		blitlist_draw(build_updated_list());
		obj_cel_update(v);
	}
}
