/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"


/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>

/* OTHER headers	---	---	---	---	---	---	--- */
#include "../sys/mem_wrap.h"
#include "../sys/endian.h"

#include "drv_video.h"
#include "vid_render.h"
#include "gfx.h"
/* PROTOTYPES	---	---	---	---	---	---	--- */
void ega_update(u16 x, u16 y, u16 width, u16 height);
void cga_update(u16 x, u16 y, u16 width, u16 height);
void dummy_update(u16 x, u16 y, u16 width, u16 height);

void ega_rect(u16 x, u16 y, u16 width, u16 height, u8 colour);
void cga_rect(u16 x, u16 y, u16 width, u16 height, u8 colour);
void dummy_rect(u16 x, u16 y, u16 width, u16 height, u8 colour);

void dummy_view_dither(u8 *view_data);
void cga_view_dither(u8 *view_data);	// dither view



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

RDRIVER render_drv_cga0 = 
{
	R_CGA0, 0,
	320, 168, 2,1, 
	cga_update, cga_rect,
	render_colour, cga_view_dither
};

RDRIVER render_drv_cga1 = 
{
	R_CGA1, 1, 
	320, 168, 2,1, 
	cga_update, cga_rect,
	render_colour, cga_view_dither
};

RDRIVER render_drv_ega = 
{
	R_EGA, 0,
	320, 168, 2,1, 
	ega_update, ega_rect,
	render_colour, dummy_view_dither
};

RDRIVER render_drv_dummy = 
{
	R_NONE, 0,
	0, 0, 1,1, 
	dummy_update, dummy_rect,
	render_colour, dummy_view_dither
};
	
RSTATE rstate = {&render_drv_ega,0,0};

/*
rend_state
rend_buff
rend_buff_size

rend_width
rend_height
rend_pal_type
rend_x_scale
rend_y_scale
*/

// init or reread new settings
void render_init()
{
	// free buffer if it already exists
	render_shutdown();

	rstate.buf_size = rstate.drv->w * rstate.drv->h;
	rstate.buf = a_malloc(rstate.buf_size);
	memset(rstate.buf, 0, rstate.buf_size);
	
	// set variables.
	state.var[V26_MONITORTYPE] = rstate.drv->agi_type;
	
	// only here so games that only setup ctrl-R for cga games work
	if (state.var[V26_MONITORTYPE] > 1)
		state.var[V26_MONITORTYPE] = 0;
}

void render_shutdown()
{	
	if (rstate.buf != 0)
	{
		a_free(rstate.buf);
		rstate.buf = 0;
		rstate.buf_size = 0;
	}
}



// ---------- UPDATE -------------------------

void render_update(u16 x, u16 y, u16 width, u16 height)
{
	rstate.drv->func_update(x, y, width, height);
	gfx_update(x, y, width, height);
}

void ega_update(u16 x, u16 y, u16 width, u16 height)
{
	u8 *pbuf, *rbuf;
	int w, h;
	
	pbuf = gfx_picbuff + 160*y + x;
	rbuf = rstate.buf + y*rstate.drv->w + x*2;
	
	for (h=height ; h!=0 ; h--)
	{
		for (w=width ; w!=0 ; w--)
		{ 
			*(rbuf++) = *(pbuf)&0xF;
			*(rbuf++) = *(pbuf++)&0xF;
		}
		pbuf -= 160 + width;
		rbuf -= rstate.drv->w + width*2;
	}
	
}

void cga_update(u16 x, u16 y, u16 width, u16 height)
{
	u8 *pbuf, *rbuf;
	int w, h;
	
	pbuf = gfx_picbuff + 160*y + x;
	rbuf = rstate.buf + y*rstate.drv->w + x*2;
	
	for (h=height ; h!=0 ; h--)
	{
		for (w=width ; w!=0 ; w--)
		{ 
			*(rbuf++) = (*pbuf & 0xC)>>2;
			*(rbuf++) = *pbuf & 0x3;
			pbuf++;
		}
		pbuf -= 160 + width;
		rbuf -= rstate.drv->w + width*2;
	}
}

void dummy_update(u16 x, u16 y, u16 width, u16 height)
{
	(void) x;
	(void) y;
	(void) width;
	(void) height;
	printf("render dummy update()\n");
}

// ---------- RECT (ally) -------------------------

// render clear can be replaced by this
void render_rect(u16 x, u16 y, u16 width, u16 height, u8 colour)
{
	rstate.drv->func_rect(x, y, width, height, colour);
	
	gfx_update(x, y, width, height);
}


void ega_rect(u16 x, u16 y, u16 width, u16 height, u8 colour)
{
	u8 *rbuf;
	int h;
	
	rbuf = rstate.buf + y*rstate.drv->w + x*2;
	
	for (h=height ; h>0 ; h--)
	{
		memset(rbuf, colour&0xF, width*2);
		rbuf -= rstate.drv->w;
	}
}

