/* ----- Pic Render Routines
_RenderOverlay                   cseg     00006379 00000005
_RenderPic                       cseg     0000637E 00000030
_PicCmdLoop                      cseg     000063AE 0000001F

_EnablePicDraw                   cseg     000063CD 00000021
_DsablePicDraw                   cseg     000063EE 00000012
_EnablePriDraw                   cseg     00006400 00000026
_DsablePriDraw                   cseg     00006426 00000012

_PlotWithPen                     cseg     00006438 00000025
_PenSizeStyle                    cseg     0000645D 00000006
*_PlotRelated                     cseg     00006463 000000D9

_DrawXCorner                     cseg     0000653C 0000000F
_DrawYCorner                     cseg     0000654B 0000000F
_DrawXCorner2                    cseg     0000655A 00000025

_AbsoluteLine                    cseg     0000657F 00000018
_RelativeLine                    cseg     00006597 0000004D

_PicFill                         cseg     000065E4 0000000D

_GetPicXYPos                     cseg     000065F1 00000009
_GetPicXPos                      cseg     000065FA 00000011
_SetCarry                        cseg     0000660B 00000002
_GetPicYPos                      cseg     0000660D 0000000D

_DrawLine                        cseg     0000661A 00000096
----- END Pic Rendering  */

// *** BIG NOTE
// all picture functions must contribute to pic_Data and pic_byte!!!

#include <stdlib.h>
#include "../agi.h"

#include "../picture/sbuf_util.h"

#include "../picture/pic_res.h"
#include "../picture/pic_render.h"
#include "../sys/drv_video.h"
#include "../sys/vid_render.h"

u8 pos_init_y = 0;
u8 pos_init_x = 0;
u8 pos_final_y = 0;
u8 pos_final_x = 0;

u8 col_even = 0xff;
u8 col_odd = 0xff;
u8 sbuff_drawmask = 0;
u8 colour_pictpart=0;
u8 colour_pripart=0;
u8 *given_pic_data = 0;

u16 pen_status = 0;
s16 pen_x = 0;		// these should be u16
s16 pen_y = 0;
u16 texture_num = 0;

/*
u8 LineFinalX
u8 LineFinalY
u8 LineXIncrem
u8 LineYIncrem
u8 LineLargestSide
*/



// jumps into render_pic so it doesn't clear the screen buffer
void render_overlay()
{
	render_pic(1);
}

// render_pic()
// clears screen buff, set various variables and runs _PicCmdLoop.
// calls a HGC command (unknown which one though)
void render_pic(u8 overlay)
{
	if (overlay != 1)
	{
		// colour 15, priority 4
		// 4 = priority, F = colour
		sbuff_fill(0x4F);	// fill screen buffer
	}

	sbuff_drawmask = 0;
	pen_status = 0;		// the pen has no style.. man
	col_odd = 0xFF;		// reset to white
	col_even = 0xFF;	// reset to white

	pic_cmd_loop();

	//if (DisplayType == 2)
	//	call loc97d9;	// this is a HGC command

// goto'd in _NextPicCmd...
// loc63AD:
	return;	
}

// ----------------

u8 *pic_code;
u8 pic_byte;

// ----------------

// pic_cmd_loop()
// reads through the picture data and executes commands.
// if there's a command it doesn't understand, it skips it and keeps going
// FFh means it quits
void pic_cmd_loop()
{
	pic_code = given_pic_data;
	pic_byte = *(pic_code++);		// read next byte

	// get next command
	while (pic_byte != 0xFF) //goto 63AD;
	{
		pic_byte -= 0xF0;	// first command = 0xf0
		// Radiation: removed obvious check.
		//~ if ( (pic_byte < 0x0) || (pic_byte > 0x0A) )
		if (pic_byte > 0x0A) 	// 0xA commands possible
			pic_byte = *(pic_code++);		// read next byte
		else
		{
			switch(pic_byte)
			{
				case 0x00: enable_pic_draw();  break;
				case 0x01: disable_pic_draw();  break;
				case 0x02: enable_pri_draw();  break;
				case 0x03: disable_pri_draw();  break;
				case 0x04: draw_y_corner();  break;
				case 0x05: draw_x_corner();  break;
				case 0x06: absolute_line();  break;
				case 0x07: relative_line();  break;
				case 0x08: pic_fill();  break;
				case 0x09: read_pen_status();  break;
				case 0x0A: plot_with_pen();  break;
			}
		}
	}


	// not used?
	//pic_byte = *(pic_code++);	
	return;	
}



