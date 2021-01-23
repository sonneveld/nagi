/* FUNCTION list 	---	---	---	---	---	---	---

*/

/*
I haven't found an ideal solution for converting 8 bit indexed surfaces to 32bit textures. However one thing that I
decided was that I don't want to call SDL_ConvertSurfaceFormat or SDL_CreateTextureFromSurface per frame.

I ended up creating two surfaces, the 8 bit one that the AGI engine draws to and a second 32bit one that has the same
pixel format as the main screen texture. On render, we SDL_BlitSurface from the 8bit to 32bit surface to do pixel 
conversion, then call SDL_UpdateTexture on the (streaming) screen texture with the 32bit surface as source.

References:
Rendering 8-bit palettized surfaces in SDL 2.0 applications: http://sandervanderburg.blogspot.com/2014/05/rendering-8-bit-palettized-surfaces-in.html
Mini code sample for SDL2 256-color palette https://discourse.libsdl.org/t/mini-code-sample-for-sdl2-256-color-palette/27147/10
*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
/* OTHER headers	---	---	---	---	---	---	--- */

#include "../base.h"

#include "mem_wrap.h"

#include "sdl_vid.h"



/* PROTOTYPES	---	---	---	---	---	---	--- */


/* VARIABLES	---	---	---	---	---	---	--- */

struct video_struct
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Surface *surface;
	SDL_Surface *surface_conv;
};

typedef struct video_struct VIDEO;

VIDEO video_data = { 0 };

/* CODE	---	---	---	---	---	---	---	--- */



// elemental
//-----------------------------------------------------------


void vid_init()
{
	printf("Initialising SDL video subsystem... ");
	
#if 0
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		printf("vid_driver_init(): unable to initialise SDL video subsystem.\n");
		agi_exit();
	}
#endif
	
	printf("done.\n");
}

