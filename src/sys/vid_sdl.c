/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>


/* OTHER headers	---	---	---	---	---	---	--- */
#include "video.h"
#include "vid_render.h"

#include "vid_sdl.h"

#include "font_16x16.h"
#include "font_8x8.h"
#include "font_4x8.h"

#include "../base.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);

SDL_Color ega_palette[] =
{ 	{0,0,0,0},		// black
	{0,0,170,0},	// dark blue
	{0,170,0,0},	// dark green
	{0,170,170,0},	// dark cyan
	{170,0,0,0},	// dark red
	{170,0,170,0}, 	// dark purple
	{170,85,0,0},	// dark brown
	{170,170,170,0},	// light grey
	{85,85,85,0}, 	// dark grey
	{85,85,255,0}, 	// light blue
	{85,255,85,0}, 	// light green
	{85,255,255,0},	// light cyan
	{255,85,85,0},  	// light red
	{255,85,255,0}, 	// light purple
	{255,255,85,0}, 	// light yellow
	{255,255,255,0}	// white
};

SDL_Color cga_0_palette[] =
{ 	{0,0,0,0},		// black
	{85,255,255,0},	// cyan
	{255,85,255,0},	// magenta
	{255,255,255,0}// white
};

SDL_Color cga_1_palette[] =
{ 	{0,0,170,0},	// blue
	{0,170,0,0},	// green
	{170,0,0,0},	// red
	{170,85,0,0}	// yellow
};

/* VARIABLES	---	---	---	---	---	---	--- */
SDL_Surface *sdl_screen = 0;


// list is sorted in preferential order (biggest to smallest preferably)
//FONT font_list[] = {font_32x32, font_16x16, font_8x8, font_4x8, 0};
FONT *font_list[] = {&font_16x16, &font_8x8, &font_4x8, 0};
FONT *agi_font = 0;
POS sdl_text_pos = {0,0};

/* CODE	---	---	---	---	---	---	---	--- */

void sdlvid_init(RSTATE *rend_state)
{
	u32 video_flags;

	
	sdlvid_shutdown();

	video_flags = SDL_SWSURFACE|SDL_HWPALETTE;
	if (vstate.fullscreen != 0)
		video_flags |= SDL_FULLSCREEN;
	
	vstate.w = ((rstate.drv->w + 159)/160) * 160 * vstate.scale;
	vstate.h = ((rstate.drv->h + 199)/200) * 200 * vstate.scale;
	
	sdl_screen = SDL_SetVideoMode(vstate.w, vstate.h, 8, video_flags);
	
	if (sdl_screen == 0)
	{
		printf("Unable to create video surface: %s\n", SDL_GetError());
		exit(1);
	}
	 
	switch(rstate.drv->type)
	{
		case R_CGA0:
			SDL_SetPalette(sdl_screen, SDL_LOGPAL|SDL_PHYSPAL, cga_0_palette, 0, 4);
			break;
		case R_CGA1:
			SDL_SetPalette(sdl_screen, SDL_LOGPAL|SDL_PHYSPAL, cga_1_palette, 0, 4);
			break;
		default:
			SDL_SetPalette(sdl_screen, SDL_LOGPAL|SDL_PHYSPAL, ega_palette, 0, 16);
			break;
	}  
	
	sdlvid_clear();
	
	font_init();

}

void sdlvid_shutdown()
{
	if (sdl_screen != 0)
	{
		SDL_FreeSurface(sdl_screen);
		sdl_screen = 0;
	}
	font_shutdown();
}

void sdlvid_clear()
{
	if ( (SDL_MUSTLOCK(sdl_screen)) &&
		(SDL_LockSurface(sdl_screen) != 0) )
			return;
	SDL_FillRect(sdl_screen, 0, 0);
	if (SDL_MUSTLOCK(sdl_screen))
		SDL_UnlockSurface(sdl_screen);
	SDL_UpdateRect(sdl_screen, 0,0,0,0);
}

