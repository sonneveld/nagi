/* FUNCTION list 	---	---	---	---	---	---	---
_TableInit                       cseg     000055DA 00000016
_ReadObjStatus                   cseg     000055F0 000000AA
_DrawNewCel                      cseg     0000569A 0000006D
_DrawViewPri                     cseg     00005707 000000AE
_BlitMirrorCell                  cseg     000057B5 000000A2
sub_cseg_5857                    cseg     00005857 0000001B
_Draw5872                        cseg     00005872 000000DA
_Draw594C                        cseg     0000594C 0000004A
_VidMode_Set                     cseg     00005996 00000016
_VidMode_Get                     cseg     000059AC 00000015
*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "../view/obj_picbuff.h"
#include "../flags.h"
#include "../view/obj_proximity.h"
#include "../view/obj_blit.h"
#include "../picture/pic_res.h"
#include "../sys/drv_video.h"
#include "../sys/gfx.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
u16 obj_chk_walk_area(VIEW *v);


/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

// 0xFF is used in v3 to determine if it's been initialised.  
// not used in v2 but it's not important
u8 pri_table[172] = {0xFF, 0};

// initialises the priority table used to determine the ego's priority at a certain ypos
void table_init()
{
	if (pri_table[0] == 0xFF)
	{
		u8 pri;
		u8 *table;
		
		memset(pri_table, 4, 48);
		
		table = pri_table + 48;
		for (pri=5; pri<15; pri++)
		{
			memset(table, pri, 12);
			table += 12;
		}
	}
}

// reads the base line of the obj and checks for water / alarms/ obstacles .. stuff like that
// returns 1 if not on a control line
// returns 0 if it is
// sets flag 0 and 3 if it's the ego
// interesting things... agi only acknowledges water if it's the ONLY priority thingy.. ie, you have to
// be completely immersed in the stuff
u16 obj_chk_control(VIEW *v)
{
	u8 *pb;
	u16 flag_control, flag_water, flag_signal;	// flag_control = di;, flag_water = bl  flag_signal = bh;
	u8 pri;
	u16 cx;
	
	if ( (v->flags & O_PRIFIXED) == 0)
		v->priority = pri_table[v->y];
	
	pb = gfx_picbuff + PBUF_MULT(v->y) + v->x;
	flag_water = 0;
	flag_signal = 0;

	cx = v->cel_data[0];	// cel width
	flag_control = 1;

	if (v->priority != 0x0F)
	{
		flag_signal = 0;
		flag_water = 1;

		do
		{
			pri = *(pb++) & 0xF0;
			if (pri == 0)	// obstacle
			{
				flag_control = 0;
				goto check_finish;
			}	
			if ( pri != 0x30) 				// water
			{
				flag_water = 0;			// we're not on water sorry
				if ( pri == 0x10)			// conditional 
				{
					if ( ( v->flags&O_BLOCKIGNORE) == 0) // observe blocks
					{
						flag_control = 0;
						goto check_finish;
					}
				}
				else if (pri == 0x20)
					flag_signal = 1;	// alarm
			}
			cx--;
		} while (cx != 0);
		
		if (flag_water != 1)
		{
			if ( (v->flags&O_WATER) != 0)	// view on water = 1
				flag_control = 0;
		}
		else if ( (v->flags&O_LAND) != 0) 
			flag_control = 0;	// view on land = 1
	}

check_finish:	

	if ( v->num == 0)
	{
		if (flag_signal == 0)
			flag_reset(F03_EGOSIGNAL);
		else
			flag_set(F03_EGOSIGNAL);
		if (flag_water == 0)
			flag_reset(F00_EGOWATER);
		else
			flag_set(F00_EGOWATER);
	}

	return flag_control;
}

// updates the screen with new cel
// determines the exact coordinates on the pbuff to remove old and draw new cel
void obj_cel_update(VIEW *v)
{
	u8 *c; //, *c_prev;	// removed because of kq4 bug
	s16 x, y, w, h;
	u8 c_prev_h, c_prev_w;

	//al = y;    ah = x;    bl = width    bh = height
	
	if (pic_visible == 0)
		return;
	
	c = v->cel_data;
	c_prev_h = v->cel_prev_height;
	c_prev_w = v->cel_prev_width;
	//c_prev = v->cel_data_prev;	// removed because of kq4 bug
	v->cel_prev_height = v->cel_data[1];
	v->cel_prev_width = v->cel_data[0];
	//v->cel_data_prev = c;	// kq4 bug
	
	{
		s16 y2, h1, h2;
		if (v->y < v->y_prev)
		{
			y = v->y_prev;
			y2 = v->y;

			h1 = c_prev_h;
			h2 = c[1];	
		}
		else
		{
			y = v->y;
			y2 = v->y_prev;
			h1 = c[1];		// height
			h2 = c_prev_h;	 // height
		}
		
		if ((y2-h2) > (y-h1))
			h = h1;
		else
			h = y - y2 + h2;
	}
	
	{
		s16 x2, w1, w2;

		if (v->x > v->x_prev)
		{
			x = v->x_prev;
			x2 = v->x;
			w1 = c_prev_w;
			w2 = c[0];
		}
		else
		{
			x = v->x;
			x2 = v->x_prev;
			w1 = c[0];		// width
			w2 = c_prev_w;	// width
		}
		
		if ((x2+w2) < (x+w1))
			w = w1;
		else
			w = w2 + x2-x;
	}

	if ( (x+w) > 161)  // if ( x+w-1 > 160)
		w = 161-x;
	if (1 < (h-y))  // if(y-h+1 < 0)
		h = y+1;
	
	render_update(x, y, w, h);
}

// draws the priority of the view for add.to.pic
// adds a small box at the bottom so ego can't go through it or weird priority errors
void obj_add_pic_pri(VIEW *v)
{
	u8 *pb;		// pointer to the pic buff
	u8 pri_height;	// height of the priority band used
	u8 sideoff;		// offset to the right side of pri box
	s16 cx;		// general counter
	u8 height;		// height of the box
	
	if ((v->priority & 0x0F) == 0)
		v->priority = v->priority | pri_table[v->y];
	obj_blit(v);
	
	if (v->priority > 0x3F)
		return;	
	
	// count up from current priority to find the size of the box that is at the view's feet
	// this prevents the ego from walking into a different priority or walking through the view.
	cx = v->y;
	pri_height = 0;
	do
	{
		pri_height++;
		if (cx <= 0) 
			break;
		cx--;
	}
	while (pri_table[cx] == pri_table[v->y]);

	height = (v->cel_data)[1];	// height
	if ( (height > pri_height))
		height = pri_height;
	
	// draw the box  -------------------------------
	pb = gfx_picbuff + PBUF_MULT(v->y) + v->x;
	
	// bottom line
	cx = (v->cel_data)[0];
	do
	{
		*pb &= 0x0F;
		*(pb++) |= (v->priority&0xF0);
		cx--;
	} while (cx > 0);

	// if there's a height..we'll build it.. or something
	if (height > 1) 
	{
		pb = gfx_picbuff + PBUF_MULT(v->y) + v->x;
		
		// the sides
		sideoff = (v->cel_data)[0] - 1;
		cx = height-1;
		do
		{
			pb -= 160;
			*pb &= 0x0F;				// left
			*pb |= (v->priority&0xF0);
			pb[sideoff] &= 0x0F;			// right
			pb[sideoff] |= (v->priority&0xF0);
			cx--;
		} while (cx > 0);
		
		// the top of the box
		cx = (v->cel_data)[0] - 2;
		do
		{				
			pb++;
			*pb &= 0x0F;
			*pb |= (v->priority&0xF0);
			cx--;
		} while (cx > 0);
	}
}

// mirror the current cel in the view
void obj_cel_mirror(VIEW *v)
{
	s8 tran_size;	// in pixels.. size of transparent start (veg) and other data (meat)
	u8 meat_size;	// in pixels.. size of data (meat)
	u8 al;
	u16 h_count;
	
	u8 buff_in_stack[0x800];
	u8 *buff;
	u8 *cel;
	u8 *cel_orig;
	u8 width;
	u8 trans;
	
	cel = v->cel_data;
	// cel[2] = cel info
	// number of loop NOT mirrored
	if ( ((cel[2]&0x70)>>4) != v->loop_cur )
	{
		// change the loop contained in the cel info
		cel[2] = (0x8F & cel[2]) | ( (v->loop_cur) << 4);	// ~ 0x70

		buff = buff_in_stack;
		width = *(cel++);	// width
		h_count = *(cel++);	// height
		trans = *(cel++) << 4;	// trans colour left shifted
		
		do
		{
			meat_size = 0;
			tran_size = width;
			
			// read in the first couple of "assumed" transparent characters
			// if you reach an end of line then it's an empty line.. so just put in a 0
			al = *(cel++);
			while (  (al != 0) && ((al&0xF0)==trans)  )
			{
				tran_size -= al&0x0F;
				al = *(cel++);
			}
			
			if (al != 0)
			{
				al &= 0x0F;	// size
				// determine the length of the actual data
				do
				{
					tran_size -= al;
					meat_size++;
					al = *(cel++)&0x0F;	// size
				}
				while (al != 0);
				
				// fill in a big bunch of transparent pixels
				while (tran_size >= 0x0F)	// full 15 pixel transparency
				{
					*(buff++) = trans|0x0F;
					tran_size -= 0x0F;
				}
				if (tran_size > 0)			// the bits left over
					*(buff++) = trans|tran_size;
				
				// draw the meat in reverse
				cel_orig = cel;
				cel -= 2;	// skip the current chunk and the '0' chunk
				while (meat_size > 0)
				{
					*(buff++) = *(cel--);
					meat_size--;
				}
				cel = cel_orig;
			}

			// end of the line
			*(buff++) = 0;
			h_count--;
		}
		while (h_count != 0);
		
		// copy the buffer into the cel.
		memcpy(v->cel_data+3, buff_in_stack, buff - buff_in_stack);
	}
}
	
// shifts the view around in a sprial under it's in a walkable area, not contacting anything and 
// not on a control line
// counter-clockwise
void obj_pos_shuffle(VIEW *v)
{
	u16 shift_count;	// countdown until next shift
	u16 shift_dir;	// shift direction
	u16 shift_size;	// size of the shift until next time
	
	if (  (v->y <= state.horizon) && ((v->flags&O_HORIZONIGNORE) == 0)  )
		v->y = state.horizon+1;

	if (obj_chk_walk_area(v) != 0)	// walkable
		if (obj_chk_contact(v) == 0)	// no contact with other obj
			if (obj_chk_control(v) != 0)	// no control line
				return;

	shift_dir = 0;
	shift_count = 1;
	shift_size = 1;
			
	while ( (obj_chk_walk_area(v)==0) || (obj_chk_contact(v)!=0) || (obj_chk_control(v)==0) )
		switch(shift_dir)
		{
			case 0:		// left
				v->x--;
				shift_count--;
				if (shift_count == 0)
				{
					shift_dir = 1;
					shift_count = shift_size;
				}
				break;
			case 1:		// down
				v->y++;
				shift_count--;
				if (shift_count == 0)
				{
					shift_dir = 2;
					shift_size++;
					shift_count = shift_size;
				}
				break;
			case 2:		// right
				v->x++;
				shift_count--;
				if (shift_count == 0)
				{
					shift_dir = 3;
					shift_count = shift_size;
				}
				break;
			case 3:		// up
				v->y--;
				shift_count--;
				if (shift_count == 0)
				{
					shift_dir = 0;
					shift_size++;
					shift_count = shift_size;
				}
				break;
		}
}

// if the object is within the walking area (including horizon if needed) then return 1
// else return 0
u16 obj_chk_walk_area(VIEW *v)
{
	if (v->x < 0) return 0;
	if ((v->x + v->x_size) > 160 ) return 0;
	if ((v->y - v->y_size) < -1) return 0;
	if (v->y > 167) return 0;
	if ( ((v->flags&O_HORIZONIGNORE)==0) && (v->y<=state.horizon) )
		return 0;
	
	return 1;	// object is within walking area
}