void vid_shutdown()
{
	//SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


// create an 8bit window with such a size
// clear it with colour 0
// 1= fullscreen
// 0 = window
void vid_display(AGISIZE *screen_size, int fullscreen_state)
{
	int result = 0;
	u32 sdl_flags;
	
	sdl_flags = SDL_WINDOW_RESIZABLE;
	if (fullscreen_state)
		sdl_flags |= SDL_WINDOW_FULLSCREEN_DESKTOP;

	result = SDL_CreateWindowAndRenderer( screen_size->w, screen_size->h,
		sdl_flags, &video_data.window, &video_data.renderer);

	if(result != 0)
	{
		printf("Unable to create video window: %s\n", SDL_GetError());
		agi_exit();
	}

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(video_data.renderer, screen_size->w, screen_size->h);

	video_data.surface = SDL_CreateRGBSurface( 0,
		screen_size->w, screen_size->h, 8,
		0, 0, 0, 0 );
	if (video_data.surface == NULL)
	{
		printf("Unable to create video surface: %s\n", SDL_GetError());
		agi_exit();
	}
	SDL_FillRect(video_data.surface, NULL, 0);

	video_data.texture = SDL_CreateTexture( video_data.renderer,
		SDL_PIXELFORMAT_RGB888,
		SDL_TEXTUREACCESS_STREAMING,
		screen_size->w, screen_size->h );
	if (video_data.texture == NULL)
	{
		printf("Unable to create video texture: %s\n", SDL_GetError());
		agi_exit();
	}	

	// Create intermediate surface to convert 8bit to 32bit pixels.
	// TODO: Nick: I think it should be okay to just use SDL_ConvertSurfaceFormat but I thought I saw some issues.
#if 1
	video_data.surface_conv = SDL_ConvertSurfaceFormat(video_data.surface, SDL_PIXELFORMAT_RGB888, 0);
	SDL_FillRect(video_data.surface_conv, NULL, SDL_MapRGBA(video_data.surface_conv->format, 0, 0, 0, 255));
#else
	int conv_bpp;
	Uint32 conv_Rmask;
	Uint32 conv_Gmask;
	Uint32 conv_Bmask;
	Uint32 conv_Amask;
	if (!SDL_PixelFormatEnumToMasks(SDL_PIXELFORMAT_RGB888, &conv_bpp, &conv_Rmask, &conv_Gmask, &conv_Bmask, &conv_Amask)) {
		printf("Unable to create pixel format masks: %s\n", SDL_GetError());
		agi_exit();
	}
	video_data.surface_conv = SDL_CreateRGBSurface(
		0, 
		screen_size->w, screen_size->h,
		conv_bpp,
		conv_Rmask, conv_Gmask, conv_Bmask, conv_Amask
		);
#endif

	if (video_data.surface_conv == NULL) {
		printf("Unable to create conversion video surface: %s\n", SDL_GetError());
		agi_exit();
	}

	// clear
	SDL_SetRenderDrawColor(video_data.renderer, 0, 0, 0, 255);
	SDL_RenderClear(video_data.renderer);

	SDL_RenderPresent(video_data.renderer);
}

void vid_free()
{
	if (video_data.texture != 0) 
	{
		SDL_DestroyTexture(video_data.texture);
		video_data.texture = 0;
	}

	if (video_data.surface != 0)
	{
		SDL_FreeSurface(video_data.surface);
		video_data.surface = 0;
	}

	if (video_data.surface_conv != 0)
	{
		SDL_FreeSurface(video_data.surface_conv);
		video_data.surface_conv = 0;
	}

	if (video_data.renderer != 0) 
	{
		SDL_DestroyRenderer(video_data.renderer);
		video_data.renderer = 0;
	}

	if (video_data.window != 0)
	{
		SDL_DestroyWindow(video_data.window);
		video_data.window = 0;
	}
}

void *vid_getbuf()
{
	assert(video_data.surface);
	return video_data.surface->pixels;
}

int vid_getlinesize()
{
	assert(video_data.surface);
	return video_data.surface->pitch;
}

SDL_Window* vid_get_main_window()
{
	return video_data.window;
}

void vid_lock()
{
	assert(video_data.surface);
	if (!SDL_MUSTLOCK(video_data.surface)) { return; }
	if (!SDL_LockSurface(video_data.surface))
	{
		printf("vid_lock(); Unable to lock video surface: %s\n", SDL_GetError());
		agi_exit();
	}
}

void vid_unlock()
{
	assert(video_data.surface);
	if (!SDL_MUSTLOCK(video_data.surface)) { return; }
	SDL_UnlockSurface(video_data.surface);
}

// update a surface
void vid_update(POS *pos, AGISIZE *size)
{
	SDL_Surface *surface;

	surface = video_data.surface;
	assert(surface);

	// check boundaries
	if ((pos->x + size->w) > surface->w)
		size->w = surface->w - pos->x;
	if ((pos->y + size->h) > surface->h)
		size->h = surface->h - pos->y;
	
	vid_render(surface, pos->x, pos->y, size->w, size->h);

}

// when resizing a window, make sure the aspect ratio is preserved
void vid_resize(s32 x, s32 y)
{
	Uint32 format;
	int access, texture_width, texture_height, window_width, window_height;
	double ratio;

	window_width = (int) x;
	window_height = (int) y;
	if(0 == SDL_QueryTexture(video_data.texture, &format, &access,
		&texture_width, &texture_height))
	{
		SDL_Window *window = vid_get_main_window();
		ratio = (double)texture_height / (double)texture_width;
		window_height = window_width * ratio;
		SDL_SetWindowSize(window, window_width, window_height);
		vid_render(video_data.surface, 0, 0, texture_width, texture_height);
	}
}

void vid_render(SDL_Surface *surface, const u32 x, const u32 y, const u32 w, const u32 h)
{
	SDL_Rect rect;
	rect.x = x;
	rect.y = y;
	rect.w = w;
	rect.h = h;

	int res;

	// Convert up from 8bpp (used on ye olde graphics cards) to
	// something relevant to this century
	res = SDL_BlitSurface(surface, NULL, video_data.surface_conv, NULL);
	if (res != 0) {
		printf("vid_render: Error converting surface: %s\n", SDL_GetError());
	}
	res = SDL_UpdateTexture(video_data.texture, NULL, video_data.surface_conv->pixels, video_data.surface_conv->pitch);
	if (res != 0) {
		printf("vid_render: Error updating screen texture: %s\n", SDL_GetError());
	}

	res = SDL_SetRenderDrawColor(video_data.renderer, 0, 0, 0, 255);
	res = SDL_RenderClear(video_data.renderer);
	if (res != 0) {
		printf("vid_render: Error clearing screen: %s\n", SDL_GetError());
	}

	res = SDL_RenderCopy(video_data.renderer, video_data.texture, NULL, NULL);
	if (res != 0) {
		printf("vid_render: Error copying texture to screen: %s\n", SDL_GetError());
	}
	SDL_RenderPresent(video_data.renderer);
}

// set 8-bit palette
void vid_palette_set(PCOLOUR *palette, u8 num)
{
	SDL_Color *sdl_palette = alloca(num * sizeof(SDL_Color));
	int i;
	SDL_Surface *surface;

	surface = video_data.surface;
	assert(surface);

	for (i=0; i<num; i++)
	{
		sdl_palette[i].r = palette[i].r;
		sdl_palette[i].g = palette[i].g;
		sdl_palette[i].b = palette[i].b;
		sdl_palette[i].a = 0;
	}

	if(SDL_SetPaletteColors(surface->format->palette, sdl_palette, 0, num) != 0)
	{
		printf( "Unable to set colour palette: %s\n", SDL_GetError());
		agi_exit();
	}
}

//derived:
//-----------------------------------------------------------

// fill the surface at a particular point.. rectangle
// can use library's fast blit's 'n stuff
void vid_fill(POS *pos, AGISIZE *size, u32 colour)
{
	assert(video_data.surface);
	
	if ( (pos->x|pos->y|size->w|size->h) == 0)
	{
		vid_lock();
		SDL_FillRect(video_data.surface, 0, colour);
		vid_unlock();
	}
	else
	{
		SDL_Rect rect;
		rect.x = pos->x;
		rect.y = pos->y;
		rect.w = size->w;
		rect.h = size->h;
		vid_lock();
		SDL_FillRect(video_data.surface, &rect, colour);
		vid_unlock();
		vid_render(video_data.surface,
			rect.x, rect.y, rect.w, rect.h);
	}
}

int shake_offset[] = {25, 0, -25};

void vid_shake(int count)
{
	u32 rmask,gmask,bmask,amask;
	u8 bpp;
	int width, height;
	SDL_Surface *orig;
	SDL_Rect dest = {0,0, 0, 0};
	SDL_Surface *surface;
	
	surface = video_data.surface;
	assert(surface);
	

	//      get screen width, height, depth, ptic... etc etc
	bpp = surface->format->BitsPerPixel;
	rmask = surface->format->Rmask;
	gmask = surface->format->Gmask;
	bmask = surface->format->Bmask;
	amask = surface->format->Amask;
	width =  surface->w;
	height =  surface->h;

	//      create new surface
	orig = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
	if (!orig) return;

	orig->format->palette->ncolors = surface->format->palette->ncolors ;
	memcpy (orig->format->palette->colors, surface->format->palette->colors, orig->format->palette->ncolors * sizeof (SDL_Color)) ;
	
	//      blit screen to new surface
	if (SDL_BlitSurface(surface, 0, orig, 0)) goto shake_error;

	count *= 8;
	while (count--)
	{
		// clear entire window
		vid_lock();
		if (SDL_FillRect(surface, 0, 0))
			goto shake_error;       
		vid_unlock();

		// print the surface in some strange location
		dest.x = shake_offset[rand()%3];
		dest.y = shake_offset[rand()%3];
		if (SDL_BlitSurface(orig, 0, surface, &dest)) // blit to some offset  stretch*10 or something
			goto shake_error; 
		vid_render(surface, 0, 0, 0, 0);

		SDL_Delay(50);
	}
	// put the original screen back on
	if (SDL_BlitSurface(orig, 0, surface, 0)) // update the screen
		goto shake_error; 
	vid_render(surface, 0, 0, 0, 0);
	
shake_error:
	SDL_FreeSurface(orig);
}
