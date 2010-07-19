/* FUNCTION list 	---	---	---	---	---	---	---

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
#include "drv_video.h"

#include "mem_wrap.h"

#include "sdl_vid.h"



/* PROTOTYPES	---	---	---	---	---	---	--- */


/* VARIABLES	---	---	---	---	---	---	--- */


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


AGISIZE prev_size;
u32 prev_flags;

// create an 8bit window with such a size
// clear it with colour 0
// 1= fullscreen
// 0 = window
void vid_display(AGISIZE *screen_size, int fullscreen_state)
{
	u32 sdl_flags;
	SDL_Surface *sdl_surface;
	
	// SDL_HWSURFACE doesn't work too well for fullscreen
	// or windibSDL_HWPALETTE 
	sdl_flags = SDL_SWSURFACE;
	if (fullscreen_state)
		sdl_flags |= SDL_FULLSCREEN;

	if ( (prev_size.w !=  screen_size->w) || 
		(prev_size.h != screen_size->h) ||
		(prev_flags != sdl_flags) )
	{
		sdl_surface = SDL_SetVideoMode(screen_size->w,
								screen_size->h,
								8, sdl_flags);
				
		prev_size.w = screen_size->w;
		prev_size.h = screen_size->h;
		prev_flags = sdl_flags;
		
		if (sdl_surface == 0)
		{
			printf("Unable to create video surface: %s\n", SDL_GetError());
			agi_exit();
		}
		
	}

	sdl_surface = SDL_GetVideoSurface();
	assert(sdl_surface);
	
	// clear
	vid_lock();
		SDL_FillRect(sdl_surface, 0, 0);
		SDL_UpdateRect(sdl_surface, 0,0,0,0);
		SDL_UnlockSurface(sdl_surface);
	vid_unlock();
}

void vid_free()
{
	//SDL_Surface *sdl_surface;
	//sdl_surface = SDL_GetVideoSurface();
	
	//SDL_FreeSurface( SDLPTR(vsurface) );
	//memset(vsurface, 0, sizeof(VSURFACE) );
	//a_free(vsurface);
}

void *vid_getbuf()
{
	SDL_Surface *surface;
	surface = SDL_GetVideoSurface();
	assert(surface);

	return surface->pixels;
}

int vid_getlinesize()
{
	SDL_Surface *surface;
	surface = SDL_GetVideoSurface();
	assert(surface);

	return surface->pitch;
}



void vid_lock()
{

	SDL_Surface *surface;
	surface = SDL_GetVideoSurface();
	assert(surface);
	
	if (SDL_LockSurface(surface) != 0)
	{
		printf("vid_lock(): Unable to lock video surface: %s\n", SDL_GetError());
		agi_exit();
	}

}

void vid_unlock()
{

	SDL_Surface *surface;
	surface = SDL_GetVideoSurface();
	assert(surface);
	
	SDL_UnlockSurface(surface);

}


// update a surface
void vid_update(POS *pos, AGISIZE *size)
{
	SDL_Surface *surface;
	

	surface = SDL_GetVideoSurface();
	assert(surface);
	
	// check boundaries
	if ((pos->x + size->w) > surface->w)
		size->w = surface->w - pos->x;
	if ((pos->y + size->h) > surface->h)
		size->h = surface->h - pos->y;
	
	SDL_UpdateRect(surface, pos->x, pos->y, size->w, size->h);

}

// set 8-bit palette
void vid_palette_set(PCOLOUR *palette, u8 num)
{
	SDL_Color sdl_palette[num];
	int i;
	SDL_Surface *surface;

	surface = SDL_GetVideoSurface();
	assert(surface);

	for (i=0; i<num; i++)
	{
		sdl_palette[i].r = palette[i].r;
		sdl_palette[i].g = palette[i].g;
		sdl_palette[i].b = palette[i].b;
		sdl_palette[i].unused = 0;
	}

	SDL_SetPalette(surface, SDL_LOGPAL|SDL_PHYSPAL,
				sdl_palette, 0, num);

}




//derived:
//-----------------------------------------------------------

// fill the surface at a particular point.. rectangle
// can use library's fast blit's 'n stuff
void vid_fill(POS *pos, AGISIZE *size, u32 colour)
{
	SDL_Surface *surface;


	vid_lock();
	
	surface = SDL_GetVideoSurface();
	assert(surface);
	
	if ( (pos->x|pos->y|size->w|size->h) == 0)
		SDL_FillRect(surface, 0, colour);
	{
		SDL_Rect rect;
	
		rect.x = pos->x;
		rect.y = pos->y;
		rect.w = size->w;
		rect.h = size->h;
	
		SDL_FillRect(surface, &rect, colour);
	}
	
	vid_unlock();

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
	
	surface = SDL_GetVideoSurface();
	assert(surface);
	

	//	get screen width, height, depth, ptic... etc etc
	bpp = surface->format->BitsPerPixel;
	rmask = surface->format->Rmask;
	gmask = surface->format->Gmask;
	bmask = surface->format->Bmask;
	amask = surface->format->Amask;
	width =  surface->w;
	height =  surface->h;
		
	// 	create new surface
	orig = SDL_CreateRGBSurface(0, width, height, bpp, rmask, gmask, bmask, amask);
	if (!orig) return;

	orig->format->palette->ncolors = surface->format->palette->ncolors ;
	memcpy (orig->format->palette->colors, surface->format->palette->colors, orig->format->palette->ncolors * sizeof (SDL_Color)) ;
	
	// 	blit screen to new surface
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
		SDL_UpdateRect(surface, 0, 0, 0, 0);
		
		SDL_Delay(50);
	}
	
	// put the original screen back on
	if (SDL_BlitSurface(orig, 0, surface, 0)) // update the screen
		goto shake_error; 
	SDL_UpdateRect(surface, 0, 0, 0, 0);
	
shake_error:
	SDL_FreeSurface(orig);
}
