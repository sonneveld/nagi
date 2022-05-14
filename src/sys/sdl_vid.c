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

#include "IBM_VGA_8x8.h"
#include "SDL.h"


/* PROTOTYPES	---	---	---	---	---	---	--- */

void vid_debug_render();
void vid_debug_strings_render();
void draw_strings();

static void vid_free_surfaces(void);
static void vid_render(SDL_Surface* surface, const u32 x, const u32 y, const u32 w, const u32 h);

/* VARIABLES	---	---	---	---	---	---	--- */

struct video_struct
{
	SDL_Window *window;
	SDL_Renderer *renderer;
	SDL_Texture *texture;
	SDL_Surface *surface;
	SDL_Surface *surface_conv;

	SDL_Window *debug_window;
	SDL_Renderer *debug_renderer;
	SDL_Palette *debug_palette;
	SDL_Surface *debug_surface;
	SDL_Texture *debug_texture;

	SDL_Window *debug_strings_window;
	SDL_Renderer *debug_strings_renderer;
	SDL_Palette *debug_strings_palette;
	SDL_Surface *debug_strings_surface;
	SDL_Texture *debug_strings_texture;

};

typedef struct video_struct VIDEO;

static VIDEO video_data = { 0 };




#define PALETTE_ALPHA (0xFF)

static SDL_Colour ega_colour_map[] = {
    {0x00, 0x00, 0x00, PALETTE_ALPHA},
    {0x00, 0x00, 0xAA, PALETTE_ALPHA},
    {0x00, 0xAA, 0x00, PALETTE_ALPHA},
    {0x00, 0xAA, 0xAA, PALETTE_ALPHA},
    {0xAA, 0x00, 0x00, PALETTE_ALPHA},
    {0xAA, 0x00, 0xAA, PALETTE_ALPHA},
    {0xAA, 0xAA, 0x00, PALETTE_ALPHA},
    {0xAA, 0xAA, 0xAA, PALETTE_ALPHA},
    {0x00, 0x00, 0x55, PALETTE_ALPHA},
    {0x00, 0x00, 0xFF, PALETTE_ALPHA},
    {0x00, 0xAA, 0x55, PALETTE_ALPHA},
    {0x00, 0xAA, 0xFF, PALETTE_ALPHA},
    {0xAA, 0x00, 0x55, PALETTE_ALPHA},
    {0xAA, 0x00, 0xFF, PALETTE_ALPHA},
    {0xAA, 0xAA, 0x55, PALETTE_ALPHA},
    {0xAA, 0xAA, 0xFF, PALETTE_ALPHA},
    {0x00, 0x55, 0x00, PALETTE_ALPHA},
    {0x00, 0x55, 0xAA, PALETTE_ALPHA},
    {0x00, 0xFF, 0x00, PALETTE_ALPHA},
    {0x00, 0xFF, 0xAA, PALETTE_ALPHA},
    {0xAA, 0x55, 0x00, PALETTE_ALPHA},
    {0xAA, 0x55, 0xAA, PALETTE_ALPHA},
    {0xAA, 0xFF, 0x00, PALETTE_ALPHA},
    {0xAA, 0xFF, 0xAA, PALETTE_ALPHA},
    {0x00, 0x55, 0x55, PALETTE_ALPHA},
    {0x00, 0x55, 0xFF, PALETTE_ALPHA},
    {0x00, 0xFF, 0x55, PALETTE_ALPHA},
    {0x00, 0xFF, 0xFF, PALETTE_ALPHA},
    {0xAA, 0x55, 0x55, PALETTE_ALPHA},
    {0xAA, 0x55, 0xFF, PALETTE_ALPHA},
    {0xAA, 0xFF, 0x55, PALETTE_ALPHA},
    {0xAA, 0xFF, 0xFF, PALETTE_ALPHA},
    {0x55, 0x00, 0x00, PALETTE_ALPHA},
    {0x55, 0x00, 0xAA, PALETTE_ALPHA},
    {0x55, 0xAA, 0x00, PALETTE_ALPHA},
    {0x55, 0xAA, 0xAA, PALETTE_ALPHA},
    {0xFF, 0x00, 0x00, PALETTE_ALPHA},
    {0xFF, 0x00, 0xAA, PALETTE_ALPHA},
    {0xFF, 0xAA, 0x00, PALETTE_ALPHA},
    {0xFF, 0xAA, 0xAA, PALETTE_ALPHA},
    {0x55, 0x00, 0x55, PALETTE_ALPHA},
    {0x55, 0x00, 0xFF, PALETTE_ALPHA},
    {0x55, 0xAA, 0x55, PALETTE_ALPHA},
    {0x55, 0xAA, 0xFF, PALETTE_ALPHA},
    {0xFF, 0x00, 0x55, PALETTE_ALPHA},
    {0xFF, 0x00, 0xFF, PALETTE_ALPHA},
    {0xFF, 0xAA, 0x55, PALETTE_ALPHA},
    {0xFF, 0xAA, 0xFF, PALETTE_ALPHA},
    {0x55, 0x55, 0x00, PALETTE_ALPHA},
    {0x55, 0x55, 0xAA, PALETTE_ALPHA},
    {0x55, 0xFF, 0x00, PALETTE_ALPHA},
    {0x55, 0xFF, 0xAA, PALETTE_ALPHA},
    {0xFF, 0x55, 0x00, PALETTE_ALPHA},
    {0xFF, 0x55, 0xAA, PALETTE_ALPHA},
    {0xFF, 0xFF, 0x00, PALETTE_ALPHA},
    {0xFF, 0xFF, 0xAA, PALETTE_ALPHA},
    {0x55, 0x55, 0x55, PALETTE_ALPHA},
    {0x55, 0x55, 0xFF, PALETTE_ALPHA},
    {0x55, 0xFF, 0x55, PALETTE_ALPHA},
    {0x55, 0xFF, 0xFF, PALETTE_ALPHA},
    {0xFF, 0x55, 0x55, PALETTE_ALPHA},
    {0xFF, 0x55, 0xFF, PALETTE_ALPHA},
    {0xFF, 0xFF, 0x55, PALETTE_ALPHA},
    {0xFF, 0xFF, 0xFF, PALETTE_ALPHA},
};


