/* FUNCTION list 	---	---	---	---	---	---	---

*/
//~ RaDIaT1oN (2002-04-29):
//~ cast for strtok

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "chargen.h"
#include "../base.h"
#include "drv_video.h"
#include "gfx.h"
#include "../ui/string.h"



#include "vid_render.h"

#include "mem_wrap.h"
#include "sys_dir.h"


/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);


// font
/*
void ch_init
void ch_shutdown

void ch_update

void ch_pos_get(TPOS *pos)
void ch_pos_set(TPOS *pos)
void ch_attrib(u8 colour, u16 flags)
void ch_put(u8 ch)
void ch_scroll(TPOS *pos1, TPOS *pos2, u16 scroll, u8 attrib)
void ch_clear(TPOS *pos1, TPOS *pos2, u8 attrib)
*/




/* VARIABLES	---	---	---	---	---	---	--- */
u8 chgen_textmode;
TPOS chgen_textpos = {0,0};

//u8 *font_list = "font_8x8.fnt,font_16x16.fnt";
u8 *font_data = 0;
u8 *font_work = 0;
SIZE font_size = {0,0};
u32 font_chsize = 0;
u32 font_linesize = 0;

POS update_pos= {0,0};
SIZE update_size= {0,0};


/* CODE	---	---	---	---	---	---	---	--- */

// returns scale of screen necessary
// returns 0 if crap
int font_check(SIZE *size)
{
	int render_w, render_h;
	
	render_w = rend_drv->w;
	render_h = rend_drv->h;
	
	// make sure they're multiples of the render w/h
	if ((size->w * 40) % render_w)
		return 0;
	if ((size->h * 21) % render_h)
		return 0;
	
	// make sure the scale is the same vert and hor
	if ((size->w * 40 / render_w) != (size->h * 21 / render_h) )
		return 0;
	
	return size->w * 40 / render_w;
}

FILE *font_open(SIZE *needed)
{
	u8 *token, *running;
	u8 *list;
	SIZE size;
	
	FILE *pref_file = 0;
	int pref_scale = 0;

	FILE *cur_file;
	int cur_scale;

	list = strdupa(c_vid_fonts_bitmap);
	token = strtok_r(list, ";", (char**)&running);
	while (token != 0)
	{
		//check token
		cur_file = fopen(token, "rb");
		if (cur_file != 0)
		{
			// FIXME .. doesn't check size
			size.w = fgetc(cur_file);
			size.h = fgetc(cur_file);

			// if right size ratio
			cur_scale = font_check(&size);
			
			if ( (cur_scale != 0) && (cur_scale > pref_scale)
				&& (cur_scale <= c_vid_scale) )
			{
				if (pref_file != 0)
					fclose(pref_file);
				pref_file = cur_file;
				pref_scale = cur_scale;
				cur_file = 0;
			}
			
			// there's a better font so close this one
			if (cur_file != 0)
				fclose(cur_file);
		}
		token = strtok_r(0, ";", (char**)&running);
	}
	
	// FIX ME
	c_vid_scale = pref_scale;
	
	return pref_file;
}

void font_load(FILE *font_stream)
{
	u8 ch;

	if (font_stream == 0)
	{
		printf("font_init(): Unable to find appropriate font for current resolution.\n");
		agi_exit();
	}

	rewind(font_stream);

	font_size.w = fgetc(font_stream);
	font_size.h = fgetc(font_stream);
	font_chsize = fgetc(font_stream);
	font_linesize = font_chsize / font_size.h;

	font_data = a_malloc(128 * font_chsize);
	memset(font_data, 0, 128 * font_chsize);
	font_work = a_malloc(font_chsize);
	memset(font_work, 0, font_chsize);

	ch = fgetc(font_stream);
	while (ch != 0xFF)
	{
		fread(&font_data[font_chsize*ch], font_chsize, 1, font_stream);
		ch = fgetc(font_stream);
	}
}

void ch_init(void)
{
	SIZE needed;

	needed.w = rend_drv->w * c_vid_scale / 40;
	needed.h = rend_drv->h * c_vid_scale / 21;

	// pick the right font.. load it up
	dir_preset_change(DIR_PRESET_NAGI);
	font_load(font_open(&needed));


	// FIXME..  SCALE THE GOD DAMN FONTTTTT!!

	// text pos = 0,0
	chgen_textpos.row = 0;
	chgen_textpos.col = 0;
	
}