// 0xF0: Change picture colour and enable picture draw
void enable_pic_draw()
{
	COLOUR new_col;
	
	pic_byte = *(pic_code++);	

	// ahah.. this is some kind of dithering method
	// for the low-colour CGA people.
	// odd's and evens are drawn on different x's

	// if not cga.. ah=al;
	//ax = _CGAColourDither();
	render_colour(pic_byte, &new_col);
	colour_pictpart = pic_byte;	// equals paint colour if ega/hgc
	sbuff_drawmask = sbuff_drawmask | 0x0F;
	col_odd = (col_odd & 0xF0) | new_col.odd;	// clear lower 4 bits
	col_even = (col_even & 0xF0) | new_col.even;

	pic_byte = *(pic_code++);	
}

// 0xF1: Disable picture draw
void disable_pic_draw()
{
	sbuff_drawmask = sbuff_drawmask & 0xF0;
	col_odd = col_odd | 0x0F;	// white
	col_even = col_even | 0x0F;
	
	pic_byte = *(pic_code++);	
	return;	
}

// 0xF2: Change priority colour and enable priority draw
void enable_pri_draw()
{
	pic_byte = *(pic_code++);	

	pic_byte *= 0x10;
	colour_pripart  = pic_byte;
	sbuff_drawmask = sbuff_drawmask | 0xF0;
	col_odd = (col_odd & 0x0F) | pic_byte;
	col_even = (col_even & 0x0F) | pic_byte;

	pic_byte = *(pic_code++);	
	return;	
}


// 0xF3: Disable priority draw
void disable_pri_draw()
{
	sbuff_drawmask = sbuff_drawmask & 0x0F;
	col_odd = col_odd | 0xF0;
	col_even = col_even | 0xF0;

	pic_byte = *(pic_code++);	
	return;	
}






// 0xFA: Plot with pen
void plot_with_pen()
{
	u8 xx, yy;
	//printf("pen plot.. incomplete \n");
loc6438:
	// solid(0) vs splater(1) (20h)
	if ((pen_status & 0x20) != 0)
	{
		pic_byte = *(pic_code++);	
		if (pic_byte >= 0xF0)
			return;
		texture_num = pic_byte;
	}
	
	if (read_xy_pos(&xx, &yy) == 1)
		return;
	pen_x = xx;
	pen_y = yy;
	
	//(push si)
		plot_with_pen_2();
	//(pop si)
	goto loc6438;
}

// 0xF9: Change pen size and style
void read_pen_status()
{	
	pen_status = *(pic_code++);
	pic_byte = *(pic_code++);
	return;
}




u16 binary_list[] = {0x8000, 0x4000, 0x2000, 0x1000, 0x800, 0x400, 0x200, 0x100, 
			0x80, 0x40, 0x20, 0x10, 0x8, 0x4, 0x2, 0x1};

u8 circle_list[] = {0, 1, 4, 9, 16, 25, 37, 50};
u16 circle_data[] =
{0x8000, 
0x0E000, 0x0E000, 0x0E000, 
0x7000, 0xF800, 0x0F800, 0x0F800, 0x7000, 
0x3800, 0x7C00, 0x0FE00, 0x0FE00, 0x0FE00, 0x7C00, 0x3800, 
0x1C00, 0x7F00, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x0FF80, 0x7F00, 0x1C00,
0x0E00, 0x3F80, 0x7FC0, 0x7FC0, 0x0FFE0, 0x0FFE0, 0x0FFE0, 0x7FC0, 0x7FC0, 0x3F80, 0x1F00, 0x0E00,
0x0F80, 0x3FE0, 0x7FF0, 0x7FF0, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x0FFF8, 0x7FF0, 0x7FF0, 0x3FE0, 0x0F80,
0x07C0, 0x1FF0, 0x3FF8, 0x7FFC, 0x7FFC, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x0FFFE, 0x7FFC, 0x7FFC, 0x3FF8, 0x1FF0, 0x07C0};
				