static int palette_ega_default[] = {
    0,
    1,
    2,
    3,
    4,
    5,
    20,
    7,
    56,
    57,
    58,
    59,
    60,
    61,
    62,
    63,
};

/* CODE	---	---	---	---	---	---	---	--- */



// elemental
//-----------------------------------------------------------


void vid_init()
{
#if 0
	printf("Initialising SDL video subsystem... ");
	
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		printf("vid_driver_init(): unable to initialise SDL video subsystem.\n");
		agi_exit();
	}
	
	printf("done.\n");
#endif
}

void vid_shutdown()
{
	//SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


void vid_debug_display()
{
	int res = 0;

	if (video_data.debug_window != 0) { return; }

	res = SDL_CreateWindowAndRenderer( 320, 168, SDL_WINDOW_RESIZABLE, &video_data.debug_window, &video_data.debug_renderer);


	SDL_SetWindowTitle(video_data.debug_window, "NAGI Debug");

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	SDL_RenderSetLogicalSize(video_data.debug_renderer, 320, 168);




 	if (video_data.debug_palette == 0) {
		// You must allocate 256 colours even though we're only used 16
		// This is because SDL doesn't check the palette range and will
		// try to go out of bounds if our surface has colours outside 16
		video_data.debug_palette = SDL_AllocPalette(256);
		assert(video_data.debug_palette != 0);
	}

    // set some ugly colour for if we accidentally go out of the 16 colour range.
    for (int i = 0; i < 256; i++) {
        SDL_Color c = {0xFF,0x00,0xFF,PALETTE_ALPHA};
        res = SDL_SetPaletteColors(video_data.debug_palette, &c, i, 1);
        assert(res == 0);
    }



	for (int i = 0; i < 16; i++) {

    res = SDL_SetPaletteColors(video_data.debug_palette, &ega_colour_map[palette_ega_default[i]], i, 1);
    assert(res == 0);
	}


    video_data.debug_surface = SDL_CreateRGBSurface(0, 160, 168, 8, 0, 0, 0, 0);
    assert(video_data.debug_surface != 0);

    res = SDL_SetSurfacePalette(video_data.debug_surface, video_data.debug_palette);
    assert(res == 0);

    if (video_data.debug_texture == 0) {
        video_data.debug_texture = SDL_CreateTexture(video_data.debug_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 160, 168);
        assert(video_data.debug_texture != 0);
        res = SDL_SetTextureBlendMode(video_data.debug_texture, SDL_BLENDMODE_NONE);
        assert(res == 0);
    }


    	SDL_RenderClear(video_data.debug_renderer);

	SDL_RenderPresent(video_data.debug_renderer);

}



void vid_debug_strings_display()
{
	int res = 0;

	if (video_data.debug_strings_window != 0) { return; }

	res = SDL_CreateWindowAndRenderer( 640, 480, SDL_WINDOW_RESIZABLE, &video_data.debug_strings_window, &video_data.debug_strings_renderer);


	SDL_SetWindowTitle(video_data.debug_strings_window, "NAGI Strings");

	SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");  // make the scaled rendering look smoother.
	// SDL_RenderSetLogicalSize(video_data.debug_strings_renderer, 320, 168);




 	if (video_data.debug_strings_palette == 0) {
		// You must allocate 256 colours even though we're only used 16
		// This is because SDL doesn't check the palette range and will
		// try to go out of bounds if our surface has colours outside 16
		video_data.debug_strings_palette = SDL_AllocPalette(256);
		assert(video_data.debug_strings_palette != 0);
	}

    // set some ugly colour for if we accidentally go out of the 16 colour range.
    for (int i = 0; i < 256; i++) {
        SDL_Color c = {0xFF,0x00,0xFF,PALETTE_ALPHA};
        res = SDL_SetPaletteColors(video_data.debug_strings_palette, &c, i, 1);
        assert(res == 0);
    }



	for (int i = 0; i < 16; i++) {

		res = SDL_SetPaletteColors(video_data.debug_strings_palette, &ega_colour_map[palette_ega_default[i]], i, 1);
		assert(res == 0);
	}


    video_data.debug_strings_surface = SDL_CreateRGBSurface(0, 640, 480, 8, 0, 0, 0, 0);
    assert(video_data.debug_strings_surface != 0);

    res = SDL_SetSurfacePalette(video_data.debug_strings_surface, video_data.debug_strings_palette);
    assert(res == 0);

    if (video_data.debug_strings_texture == 0) {
        video_data.debug_strings_texture = SDL_CreateTexture(video_data.debug_strings_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, 640, 480);
        assert(video_data.debug_strings_texture != 0);
        res = SDL_SetTextureBlendMode(video_data.debug_strings_texture, SDL_BLENDMODE_NONE);
        assert(res == 0);
    }


    	SDL_RenderClear(video_data.debug_strings_renderer);

	SDL_RenderPresent(video_data.debug_strings_renderer);

}

// create an 8bit window with such a size
// clear it with colour 0
// 1= fullscreen
// 0 = window
void vid_display(AGISIZE *screen_size, int fullscreen_state)
{
	int result = 0;

	vid_debug_display();

	vid_debug_strings_display();

	if (video_data.window == 0)
	{
		Uint32 sdl_flags;
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
	} 
	else 
	{
		Uint32 sdl_flags = fullscreen_state ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0;
		result = SDL_SetWindowFullscreen(video_data.window, sdl_flags);
		if (result != 0)
		{
			printf("Error trying to set fullscreen state to %d: %s\n", fullscreen_state, SDL_GetError());
		}
	}

	assert(video_data.window != 0);
	assert(video_data.renderer != 0);


	SDL_SetWindowTitle(video_data.window, "NAGI");

	if (video_data.surface != 0)
	{
		if ((video_data.surface->h != screen_size->h) || (video_data.surface->w != screen_size->w))
		{
			vid_free_surfaces();
			assert(video_data.surface == 0);
		}
	}

	if (video_data.surface == 0)
	{
		assert(video_data.surface == 0);
		video_data.surface = SDL_CreateRGBSurface( 0,
			screen_size->w, screen_size->h, 8,
			0, 0, 0, 0 );
		if (video_data.surface == NULL)
		{
			printf("Unable to create video surface: %s\n", SDL_GetError());
			agi_exit();
		}
		SDL_FillRect(video_data.surface, NULL, 0);

		assert(video_data.texture == 0);
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
		assert(video_data.surface_conv == 0);
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

		vid_notify_window_size_changed(SDL_GetWindowID(video_data.window));
	}

	// clear
	SDL_SetRenderDrawColor(video_data.renderer, 0, 0, 0, 255);
	SDL_RenderClear(video_data.renderer);

	SDL_RenderPresent(video_data.renderer);
}

static void vid_free_surfaces(void)
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
}

