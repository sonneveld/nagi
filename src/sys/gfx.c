/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>

/* OTHER headers	---	---	---	---	---	---	--- */

#include "drv_video.h"
#include "vid_render.h"

#include "gfx.h"
#include "../view/obj_picbuff.h"
#include "../picture/sbuf_util.h"
#include "../sys/mem_wrap.h"

#include "chargen.h"



/* PROTOTYPES	---	---	---	---	---	---	--- */

/* VARIABLES	---	---	---	---	---	---	--- */

PCOLOUR ega_palette[] =
{ 	{0,0,0},		// black
	{0,0,170},	// dark blue
	{0,170,0},	// dark green
	{0,170,170},	// dark cyan
	{170,0,0},	// dark red
	{170,0,170}, 	// dark purple
	{170,85,0},	// dark brown
	{170,170,170},	// light grey
	{85,85,85}, 	// dark grey
	{85,85,255}, 	// light blue
	{85,255,85}, 	// light green
	{85,255,255},	// light cyan
	{255,85,85},  	// light red
	{255,85,255}, 	// light purple
	{255,255,85}, 	// light yellow
	{255,255,255}	// white
};

// same as ega atm.. must change
PCOLOUR text_palette[] =
{ 	{0,0,0},		// black
	{0,0,170},	// dark blue
	{0,170,0},	// dark green
	{0,170,170},	// dark cyan
	{170,0,0},	// dark red
	{170,0,170}, 	// dark purple
	{170,85,0},	// dark brown
	{170,170,170},	// light grey
	{85,85,85}, 	// dark grey
	{85,85,255}, 	// light blue
	{85,255,85}, 	// light green
	{85,255,255},	// light cyan
	{255,85,85},  	// light red
	{255,85,255}, 	// light purple
	{255,255,85}, 	// light yellow
	{255,255,255}	// white
};

PCOLOUR cga_0_palette[] =
{ 	{0,0,0},		// black
	{85,255,255},	// cyan
	{255,85,255},	// magenta
	{255,255,255}	// white
};

PCOLOUR cga_1_palette[] =
{ 	{0,0,170},	// blue
	{0,170,0},	// green
	{170,0,0},	// red
	{170,85,0}	// yellow
};

PCOLOUR bw_palette[] =
{	{0,0,0},		// black
	{255, 255, 255}	// white
};


VSURFACE *gfx_surface = 0;

u8 gfx_paltype = 0;
PCOLOUR *gfx_pal = 0;	// set from config
u8 gfx_palsize = 0;

//u8 gfx_scale = 1;	// set from config
//u8 gfx_fullscreen = 0;	// set from config

POS render_scale = {0, 0};

SIZE gfx_size = {320, 200};	// from render size * scale
u8 *gfx_picbuff = 0;	// created in gfx_init();
u8 gfx_picbuffrow = 0;	// set after fonts are init'd
int gfx_picbuffrotate = 0;

/* CODE	---	---	---	---	---	---	---	--- */

// gfx_init
void gfx_init(void)
{
	int i; 
	
	// gfx drvr init
	vid_init();

	// init rendererrerer
	render_init();

	// chargen init
	ch_init();
	// **************************

	// do something to calc this from render/font
	gfx_size.w = 40 * font_size.w;
	gfx_size.h = 25 * font_size.h;

	gfx_surface = vid_display(0, &gfx_size, c_vid_full_screen);

	// setup the palette
	gfx_palette_update();

	// init pic buffer
	if (gfx_picbuff == 0)
		gfx_picbuff = (u8 *)a_malloc(PICBUFF_WIDTH*PICBUFF_HEIGHT);

	sbuff_fill(0x40);
	table_init();
}



void gfx_shutdown()
{
	ch_shutdown();
	render_shutdown();
	if (gfx_surface != 0)
	{
		vid_free(gfx_surface);
		gfx_surface = 0;
	}
	vid_shutdown();
	if (gfx_picbuff != 0)
	{
		a_free(gfx_picbuff);
		gfx_picbuff = 0;
	}
}

// using available open resources.. refresh the gfx
void gfx_reinit()
{
	ch_shutdown();
	render_shutdown();
	render_init();
	ch_init();
	
	gfx_size.w = 40 * font_size.w;
	gfx_size.h = 25 * font_size.h;
	
	gfx_surface = vid_display(gfx_surface, &gfx_size, c_vid_full_screen);
	gfx_palette_update();
}