void cga_rect(u16 x, u16 y, u16 width, u16 height, u8 colour)
{
	u8 *rbuf;
	COLOUR rend_col;
	u16 h_count, w_count;
	
	rbuf = rstate.buf + y*rstate.drv->w + x*2;
	
	// get cga colour!!
	// uses odd AND even colours
	render_colour(colour&0xF, &rend_col);
	
	if ((width & 1) != 0)
	{
		for (h_count=height; h_count!=0; h_count--)
		{
			*(rbuf++) = (rend_col.odd & 0xC)>>2;
			*(rbuf++) = rend_col.odd & 0x3;
			for (w_count=(width-1)/2; w_count!=0; w_count--)
			{
				*(rbuf++) = (rend_col.even& 0xC)>>2;
				*(rbuf++) = rend_col.even & 0x3;
				*(rbuf++) = (rend_col.odd & 0xC)>>2;
				*(rbuf++) = rend_col.odd & 0x3;
			}
			rbuf -= rstate.drv->w + width*2;
		}
	}
	else
	{
		for (h_count=height; h_count!=0; h_count--)
		{
			for (w_count=width/2; w_count!=0; w_count--)
			{
				*(rbuf++) = (rend_col.even& 0xC)>>2;
				*(rbuf++) = rend_col.even & 0x3;
				*(rbuf++) = (rend_col.odd & 0xC)>>2;
				*(rbuf++) = rend_col.odd & 0x3;
			}
			rbuf -= rstate.drv->w + width*2;
		}
	}

}

void dummy_rect(u16 x, u16 y, u16 width, u16 height, u8 colour)
{
	(void) x;
	(void) y;
	(void) width;
	(void) height;
	(void) colour;
	printf("render dummy rect\n");
}

// ---------- DITHERING -------------------------

// PIC DITHERING
u8 cga_colour_pal[]={0x00, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x04,
				0x05, 0x03, 0x01, 0x00, 0x04, 0x0A, 0x0A, 0x06,
				0x03, 0x0C, 0x08, 0x0B, 0x0E, 0x05, 0x03, 0x04,
				0x0A, 0x04, 0x03, 0x07, 0x0D, 0x00, 0x09, 0x01,
				0x04, 0x0B, 0x05, 0x05, 0x0E, 0x0E, 0x0E, 0x0C,
				0x02, 0x08, 0x0D, 0x0D, 0x07, 0x0F, 0x0F, 0x0F};

void render_colour(u8 col, COLOUR *col_dith)
{
	switch (rstate.drv->type)
	{
		case R_CGA0:
			col_dith->odd = cga_colour_pal[3*col];
			col_dith->even =  col_dith->odd;
			break;
		case R_CGA1:
			col_dith->odd =  cga_colour_pal[3*col + 1];
			col_dith->even =  cga_colour_pal[3*col + 2];
			break;
		default:
			col_dith->odd = col;
			col_dith->even = col;
			break;
	}
}


//(black-cyan-magenta-white CGA palette)
// display == 0
u8 cga0_view_pal[]={0x00, 0x22, 0x11, 0x33, 0x44, 0x66, 0x88, 0x55,
				0xAA, 0x77, 0x99, 0xBB, 0xEE, 0xCC, 0xDD, 0xFF};
	
//(blue-yellow-red-green CGA palette)
// display == 1
u8 cga1_view_pal[]={0x00, 0x00, 0xCC, 0x11, 0xAA, 0x22, 0x99, 0xDD,
				0x00, 0x33, 0x55, 0x77, 0xEE, 0xEE, 0xFF, 0xFF};

// copy of cga1_view_pal.. it's used as a buffer.. because one of the colours
// gets changed to the transparent colour (only in this mode (1)though)
u8 cga1_view_buff[]={0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

void render_view_dither(u8 *view_data)
{
	rstate.drv->func_view_dither(view_data);
}

void dummy_view_dither(u8 *view_data)
{
	// oh wait.. I don't DO anything!
	(void) view_data;
}
				
void cga_view_dither(u8 *view_data)	// dither view
{
	u16 loop_prev;	// checks to make sure you're not mirroring a previous loop	
	u8 *loop_table;
	u8 loop_count;
	
	loop_prev = 0;
	if (rstate.drv->type == R_CGA1)
		memcpy(cga1_view_buff, cga1_view_pal, 16);
	loop_count = view_data[2];	// number of loops
	loop_table = view_data + 5;
	
	while (loop_count != 0)
	{
		if (load_le_16(loop_table) != loop_prev)
		{
			u8 *loop_data;
			u8 cel_count;	// cx
			
			loop_prev = load_le_16(loop_table);
			loop_data = view_data + loop_prev;		// si points to current loop data
			cel_count = *(loop_data++);
			
			while (cel_count != 0)
			{
				u8 *si;
				u8 line_count;
				u8 *palette;	// 16 colour ega -> 4 colour cga
				
				si = view_data + loop_prev + load_le_16(loop_data);	// loop start + position of cel
				loop_data += 2;
				
				si++;		// skip width
				line_count = *(si++);	// height
				// *si&0x0F = transparent colour
				// *si&0xF0 = loop/mirror information
				if (rstate.drv->type == R_CGA1)
				{
					palette = cga1_view_buff;
					palette[*si&0x0F] = 0x44;	// transparent colour
					*si = (*si&0xF0) | 0x04;
				}
				else
				{
					palette = cga0_view_pal;
					*si = (*si&0xF0) | (palette[*si&0x0F]&0x0F);
				}
				si++;
				
				while (line_count != 0)
				{
					while (*si != 0)
					{
						*si = (palette[(*si)>>4]&0xF0) | (*si&0x0F);
						si++;
					}
					si++;	// skip the 0 and go to next line
					line_count--;
				}
					
				cel_count --;
			}
		}
		loop_table += 2;
		loop_count--;
	}
}