void vid_free()
{
	vid_free_surfaces();

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

SDL_Window* vid_get_main_window(void)
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

void vid_update_whole_screen()
{
	vid_debug_render();
		
	draw_strings();
	vid_debug_strings_render();

	vid_render(video_data.surface, 0, 0, 0, 0);
}

// when resizing a window, make sure the aspect ratio is preserved
void vid_notify_window_size_changed(Uint32 windowID)
{
	if (video_data.window == 0)
	{ 
		printf("vid_notify_window_size_changed(): ERROR: received window resize event, but no window!\n");
		return; 
	}

	Uint32 current_window_id = SDL_GetWindowID(video_data.window);
	if (current_window_id == 0)
	{
		printf("vid_notify_window_size_changed(): ERROR: received window resize event, but unable to determine current window id: %s\n", SDL_GetError());
		return;
	}

	if (current_window_id != windowID) { return; }

	int window_width, window_height;
	SDL_GetWindowSize(video_data.window, &window_width, &window_height);

	if (video_data.texture == 0) 
	{
		printf("vid_notify_window_size_changed(): ERROR: received window resize event, but no backing texture!\n");
		return; 
	}

	Uint32 format;
	int access, texture_width, texture_height;
	if (SDL_QueryTexture(video_data.texture, &format, &access, &texture_width, &texture_height) != 0)
	{
		printf("vid_notify_window_size_changed(): ERROR: received window resize event, but unable to determine texture size: %s\n", SDL_GetError());
		return;
	}

	int new_window_width = window_width;
	int new_window_height = window_width * texture_height / texture_width;

	// we have to check size is different or we repeatedly get change events.
	if ((new_window_height != window_height) || (new_window_width != window_width))
	{
		SDL_SetWindowSize(video_data.window, new_window_width, new_window_height);
		vid_render(video_data.surface, 0, 0, texture_width, texture_height);
	}
}

extern u8 *gfx_picbuff;	// created in gfx_init();

void vid_debug_render()
{
    int res = -1;


    res = SDL_RenderClear(video_data.debug_renderer);
    assert(res == 0);


	u8 *dest_px = (u8 *)video_data.debug_surface->pixels;
	for (int i = 0; i < 160*168 ; i ++)
	{
		dest_px[i] = (gfx_picbuff[i] >> 4) & 0xF;
	}


    SDL_Surface *textureSurface = 0;
    res = SDL_LockTextureToSurface(video_data.debug_texture, 0, &textureSurface);
    assert(res == 0);
    assert(textureSurface != 0);

    res = SDL_BlitSurface(video_data.debug_surface, 0, textureSurface, 0);
    assert(res == 0);

    SDL_UnlockTexture(video_data.debug_texture);


    res = SDL_RenderCopy(video_data.debug_renderer, video_data.debug_texture, 0, 0);
    assert(res == 0);

    SDL_RenderPresent(video_data.debug_renderer);
}


void draw_char(SDL_Surface *surface, int col, int row, int ch, int dos_char_fg)
{
	int total_rows = video_data.debug_strings_surface->h / 8;
	int total_cols = video_data.debug_strings_surface->w / 8;


	if (row < 0) { return; }
	if (row >= total_rows) { return; }
	if (col < 0) { return; }
	if (col >= total_cols) { return; }

	// int dos_char_fg = 0xf;
	int dos_char_bg = 0;


	unsigned char *pixels = (unsigned char *)surface->pixels;

	uint8_t *bptr = &IBM_VGA_8x8[ch*8];

	for (int y = 0; y < 8; y++) {
		uint8_t b = *bptr++;
		uint8_t *dest = pixels + col*8 + (row*8+y)*surface->pitch ;
		for (int x = 0; x < 8; x++) {
			*dest++ = (b & 0x80) ? dos_char_fg : dos_char_bg;
			b <<= 1;
		}
	}
}

void print_string(SDL_Surface *surface, int col, int row, const char *string, int colour)
{
	while(*string) {
		draw_char(surface, col, row, *string, colour);
		col += 1;
		string++;
	}
}

extern char *sentences_seen[1000];
extern int sentences_count;
extern int is_sentence_entered(const char *sentence);


void reconstruct_debug_strings()
{
	int res;

	int win_w, win_h;
	SDL_GetWindowSize(video_data.debug_strings_window, &win_w, &win_h);

	SDL_RenderSetLogicalSize(video_data.debug_strings_renderer, win_w, win_h);

 	if (video_data.debug_strings_palette == 0) {
		// You must allocate 256 colours even though we're only used 16
		// This is because SDL doesn't check the palette range and will
		// try to go out of bounds if our surface has colours outside 16
		video_data.debug_strings_palette = SDL_AllocPalette(256);
		assert(video_data.debug_strings_palette != 0);
	}

    // set some ugly colour for if we accidentally go out of the 16 colour range.
    for (int i = 0; i < 256; i++) {
        SDL_Color c = {0xFF,0x00,0xFF,PALETTE_ALPHA};
        res = SDL_SetPaletteColors(video_data.debug_strings_palette, &c, i, 1);
        assert(res == 0);
    }



	for (int i = 0; i < 16; i++) {

		res = SDL_SetPaletteColors(video_data.debug_strings_palette, &ega_colour_map[palette_ega_default[i]], i, 1);
		assert(res == 0);
	}


	if (video_data.debug_strings_surface) {
		SDL_FreeSurface(video_data.debug_strings_surface);
		video_data.debug_strings_surface = 0;
	}

    video_data.debug_strings_surface = SDL_CreateRGBSurface(0, win_w, win_h, 8, 0, 0, 0, 0);
    assert(video_data.debug_strings_surface != 0);

    res = SDL_SetSurfacePalette(video_data.debug_strings_surface, video_data.debug_strings_palette);
    assert(res == 0);


	if (video_data.debug_strings_texture) {
		SDL_DestroyTexture(video_data.debug_strings_texture);
		video_data.debug_strings_texture = 0;
	}

    if (video_data.debug_strings_texture == 0) {
        video_data.debug_strings_texture = SDL_CreateTexture(video_data.debug_strings_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING, win_w, win_h);
        assert(video_data.debug_strings_texture != 0);
        res = SDL_SetTextureBlendMode(video_data.debug_strings_texture, SDL_BLENDMODE_NONE);
        assert(res == 0);
    }

}


void draw_strings()
{
	int win_w, win_h;
	SDL_GetWindowSize(video_data.debug_strings_window, &win_w, &win_h);

	if (win_w != video_data.debug_strings_surface->w || win_h != video_data.debug_strings_surface->h)
	{
		reconstruct_debug_strings();
	}

	SDL_FillRect(video_data.debug_strings_surface, 0, 0);
	int total_rows;

	total_rows = video_data.debug_strings_surface->h / 8;

	int row = 0;
	int col = 0;

	for (int i = 0; i < sentences_count; i++) {

		if (is_sentence_entered(sentences_seen[i])) {
			print_string(video_data.debug_strings_surface, col*20, row, sentences_seen[i], 0xE);
		}
		else {
			print_string(video_data.debug_strings_surface, col*20, row, sentences_seen[i], 0xF);
		}

		row += 1;
		if (row >= total_rows) {
			row = 0;
			col += 1;
		}

	}


}

void vid_debug_strings_render()
{
    int res = -1;


    res = SDL_RenderClear(video_data.debug_strings_renderer);
    assert(res == 0);


    SDL_Surface *textureSurface = 0;
    res = SDL_LockTextureToSurface(video_data.debug_strings_texture, 0, &textureSurface);
    assert(res == 0);
    assert(textureSurface != 0);

    res = SDL_BlitSurface(video_data.debug_strings_surface, 0, textureSurface, 0);
    assert(res == 0);

    SDL_UnlockTexture(video_data.debug_strings_texture);

    res = SDL_RenderCopy(video_data.debug_strings_renderer, video_data.debug_strings_texture, 0, 0);
    assert(res == 0);

    SDL_RenderPresent(video_data.debug_strings_renderer);
}


static void vid_render(SDL_Surface *surface, const u32 x, const u32 y, const u32 w, const u32 h)
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

static int shake_offset[] = {25, 0, -25};

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
