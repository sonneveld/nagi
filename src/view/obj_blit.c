/*
_SaveArea                        cseg     00009CF0 00000003
_RestoreArea                     cseg     00009CF3 00000003
_BlitArea                        cseg     00009CF6 00000003
_IBMSaveArea                     cseg     00009CF9 0000003F
_IBMRestoreArea                  cseg     00009D38 0000003D
_IBMBlitArea                     cseg     00009D75 000000D0
*/

#include <string.h>
#include <stdlib.h>
#include "../agi.h"
#include "../flags.h"

#include "obj_blit.h"
#include "obj_base.h"
#include "../view/obj_picbuff.h"

#include "../sys/drv_video.h"
#include "../sys/gfx.h"

// some of these aren't good names for functions
// ie.. blit_area could be more like sbuff_render_view  or something


void blit_save(BLIT *b)
{
	u8 y_count;			// ah = y, al = x
	u8 *pic_cur, *blit_cur;	// screen buff (si) and blit buf (di)
	
	pic_cur = gfx_picbuff + PBUF_MULT(b->y) + b->x;	// calc_screen_off()
	blit_cur = b->buffer;
	y_count = b->y_size;
	
	do
	{
		memcpy(blit_cur, pic_cur, b->x_size);
		pic_cur += 160;
		blit_cur += b->x_size;
		y_count--;		
	} while (y_count != 0);
}


void blit_restore(BLIT *b)
{
	u8 y_count;			// ah = y, al = x
	u8 *pic_cur, *blit_cur;	// screen buff (di) and blit buf (si)

	pic_cur = gfx_picbuff + PBUF_MULT(b->y) + b->x;	// calc_screen_off()
	blit_cur = b->buffer;
	y_count = b->y_size;
	
	do
	{
		memcpy(pic_cur, blit_cur, b->x_size);
		pic_cur += 160;
		blit_cur += b->x_size;
		y_count--;
	} while (y_count != 0);
}

void obj_blit(VIEW *v)
{
	u8 *init, *pb, *c, *temp;
	u8 cel_height;		// height of graphic
	u8 cel_invis;		// 1 invisible, 0 not
	u8 chunk;
	u8 pb_pri;
	u8 col, chunk_len, cel_tran, view_pri, ch;
		
	c = v->cel_data;		// start of cel data!!

	if (  ( c[0x2] & 0x80) != 0  )	// 1000 0000
	{
		obj_cel_mirror(v);	// mirror cell
		//c = v->cel_data;
	}
	
	c++;					// skip width of cell
	cel_height = *(c++);		// height
	cel_tran = *(c++) & 0x0F;	// transparency info
	pb = gfx_picbuff + PBUF_MULT(v->y - cel_height+1) + v->x;
	cel_invis = 1; 
	view_pri = v->priority << 4;	// priority
	init = pb;
	chunk_len = 0;

	while (cel_height != 0)
	{
		chunk = *(c++);	// get next "chunk"
		if (chunk == 0)		// eol
		{
			cel_height--;
			init += 160;
			pb = init;
		}
		else
		{
			col = chunk >> 4;	// colour	// ah
			chunk_len = chunk & 0x0F;	// length 	// al
		
			if (col == cel_tran) 	// colour is transparent
				pb += chunk_len;
			else
				do
				{
					pb_pri = *pb & 0xF0;			// get priority from gfx_picbuff
					
					if (pb_pri <= 0x20)
					{
						temp = pb;
						ch = 0;
						while ( ((temp-gfx_picbuff)<(0x6860)) && (ch<=0x20) )
						{
							temp += 160;
							ch = *temp & 0xF0;
						}
						if (ch > view_pri) 
							pb_pri = 0xFF;
					}
					else	
						if (pb_pri > view_pri) 
							pb_pri = 0xFF;
						else
							pb_pri = view_pri;
					
					if (pb_pri != 0xFF)
					{	
						*(pb++) = pb_pri|col;
						cel_invis = 0;
					}
					else
						pb++;
					
					chunk_len--;
				}
				while (chunk_len != 0);
		}
	}
	
// finish up
	if (v->num == 0)		// if view 0
	{
		if (cel_invis == 0) 
			flag_reset(1);	// not-invisible
		else
			flag_set(1);	// the invisible man
	}
}