// called by plot with pen.
void plot_with_pen_2()
{
	u16 circle_word;
	u16 *circle_ptr;	// si
	u16 counter;	// bx sometimes
	u16 pen_width = 0;
	u16 pen_final_x = 0;	// 15EA
	u16 pen_final_y = 0;	// 15EC
	
	u8 t = 0;
	u8 temp8;
	u16 temp16;

	circle_ptr = &circle_data[ circle_list[(pen_status & 0x07)] ];	// pen size
	
	// setup the X position
	// = pen_x - pen.size/2

	pen_x = (pen_x * 2) - (pen_status & 0x07);
	if (pen_x < 0) pen_x = 0;

	temp16 = 0x140 - (2 * (pen_status & 0x07));
	if (pen_x >= temp16)
		pen_x = temp16;
		
	pen_x /= 2;
	pen_final_x = pen_x;	// original starting point?? -> used in plotrelated

	// Setup the Y Position
	// = pen_y - pen.size
	pen_y = pen_y - (pen_status & 0x07);
	if (pen_y < 0) pen_y = 0;

	temp16 = 0xA7 - (2 * (pen_status & 0x07));
	if (pen_y >= temp16)
		pen_y = temp16;
		
	pen_final_y = pen_y;	// used in plotrelated

	t = texture_num | 0x01;		// even
	
	// new purpose for temp16
	
	temp16 =( (pen_status & 0x07)<<1) +1;	// pen size
	pen_final_y += temp16;					// the last row of this shape
	temp16 = temp16 << 1;
	pen_width = temp16;					// width of shape?

	//printf("status = %X   pen_width = %d  pen_final_y = %d  pen_x = %d   pen_y = %d\n", pen_status, pen_width, pen_final_y, pen_x, pen_y);
	
	//printf("width = %d     ", pen_width);
	
loc64DB:			// new y
	counter = 0;
	circle_word = *circle_ptr;
	circle_ptr += 1;
loc64DF:			// new x
	if (   ((pen_status&0x10) != 0) || ( (binary_list[counter>>1] & circle_word) != 0)   )
	{
		if ( (pen_status & 0x20) == 0) goto loc6506;			// skip last column??  not sure

		temp8 = t % 2;
		t = t >> 1;
		if (temp8 != 0) 
			t = t ^ 0xB8;

		if ((t & 0x01) != 0) goto loc651b;
		if ((t & 0x02) == 0) goto loc651b;
	loc6506:
		//(push dx, ax, bx, si)
		pos_init_y = pen_y;
		pos_init_x = pen_x;
		sbuff_plot();
		//(pop si, bx, ax, dx)
	}
loc651b:
	pen_x++;
	counter += 4;	// needs to be 4 or the width will be wrong
	if (counter <= pen_width)
		goto loc64DF;
	pen_x = pen_final_x;
	pen_y++;
	if (pen_y != pen_final_y)
		goto loc64DB;
	return;
}



















// 0xF6: Absolute line
void absolute_line()
{	
	if (read_xy_pos(&pos_init_x, &pos_init_y) != 1)
	{
		sbuff_plot();
		
		while (read_xy_pos(&pos_final_x, &pos_final_y) != 1)
		draw_line();
	}
}


	
	

// 0xF5: Draw an X corner
void draw_x_corner()
{
	if (read_xy_pos(&pos_init_x, &pos_init_y) != 1)
	{
		sbuff_plot();
		draw_corner(0);
	}
}

// 0xF4: Draw a Y corner
void draw_y_corner()
{
	if (read_xy_pos(&pos_init_x, &pos_init_y) != 1)
	{
		sbuff_plot();
		draw_corner(1);
	}
} 

