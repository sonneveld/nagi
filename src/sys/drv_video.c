/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);
//#include "video.h"
//#include "vid_render.h"

//#include "vid_sdl.h"
//#include "vid_dummy.h"

#include "drv_video.h"
#include "drv_sdl.h"
#include "drv_dummy.h"


/* VARIABLES	---	---	---	---	---	---	--- */



/* CODE	---	---	---	---	---	---	---	--- */

// to do.. mouse stuff?
// blitting to and fro other surfaces
// fullscreen toggle

VDRIVER vdriver;

void vid_init(void)
{
	// init video (scale x size of renderer)
	if (strcasecmp (c_vid_driver, "SDL") == 0)
		sdl_driver_init(&vdriver);
	else
		dummy_driver_init(&vdriver);
}

void vid_shutdown(void)
{
	vdriver.ptr_shutdown();
}

VSURFACE *vid_display(VSURFACE *vsurface, SIZE *screen_size, int fullscreen_state)
{
	assert(screen_size != 0); 
	return vdriver.ptr_display(vsurface, screen_size, fullscreen_state);
}

void vid_free(VSURFACE *vsurface)
{
	assert(vsurface != 0);
	vdriver.ptr_free(vsurface);
}

void vid_lock(VSURFACE *vsurface)
{
	assert(vsurface != 0);
	vdriver.ptr_lock(vsurface);
}

void vid_unlock(VSURFACE *vsurface)
{
	assert(vsurface != 0);
	vdriver.ptr_unlock(vsurface);
}

void vid_update(VSURFACE *vsurface, POS *pos, SIZE *size)
{
	assert(vsurface != 0);
	assert(pos != 0);
	assert(size != 0);
	vdriver.ptr_update(vsurface, pos, size);
}

void vid_palette_set(VSURFACE *vsurface, PCOLOUR *palette, u8 num)
{
	assert(vsurface != 0);
	assert(palette != 0);
	vdriver.ptr_palette_set(vsurface, palette, num);
}

void vid_fill(VSURFACE *vsurface, POS *pos, SIZE *size, u32 colour)
{
	assert(vsurface != 0);
	assert(pos != 0);
	assert(size != 0);
	vdriver.ptr_fill(vsurface, pos, size, colour);
}