void ch_shutdown(void)
{
	// text pos = 0,0
	chgen_textpos.row = 0;
	chgen_textpos.col = 0;

	a_free(font_data);
	a_free(font_work);
	font_data = 0;
	font_work = 0;

	memset (&font_size, 0, sizeof(SIZE));
	font_chsize = 0;
	font_linesize = 0;
}

// font pos to screen pos

// force update of fonts and unfreeze any freezing

void ch_update(void)
{
	vid_update(gfx_surface, &update_pos, &update_size);
	update_size.w = 0;
	update_size.h = 0;
}

void font_lazy_update(POS *pos, SIZE *size)
{
	POS p1, p2;
	
	if ((update_size.w | update_size.h) != 0)
	{
		//upper y
		if (pos->y < update_pos.y)
			p1.y = pos->y;
		else
			p1.y = update_pos.y;
		
		//lower y
		if ( (pos->y + size->h) > (update_pos.y + update_size.h))
			p2.y = pos->y + size->h;
		else
			p2.y = update_pos.y + update_size.h;
		
		// left x
		if (pos->x < update_pos.x)
			p1.x = pos->x;
		else
			p1.x = update_pos.x;
		
		// right x
		if ((pos->x + size->w) > (update_pos.x + update_size.w))
			p2.x = pos->x + size->w;
		else
			p2.x = update_pos.x + update_size.w;
		
		update_pos.y = p1.y;
		update_pos.x = p1.x;
		update_size.w = p2.x - p1.x;
		update_size.h = p2.y - p1.y;
	}
	else
	{
		update_pos.y = pos->y;
		update_pos.x = pos->x;
		update_size.w = size->w;
		update_size.h = size->h;
	}
}

void ch_pos_get(TPOS *pos)
{
	pos->row = chgen_textpos.row;
	pos->col = chgen_textpos.col;
}

void ch_pos_set(TPOS *pos)
{
	chgen_textpos.row = pos->row;
	chgen_textpos.col = pos->col;
}

u8 given_colour = 0;
u16 given_flags = 0;

void ch_attrib( u8 colour, u16 flags )
{
	given_colour = colour;
	given_flags = flags;
}


#define TEXT_INVERT 0x1
#define TEXT_SHADE 0x2

void ch_put(u8 ch)
{
	u8 *pixels;
	POS gfx_pos;
	u8 *fontp;
	u32 h_count, w_count;
	u8 mask_xor = 0;
	u8 mask_or = 0;
	u8 font_d, b;
	
	gfx_pos.x = font_size.w * chgen_textpos.col;
	gfx_pos.y = font_size.h * chgen_textpos.row;

	pixels = (u8 *)gfx_surface->pixels + gfx_pos.y*gfx_surface->line_size + gfx_pos.x;

	if (   ((given_flags&TEXT_INVERT)!=0) ||
		(((given_colour & 0x80)!=0) && (chgen_textmode==0))    )
			mask_xor = 0xFF;

	if ((given_flags & TEXT_SHADE) != 0)
		mask_or = 0xAA;

	if ((mask_xor | mask_or) != 0)
	{	
		u8 *pp;
		
		fontp = font_work;
		memcpy(fontp, font_data + (font_chsize*ch), font_chsize);
		pp = fontp;

		for (h_count=0; h_count<font_size.h; h_count++)
		{
			for (w_count=0; w_count<font_linesize ; w_count++)
			{
				*pp ^= mask_xor;
				*pp |= mask_or;
				pp++;
			}
			if (mask_or != 0)
				mask_or ^= 0xFF;
		}
	}
	else
	{
		fontp = font_data + (font_chsize*ch);
	}

	vid_lock(gfx_surface);

	for (h_count=0;h_count<font_size.h; h_count++)
	{
		w_count = font_size.w;
		font_d = *(fontp++);
		b = 0x80;	// 1000 0000b

		while (w_count != 0)  // for characters more than 8 bits wide
		{
			if (b==0)
			{
				font_d = *(fontp++);
				b = 0x80;	// 1000 0000b
			}

			if ((font_d & b) != 0)
				*pixels = given_colour & 0x0F;
			else
				*pixels = (given_colour & 0x70)>>4;
			pixels++;
			b >>= 1;
			w_count--;
		}
		pixels += gfx_surface->line_size - font_size.w;
	}

	vid_unlock(gfx_surface);
	font_lazy_update(&gfx_pos, &font_size);
}