// 0 = x corner
// 1 = y corner
void draw_corner(u8 type)
{
	u8 pos;
	u8 orig_x, orig_y;
	if (type == 1)
		goto draw_y;	
draw_x:	

	
	if (get_x_pos(&pos) == 1)
		return;

	pos_final_x = pos;
	pos_final_y = pos_init_y;	// y
	orig_x = pos_final_x;
	orig_y = pos_final_y;
	sbuff_xline();
	pos_init_y = orig_y;
	pos_init_x = orig_x;

draw_y:
	if (get_y_pos(&pos) == 1)
		return;

	pos_final_y = pos;
	pos_final_x = pos_init_x;
	orig_x = pos_final_x;
	orig_y = pos_final_y;
	sbuff_yline();
	pos_init_y = orig_y;
	pos_init_x = orig_x;
	goto draw_x;
}
	
	
	
	
	
	
	
	
	
	
void relative_line()
{
	u8 x_pos, y_pos;
	u8 x_step, y_step;	// x = bh, y = bl;
	u8 pos_data;
	
	//printf("relative line.. incomplete \n");
	
	if (read_xy_pos(&pos_init_x, &pos_init_y) == 1)
		return;
	sbuff_plot();

	loc65a2:
	pic_byte = *(pic_code++);
	pos_data = pic_byte ;
	if (pos_data > 0xEF)
	{
		//printf("nope.. outta here\n");
		return;
	}
	
	x_step = pos_data;
	x_pos = pos_init_x;
	y_pos = pos_init_y;
	
	x_step = (x_step & 0x70) / 16;		// x
	if ( (pos_data & 0x80) == 0)	// sign
		x_pos += x_step;
	else
		x_pos -= x_step;

	if (x_pos > 0x9F)
		x_pos = 0x9F;

	y_step = pos_data;
	y_step = y_step & 0x7;
	
	if ( (pos_data & 0x08) == 0)	// sign
		y_pos += y_step;
	else
		y_pos -= y_step;

	if ( y_pos > 0xA7)
		y_pos = 0xA7;

	pos_final_x = x_pos;
	pos_final_y = y_pos;

	draw_line();

	goto loc65a2;
	
}





// 0xF8: Fill
void pic_fill()
{
	while (read_xy_pos(&pos_init_x, &pos_init_y) != 1)
		sbuff_picfill(pos_init_y, pos_init_x);
}


// reads the position from the pic file data
// puts read positions into x and y mem ptrs
// returns 0 if successful
int read_xy_pos(u8 *x, u8 *y)
{
	if(get_x_pos(x) == 1)
		return 1;
	return(get_y_pos(y));
}

int get_x_pos(u8 *x)
{
	pic_byte = *(pic_code++);
	*x = pic_byte;

	if ( *x > 0xEF)	// command
		return 1;
	if ( *x > 159)	// 159
		*x = 159;
	return 0;
}

int get_y_pos(u8 *y)
{
	pic_byte = *(pic_code++);
	*y = pic_byte;
	
	if ((*y) > 0xEF)	// command
		return 1;
	if ((*y) > 167)	// 167
		*y = 167;
	return 0;
}



void draw_line()
{
	//u8 line_x_final;	// these final are ignored in this or any function
	//u8 line_y_final;	// I'll put them in if necessary
	s16 line_x_inc;
	s16 line_y_inc;
	u8 line_largest;	// largest side

	// register replacements
	s16 x_component; // dh
	s16 y_component; // dl
	u8 counter; // cx
	u8 pos_x; // bl
	u8 pos_y; // bh
	s16 y_count;	// ah
	s16 x_count;	// al

	pos_y = pos_init_y;	// y
	pos_x = pos_init_x;

	// if straight line.. call the straight line function
	if (pos_y == pos_final_y)
	{
		sbuff_xline();
		return;
	}
	else if (pos_x == pos_final_x)
	{
		sbuff_yline();
		return;
	}

	line_y_inc = 0x1;
	y_component = pos_final_y - pos_init_y;	// y
	if (y_component < 0)
	{
		line_y_inc *= -1;
		y_component *= -1;
	}

	line_x_inc = 0x1;
	x_component = pos_final_x - pos_init_x;
	if (x_component < 0)
	{
		line_x_inc *= -1;
		x_component *= -1;
	}

	if (x_component >= y_component)
	{
		counter = x_component;
		line_largest = x_component;
		y_count = x_component/2;
		x_count = 0;
	}
	else
	{
		counter = y_component;
		line_largest = y_component;
		x_count = y_component/2;
		y_count = 0;
	}

	do
	{
		y_count += y_component;
		if (y_count >= line_largest)
		{
			y_count -= line_largest;
			pos_y += line_y_inc;
		}
	
		x_count += x_component;
		if (x_count >= line_largest)
		{
			x_count -= line_largest;
			pos_x += line_x_inc;
		}
	
		//(push ax, bx, cx, dx)

		pos_init_y = pos_y;	// y;
		pos_init_x = pos_x;
		sbuff_plot();

		//(pop dx, cx, bx, ax)

		counter--;
	} while (counter != 0);
}



