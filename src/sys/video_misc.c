/*
_VideoInit                       cseg     00005460 0000001E
_pic_buff_update                        cseg     0000547E 0000004A
_BoxNBorder                      cseg     000054C8 0000007C
_PutBlock                        cseg     00005544 00000018
_FBuff_Offset                    cseg     0000555C 00000042
_SBuff_Offset                    cseg     0000559E 0000001F
_CGAColorDither                  cseg     000055BD 0000001D

*/

#include <stdlib.h>

#include "../agi.h"
#include "../sys/video.h"

#include "../sys/video_misc.h"
#include "../view/obj_picbuff.h"
#include "../picture/sbuf_util.h"

u16 pic_buff_rotate = 0; 

// changes to the correct video mode
// clears picture buffer
// initialises pri table
void graf_init(void)
{
	vid_init();
	sbuff_fill(0x40);
	table_init();
	vid_clear();
}

// updates the picture buffer on the screen.
void pic_buff_update()
{
	if (pic_buff_rotate != 0)
	{
		u16 i;
		u8 *pb = vstate.pic_buf;
		for (i=0; i<(160*167); i++)
			*(pb++) = (*pb<<4) | (*pb>>4);
	}
	
	render_update(0, 167, 160, 168);
}

// draws the familiar white/red boxes that sierra used in the picture buffer
void box_n_border(u8 x, u8 y, u8 w, u8 h, u8 bg, u8 line)
{
	render_rect(x,y, w, h, bg);			// white
	render_rect(x+1, y-1, w-2, 1, line);		// bottom
	render_rect(x+w-2, y-2, 1, h-4, line);	// right
	render_rect(x+1, y-h+2, w-2, 1, line);	// top
	render_rect(x+1, y-2, 1, h-4, line);		// left
}



/*
calc screen offset
di = 160 * y + x;

frame buff off:
di = (word136F + y)*40 + x/4;
// it's x/4 because 
// 1) double pixels
// 2) 8 pixels in one byte.. it's planar

// * 40 because it's 40 bytes per line
*/

/*
Uint16 *fbuff_offset(u8 x, u8 y)
{
	return (Uint16 *)screen->pixels + (y+pic_top_line)*screen->pitch/2 + x*2;
}

u8 *sbuff_offset(u8 x, u8 y)
{
	return pic_buff + 160*y + x;
	di = sbuf+blah;
}
*/

/*
void cga_colour_dither()
*/