// scrolled is signed
// positive is up
// negative is down
 

void ch_scroll(TPOS *pos1, TPOS *pos2, s16 scroll, u8 attrib)
{
	/*SDL_Rect srcrect;
	SDL_Rect dstrect;

	vid_lock(gfx_surface);

	srcrect.x = pos1->col * agi_font->width;
	srcrect.y = (pos1->row + scroll) * agi_font->height;
	srcrect.w = (pos2->col - pos1->col + 1)*agi_font->width;
	srcrect.h = (pos2->row - pos1->row + 1 - scroll)* agi_font->height;

	dstrect.x = pos1->col * agi_font->width;
	dstrect.y = pos1->row * agi_font->height;

	SDL_BlitSurface(sdl_screen, &srcrect, sdl_screen, &dstrect);

	vid_unlock(gfx_surface);
	vid_update(gfx_surface, &pos, &size);
*/


	
	// ----------------------
	
	int copy_width;
	int copy_next;
	int copy_count;
	int copy_old;
	u8 *copy_new;
	TPOS tpos_other;
	POS gfx_pos;
	SIZE g_size; 
	
	if (scroll == 0)	// whoo.. all done :)
		return;
	copy_count = (pos2->row - pos1->row - scroll + 1) * font_size.h;
	
	if (copy_count <= 0)
	{
		// CLEAR THE ENTIRE WINDOW
		ch_clear(pos1, pos2, attrib);
	}
	else
	{
		copy_width = (pos2->col - pos1->col + 1) * font_size.w;
		
		gfx_pos.x = pos1->col * font_size.w;
		
		// do SCROLLING MAGIC
		if (scroll > 0)	// scrolling up
		{
			copy_next = gfx_surface->line_size;	// scan down the screen
			gfx_pos.y = pos1->row * font_size.h;
			copy_new = (u8 *)gfx_surface->pixels + gfx_pos.x +
					gfx_pos.y*gfx_surface->line_size;
			copy_old = scroll * font_size.h * gfx_surface->line_size;
		}
		else			// scrolling down
		{
			copy_next = -(gfx_surface->line_size);	// scan up the screen
			gfx_pos.y = (pos1->row+scroll) * font_size.h;
			copy_new = (u8 *)gfx_surface->pixels + gfx_pos.x +
						(gfx_pos.y+font_size.h - 1)*gfx_surface->line_size;
			copy_old = -(scroll * font_size.h * gfx_surface->line_size);
		}
		
		g_size.w = copy_width;
		g_size.h = copy_count;
		
		vid_lock(gfx_surface);
		while (copy_count)
		{
			memcpy(copy_new, copy_new+copy_old, copy_width);
			copy_new += copy_next;
			copy_count--;
		}
		vid_unlock(gfx_surface);

		font_lazy_update(&gfx_pos, &g_size);
		
		if (scroll > 0)	// scrolling up
		{
			// clear bottom bit of screen
			tpos_other.col = pos1->col;
			tpos_other.row = pos2->row - scroll + 1;
			ch_clear(&tpos_other, pos2, attrib);
		}
		else			// scrolling down
		{
			// clear top bit of area
			tpos_other.col = pos2->col;
			tpos_other.row = pos1->row + scroll - 1;
			ch_clear(pos1, &tpos_other, attrib);
		}
	}
	
	
 
}



void ch_clear(TPOS *pos1, TPOS *pos2, u8 attrib)
{
	POS fill_pos;
	SIZE fill_size;

	// position
	fill_pos.x = pos1->col * font_size.w;
	fill_pos.y = pos1->row * font_size.h;
	fill_size.w = (pos2->col - pos1->col + 1) * font_size.w;
	fill_size.h =  (pos2->row -pos1->row+1) * font_size.h;

	// colour
	attrib = (attrib & 0xF0)>>4;
	if (chgen_textmode != 0)
		attrib = attrib & 0x7;	// textmode does not support bright backgrounds
	switch(gfx_paltype)
	{
		case PAL_CGA0:
		case PAL_CGA1:
			if (chgen_textmode == 0)
				attrib = attrib & 0x3;	// textmode does not support bright backgrounds
			break;
		default: ;
	}

	vid_lock(gfx_surface);
	vid_fill(gfx_surface, &fill_pos, &fill_size, attrib&0x0F);
	vid_unlock(gfx_surface);
	font_lazy_update(&fill_pos, &fill_size);
}



