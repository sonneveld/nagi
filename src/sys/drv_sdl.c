/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>

/* OTHER headers	---	---	---	---	---	---	--- */

#include "../base.h"
#include "drv_video.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
void sdl_driver_shutdown(void);
VSURFACE *sdl_display(VSURFACE *vsurface, SIZE *screen_size, int fullscreen_state);
void sdl_free(VSURFACE *vsurface);
void sdl_lock(VSURFACE *vsurface);
void sdl_unlock(VSURFACE *vsurface);
void sdl_update(VSURFACE *vsurface, POS *pos, SIZE *size);
void sdl_palette_set(VSURFACE *vsurface, PCOLOUR *palette, u8 num);
void sdl_fill(VSURFACE *vsurface, POS *pos, SIZE *size, u32 colour);

/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


#define SDLPTR(vsurface) ((SDL_Surface *)(vsurface->system_surface))

// elemental
//-----------------------------------------------------------


void sdl_driver_init(VDRIVER *drv)
{
	printf("Initialising SDL video subsystem... ");
	
	drv->ptr_shutdown = sdl_driver_shutdown;
	drv->ptr_display = sdl_display;
	drv->ptr_free = sdl_free;
	drv->ptr_lock = sdl_lock;
	drv->ptr_unlock = sdl_unlock;
	drv->ptr_update = sdl_update;
	drv->ptr_palette_set = sdl_palette_set;

	drv->ptr_fill = sdl_fill;
	
	if (SDL_InitSubSystem(SDL_INIT_VIDEO) != 0)
	{
		printf("sdl_driver_init(): unable to initialise SDL video subsystem.\n");
		agi_exit();
	}
	
	printf("done.\n");
}

void sdl_driver_shutdown()
{
	//SDL_QuitSubSystem(SDL_INIT_VIDEO);
}


SIZE prev_size;
u32 prev_flags;

// create an 8bit window with such a size
// clear it with colour 0
// 1= fullscreen
// 0 = window
VSURFACE *sdl_display(VSURFACE *vsurface, SIZE *screen_size, int fullscreen_state)
{
	u32 sdl_flags;
	SDL_Surface *sdl_surface;
	
	if (vsurface == 0)
	{
		vsurface = (VSURFACE*)a_malloc(sizeof(VSURFACE));
		memset(vsurface, 0, sizeof(VSURFACE));
	}
	
	// SDL_HWSURFACE doesn't work too well for fullscreen
	// or windibSDL_HWPALETTE 
	sdl_flags = 0;
	if (fullscreen_state)
		sdl_flags |= SDL_FULLSCREEN;

	if ( (prev_size.w !=  screen_size->w) || 
		(prev_size.h != screen_size->h) ||
		(prev_flags != sdl_flags) )
	{
		vsurface->size.w = screen_size->w;
		vsurface->size.h = screen_size->h;
		
		sdl_surface = SDL_SetVideoMode(vsurface->size.w,
								vsurface->size.h,
								8, sdl_flags);
		
		prev_size.w = vsurface->size.w;
		prev_size.h = vsurface->size.h;
		prev_flags = sdl_flags;
		
		if (sdl_surface == 0)
		{
			printf("Unable to create video surface: %s\n", SDL_GetError());
			agi_exit();
		}
		
		vsurface->pixels = sdl_surface->pixels;
		vsurface->line_size = sdl_surface->pitch;
		vsurface->pixel_size = sdl_surface->pitch / vsurface->size.w;
		vsurface->system_surface = (void *)sdl_surface;
	}
	else
		sdl_surface = vsurface->system_surface;
	
	// clear
	if ( (SDL_MUSTLOCK(sdl_surface) && (!SDL_LockSurface(sdl_surface)) ) ||
		(!SDL_MUSTLOCK(sdl_surface)) )
	{
		SDL_FillRect(sdl_surface, 0, 0);
		SDL_UpdateRect(sdl_surface, 0,0,0,0);
		if (SDL_MUSTLOCK(sdl_surface))
			SDL_UnlockSurface(sdl_surface);
	}

	return vsurface;
}

void sdl_free(VSURFACE *vsurface)
{
	//SDL_FreeSurface( SDLPTR(vsurface) );
	memset(vsurface, 0, sizeof(VSURFACE) );
	a_free(vsurface);
}

// lock a surface
void sdl_lock(VSURFACE *vsurface)
{
	if ( SDL_MUSTLOCK( SDLPTR(vsurface) ) )
		if (SDL_LockSurface(SDLPTR(vsurface)) != 0)
		{
			printf("SDL: unable to lock surface\n");
			agi_exit();
		}
}

// unlock a surface for editing.
void sdl_unlock(VSURFACE *vsurface)
{
	if (SDL_MUSTLOCK(SDLPTR(vsurface)))
		SDL_UnlockSurface(SDLPTR(vsurface));
}

// update a surface
void sdl_update(VSURFACE *vsurface, POS *pos, SIZE *size)
{
	// check boundaries
	if ((pos->x + size->w) > vsurface->size.w)
		size->w = vsurface->size.w - pos->x;
	if ((pos->y + size->h) > vsurface->size.h)
		size->h = vsurface->size.h - pos->y;
	
	SDL_UpdateRect(SDLPTR(vsurface), pos->x, pos->y, size->w, size->h);
}

// set 8-bit palette
void sdl_palette_set(VSURFACE *vsurface, PCOLOUR *palette, u8 num)
{
	SDL_Color sdl_palette[num];
	int i;

	for (i=0; i<num; i++)
	{
		sdl_palette[i].r = palette[i].r;
		sdl_palette[i].g = palette[i].g;
		sdl_palette[i].b = palette[i].b;
		sdl_palette[i].unused = 0;
	}

	SDL_SetPalette(SDLPTR(vsurface), SDL_LOGPAL|SDL_PHYSPAL,
				sdl_palette, 0, num);
}




//derived:
//-----------------------------------------------------------

// fill the surface at a particular point.. rectangle
// can use library's fast blit's 'n stuff
void sdl_fill(VSURFACE *vsurface, POS *pos, SIZE *size, u32 colour)
{
	
	if ( (pos->x|pos->y|size->w|size->h) == 0)
		SDL_FillRect(SDLPTR(vsurface), 0, colour);
	{
		SDL_Rect rect;
	
		rect.x = pos->x;
		rect.y = pos->y;
		rect.w = size->w;
		rect.h = size->h;
	
		SDL_FillRect(SDLPTR(vsurface), &rect, colour);
	}
}