void sdlvid_update(u16 rect_x, u16 rect_y, u16 rect_w, u16 rect_h)
{
	u8 *sdl_buf;
	u8 *sdl_line;
	u8 *rend_buf;
	
	u16 sdl_x, sdl_y, sdl_w, sdl_h;
	u16 rend_x, rend_y, rend_w, rend_h;
	u16 h_count, w_count, i;
	
	rend_x = rect_x * rstate.drv->scale_x;
	rend_y = rstate.drv->scale_y*(rect_y + 1) - 1;
	rend_w = rect_w * rstate.drv->scale_x;
	rend_h = rect_h * rstate.drv->scale_y;
	
	sdl_x = rend_x * vstate.scale;
	sdl_y = vstate.scale*(rend_y + 1) - 1     + state.window_row_min * agi_font->height;
	sdl_w = rend_w * vstate.scale;
	sdl_h = rend_h * vstate.scale;
	
	if ( (SDL_MUSTLOCK(sdl_screen)) &&
		(SDL_LockSurface(sdl_screen) != 0) )
			return;
	
	rend_buf = rstate.buf + rend_y*rstate.drv->w + rend_x;
	sdl_buf = (u8 *)sdl_screen->pixels + sdl_y*sdl_screen->pitch + sdl_x;
	
	for (h_count=rend_h; h_count!=0; h_count--)
	{
		// draw line
		for (w_count=rend_w; w_count!=0; w_count--)
		{
			memset(sdl_buf, (*rend_buf)&0xF, vstate.scale);
			sdl_buf += vstate.scale;
			rend_buf++;
		}
		
		// repeat line
		if (vstate.scale != 1)
		{
			sdl_line = sdl_buf - rend_w*vstate.scale;
			sdl_buf -= sdl_screen->pitch + rend_w*vstate.scale;
			for (i=0; i<vstate.scale-1; i++)
			{
				memcpy(sdl_buf, sdl_line, rend_w*vstate.scale);
				sdl_buf -= sdl_screen->pitch;
			}
		}
		else
			sdl_buf -= sdl_screen->pitch*vstate.scale + rend_w*vstate.scale;

		// next line in buffer
		rend_buf -= rstate.drv->w + rend_w;
	}
	
	if (SDL_MUSTLOCK(sdl_screen))
		SDL_UnlockSurface(sdl_screen);
 
	#warning need to check for SDLs sanity..  could be a bug?
	//if (final width+x > side of screen)
	//	fix final width
	
	SDL_UpdateRect(sdl_screen, sdl_x,sdl_y-sdl_h+1,sdl_w,sdl_h);
}

void sdlvid_shake(u8 count)
{
	printf("dummy video shake\n");
}



// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------




void font_init(void)
{
	FONT **blah;
	
	// init font code
	blah = &font_list[0];
	agi_font = *blah;
	while (agi_font != 0)
	{
		if ((sdl_screen->w % (agi_font->width * 40)) == 0)
			break;
		blah ++;
		agi_font = *blah;
	}

	if (agi_font  == 0)
	{
		printf("font_init(): Unable to find appropriate font for current resolution.\n");
		agi_exit();
	}

	memset(&sdl_text_pos, 0, sizeof(POS));
}

void font_shutdown(void)
{
}

// font pos to screen pos

// force update of fonts and unfreeze any freezing
void font_update(void)
{
}




void sdlvid_pos_get(POS *pos)
{
	memcpy(pos, &sdl_text_pos, sizeof(POS));
}

void sdlvid_pos_set(POS *pos)
{
	memcpy(&sdl_text_pos, pos, sizeof(POS));
}

#warning conversion from ega collurs to cga??
u8 given_colour = 0;
u16 given_flags = 0;

void sdlvid_char_attrib( u8 colour, u16 flags )
{
	given_colour = colour;
	given_flags = flags;
}


#define TEXT_INVERT 0x1
#define TEXT_SHADE 0x2

