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
#include "mem_wrap.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
void dummy_drv_shutdown(void);
VSURFACE *dummy_display(VSURFACE *vsurface, SIZE *screen_size, int fullscreen_state);
void dummy_free(VSURFACE *vsurface);
void dummy_lock(VSURFACE *vsurface);
void dummy_unlock(VSURFACE *vsurface);
void dummy_vid_update(VSURFACE *vsurface, POS *pos, SIZE *size);
void dummy_palette_set(VSURFACE *vsurface, PCOLOUR *palette, u8 num);
void dummy_fill(VSURFACE *vsurface, POS *pos, SIZE *size, u32 colour);

/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


// elemental
//-----------------------------------------------------------


VSURFACE dummy_surface = {0, {0,0}, 0,0,0};

void dummy_driver_init(VDRIVER *drv)
{
	drv->ptr_shutdown = dummy_drv_shutdown;
	drv->ptr_display = dummy_display;
	drv->ptr_free = dummy_free;
	drv->ptr_lock = dummy_lock;
	drv->ptr_unlock = dummy_unlock;
	drv->ptr_update = dummy_vid_update;
	drv->ptr_palette_set = dummy_palette_set;
	
	drv->ptr_fill = dummy_fill;
}


void dummy_drv_shutdown(void)
{
	printf("dummy_shutdown() called.\n");
}
// create an 8bit window with such a size
// clear it with colour 0
// 1= fullscreen
// 0 = window
VSURFACE *dummy_display(VSURFACE *vsurface, SIZE *screen_size, int fullscreen_state)
{
	printf("dummy_display() called.\n");
	
	memset(&dummy_surface, 0, sizeof(VSURFACE) );
	
	dummy_surface.size.w = screen_size->w;
	dummy_surface.size.h = screen_size->h;
	dummy_surface.pixels = a_malloc(screen_size->w * screen_size->h * sizeof(u8));
	
	return &dummy_surface;
}

void dummy_free(VSURFACE *vsurface)
{
	printf("dummy_free() called.\n");
	a_free(dummy_surface.pixels);
	memset(&dummy_surface, 0, sizeof(VSURFACE) );
}

// lock a surface
void dummy_lock(VSURFACE *vsurface)
{
	printf("dummy_lock() called.\n");
}

// unlock a surface for editing.
void dummy_unlock(VSURFACE *vsurface)
{
	printf("dummy_unlock() called.\n");
}

// update a surface
void dummy_vid_update(VSURFACE *vsurface, POS *pos, SIZE *size)
{
	printf("dummy_update() called.\n");
}

// set 8-bit palette
void dummy_palette_set(VSURFACE *vsurface, PCOLOUR *palette, u8 num)
{
	printf("dummy_palette_set() called.\n");
}


// to do.. mouse stuff?
// blitting for other surfaces
// fullscreen toggle

//derived:
//-----------------------------------------------------------

// fill the surface at a particular point.. rectangle
// can use library's fast blit's 'n stuff 
void dummy_fill(VSURFACE *vsurface, POS *pos, SIZE *size, u32 colour)
{
	printf("dummy_fill() called.\n");
}