/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);
#include "video.h"
#include "vid_render.h"

#include "vid_sdl.h"
#include "vid_dummy.h"


/* VARIABLES	---	---	---	---	---	---	--- */



/* CODE	---	---	---	---	---	---	---	--- */

VDRIVER video_driver_sdl =
{
	VID_SDL,
	sdlvid_init, sdlvid_shutdown,
	sdlvid_update, sdlvid_clear, sdlvid_shake, 
	sdlvid_pos_get, sdlvid_pos_set,
	sdlvid_char_attrib, sdlvid_char_put,
	sdlvid_scroll, sdlvid_char_clear
};

VDRIVER video_driver_dummy =
{
	VID_NONE,
	dummyvid_init, dummyvid_shutdown,
	dummyvid_update, dummyvid_clear, dummyvid_shake, 
	dummyvid_pos_get, dummyvid_pos_set,
	dummyvid_char_attrib, dummyvid_char_put, 
	dummyvid_scroll, dummyvid_char_clear
};

VSTATE vstate =
{
	&video_driver_sdl,  // driver
	0,	// pal
	16,	// pal_size
 	1,	// scale
	1,	// scale_fill
	0,	// full screen
	0,	// w
	0,	// h
	0,	// pic_buf
	0,	// pic_buf_row
	0 // textmode
};


void vid_init()
{
	// init pic buffer
	if (vstate.pic_buf == 0)
		vstate.pic_buf = (u8 *)a_malloc(160*168);
	
	// init rendererrerer
	render_init();
	// init video (scale x size of renderer)
	// init fonts
	vstate.drv->init(&rstate);
}

void vid_shutdown(void)
{
	vstate.drv->shutdown();
}

// update whatever's in rstate.buff to the screen
void vid_update(u16 x, u16 y, u16 width, u16 height)
{
	vstate.drv->update(x, y, width, height);
}

void vid_clear()
{
	vstate.drv->clear();
}

// shake the render buff around.. show it who's da boss
void vid_shake(u8 count)
{
	vstate.drv->shake(count);
}

void t_pos_get(POS *pos)
{
	vstate.drv->t_pos_get(pos);
}

void t_pos_set(POS *pos)
{
	vstate.drv->t_pos_set(pos);
}

void t_char_attrib(u8 colour, u16 flags)
{
	vstate.drv->t_char_attrib(colour, flags);
}

void t_char_put(u8 ch)
{
	vstate.drv->t_char_put(ch);
}

void t_scroll(POS *pos1, POS *pos2, u16 scroll, u8 attrib)
{
	vstate.drv->t_scroll(pos1,pos2, scroll, attrib);
}


void t_clear(POS *pos1, POS *pos2, u8 attrib)
{
	vstate.drv->t_clear(pos1,pos2,  attrib);
}