void sdlvid_char_put(u8 ch)
{
	// get position
	// get attributes
	// get flags
	// post it
	
	u8 *sdl_buf;
	u16 sdl_x, sdl_y;
	u8 font_d, b;
	u8 *font_dp, *pp;
	int h_count, w_count;
	
	sdl_x = agi_font->width * sdl_text_pos.col;
	sdl_y = agi_font->height * sdl_text_pos.row;
	
	sdl_buf = (u8 *)sdl_screen->pixels + sdl_y*sdl_screen->pitch + sdl_x;
	
	font_dp = alloca(agi_font->align);
	memcpy(font_dp, agi_font->data + agi_font->align*ch, agi_font->align);
	
	if ((given_flags & (TEXT_INVERT|TEXT_SHADE) ) != 0)
	{
		u8 mask_xor = 0;
		u8 mask_or = 0;
		
		if (   ((given_flags&TEXT_INVERT)!=0) ||
			(((given_colour & 0x80)!=0) && (vstate.text_mode==0))    )
				mask_xor = 0xFF;
		
		if ((given_flags & TEXT_SHADE) != 0)
			mask_or = 0xAA;
		
		pp = font_dp;
		
		for (h_count=0;h_count<agi_font->height; h_count++)
		{
			for (w_count=0; w_count<agi_font->line_size ; w_count++)
			{
				*pp ^= mask_xor;
				*pp |= mask_or;
				pp++;
			}
			if (mask_or != 0)
				mask_or ^= 0xFF;
		}
	}
	
	
	if ( (SDL_MUSTLOCK(sdl_screen)) &&
		(SDL_LockSurface(sdl_screen) != 0) )
			return;
	
	
	
	for (h_count=0;h_count<agi_font->height; h_count++)
	{
		w_count = agi_font->width;
		font_d = *(font_dp++);
		b = 0x80;	// 1000 0000b 
		
		while (w_count != 0)  // for characters more than 8 bits wide
		{
			if (b==0)
			{
				font_d = *(font_dp++);
				b = 0x80;	// 1000 0000b 
			}
			
			if ((font_d & b) != 0)
				*sdl_buf = given_colour & 0x0F;
			else
				*sdl_buf = (given_colour & 0x70)>>4;
			sdl_buf++;
			b = b >> 1;
			w_count--;
		}
		sdl_buf += sdl_screen->pitch - agi_font->width;
	}
	
	if (SDL_MUSTLOCK(sdl_screen))
		SDL_UnlockSurface(sdl_screen);
	SDL_UpdateRect(sdl_screen, sdl_x, sdl_y, agi_font->width, agi_font->height);

}

void sdlvid_scroll(POS *pos1, POS *pos2, u16 scroll, u8 attrib)
{
	SDL_Rect srcrect;
	SDL_Rect dstrect;
	
	if ( (SDL_MUSTLOCK(sdl_screen)) &&
		(SDL_LockSurface(sdl_screen) != 0) )
			return;
	
	srcrect.x = pos1->col * agi_font->width;
	srcrect.y = (pos1->row + scroll) * agi_font->height;
	srcrect.w = (pos2->col - pos1->col + 1)*agi_font->width;
	srcrect.h = (pos2->row - pos1->row + 1 - scroll)* agi_font->height;

	dstrect.x = pos1->col * agi_font->width;
	dstrect.y = pos1->row * agi_font->height;
		
	SDL_BlitSurface(sdl_screen, &srcrect, sdl_screen, &dstrect);
	
	if (SDL_MUSTLOCK(sdl_screen))
		SDL_UnlockSurface(sdl_screen);
	SDL_UpdateRect(sdl_screen, srcrect.x,0,0,0);

}



void sdlvid_char_clear(POS *pos1, POS *pos2, u8 attrib)
{
	SDL_Rect fill_area;
	
	fill_area.x = pos1->col * agi_font->width;
	fill_area.y = pos1->row * agi_font->height;
	fill_area.w = (pos2->col - pos1->col + 1) * agi_font->width;
	fill_area.h =  (pos2->row -pos1->row+1)*agi_font->height;

	attrib = (attrib & 0xF0)>>4;
	if (vstate.text_mode != 0)
		attrib = attrib & 0x7;	// textmode does not support bright backgrounds
	switch(rstate.drv->type)
	{
		case R_CGA0:
		case R_CGA1:
			if (vstate.text_mode == 0)
				attrib = attrib & 0x3;	// textmode does not support bright backgrounds
			break;
		default:
	}
	
	if ( (SDL_MUSTLOCK(sdl_screen)) &&
		(SDL_LockSurface(sdl_screen) != 0) )
			return;
	
	SDL_FillRect(sdl_screen, &fill_area, attrib&0x0F);
	
	if (SDL_MUSTLOCK(sdl_screen))
		SDL_UnlockSurface(sdl_screen);
	SDL_UpdateRect(sdl_screen, fill_area.x, fill_area.y, fill_area.w, fill_area.h);
}