// update the render buffer onto the screen
// scaled
// ripped from old sdl code
void gfx_update(u16 rect_x, u16 rect_y, u16 rect_w, u16 rect_h)
{
	u8 *sdl_buf;
	u8 *sdl_line;
	u8 *r_buf;

	u16 sdl_x, sdl_y, sdl_w, sdl_h;
	u16 rend_x, rend_y, rend_w, rend_h;
	u16 h_count, w_count, i;

	rend_x = rect_x * rend_drv->scale_x;
	rend_y = rend_drv->scale_y*(rect_y + 1) - 1;
	rend_w = rect_w * rend_drv->scale_x;
	rend_h = rect_h * rend_drv->scale_y;

	sdl_x = rend_x * c_vid_scale;
	sdl_y = c_vid_scale*(rend_y + 1) - 1   + state.window_row_min * font_size.h;
	sdl_w = rend_w * c_vid_scale;
	sdl_h = rend_h * c_vid_scale;

	vid_lock(gfx_surface);

	r_buf = rend_buf + rend_y*rend_drv->w + rend_x;
	sdl_buf = (u8 *)gfx_surface->pixels + sdl_y*gfx_surface->line_size + sdl_x;

	for (h_count=rend_h; h_count!=0; h_count--)
	{
		// draw line
		for (w_count=rend_w; w_count!=0; w_count--)
		{
			memset(sdl_buf, (*r_buf)&0xF, c_vid_scale);
			sdl_buf += c_vid_scale;
			r_buf++;
		}

		// repeat line
		if (c_vid_scale != 1)
		{
			sdl_line = sdl_buf - rend_w*c_vid_scale;
			sdl_buf -= gfx_surface->line_size + rend_w*c_vid_scale;
			for (i=0; i<c_vid_scale-1; i++)
			{
				memcpy(sdl_buf, sdl_line, rend_w*c_vid_scale);
				sdl_buf -= gfx_surface->line_size;
			}
		}
		else
			sdl_buf -= gfx_surface->line_size*c_vid_scale + rend_w*c_vid_scale;

		// next line in buffer
		r_buf -= rend_drv->w + rend_w;
	}

	vid_unlock(gfx_surface);

	{
		POS sdl_pos = {sdl_x, sdl_y-sdl_h+1};
		SIZE sdl_size = {sdl_w, sdl_h};
		vid_update(gfx_surface, &sdl_pos, &sdl_size);
	}
}


void gfx_shake(u8 count)
{
	printf("dummy video shake\n");
	// create a new surface
	// copy screen onto surface
	// shift into corner
	// flip or something.. i dunno
}


// draws the familiar white/red boxes that sierra used in the picture buffer
void gfx_msgbox(u8 x, u8 y, u8 w, u8 h, u8 bg, u8 line)
{
	render_rect(x,y, w, h, bg);			// white
	render_rect(x+1, y-1, w-2, 1, line);	// bottom
	render_rect(x+w-2, y-2, 1, h-4, line);	// right
	render_rect(x+1, y-h+2, w-2, 1, line);	// top
	render_rect(x+1, y-2, 1, h-4, line);	// left
}

 
void gfx_palette_update(void)
{
	if (chgen_textmode)
		vid_palette_set(gfx_surface, text_palette, 16);
	else
	{
		switch (gfx_paltype)
		{
			case PAL_CUSTOM:
				vid_palette_set(gfx_surface, gfx_pal, gfx_palsize);
				break;
			case PAL_CGA0:
				vid_palette_set(gfx_surface, cga_0_palette, 4);
				break;
			case PAL_CGA1:
				vid_palette_set(gfx_surface, cga_1_palette, 4);
				break;
			case PAL_16:
			default:
				vid_palette_set(gfx_surface, ega_palette, 16);
		}
	}
}

void gfx_clear(void)
{
	POS pos = {0, 0};
	SIZE size = {0, 0};

	vid_lock(gfx_surface);
	vid_fill(gfx_surface, &pos, &size, 0);
	vid_unlock(gfx_surface);
	vid_update(gfx_surface, &pos, &size);
}

// updates the picture buffer on the screen.
void gfx_picbuff_update(void)
{
	if (gfx_picbuffrotate)
	{
		u16 i;
		u8 *pb = gfx_picbuff;
		for (i=0; i<(160*168); i++)
		{
			*pb = (*pb<<4) | (*pb>>4);
			pb++;
		}
	}

	render_update(0, 167, 160, 168);
}

