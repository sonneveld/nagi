/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

/* OTHER headers	---	---	---	---	---	---	--- */
#include "pcm_out.h"
#include "pcm_out_sdl.h"

#include "../sys/drvpick.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

// pcm out interface

DRVINIT pcm_out_init_list[] =
{
	{"sdl", pcm_out_sdl_drv_init}
	//{"speaker", 0},
	//{"waveout", 0},
	//{"directx", 0}
};

PCM_OUT_DRIVER pcm_out_drv;

int pcm_out_init(int freq, int format)
{
	DRVINITSTATE initstate;
	int drv_avail;
	
	drv_avail = drvpick_first(&initstate, pcm_out_init_list, 
			sizeof(pcm_out_init_list) / sizeof(DRVINIT), 
			c_snd_driver, &pcm_out_drv);
	
	while (!drv_avail)
	{
		if (!pcm_out_drv.ptr_init(freq, format))
			return 0;
		drv_avail = drvpick_next(&initstate);
	}
	return -1;
}

void pcm_out_shutdown(void)
{
	pcm_out_drv.ptr_shutdown();
}

// get available options
void pcm_out_avail(void)
{
	pcm_out_drv.ptr_avail();
}

int pcm_out_open( int (*callback)(void *userdata, Uint8 *stream, int len), void *userdata)
{
	assert(callback);
	return pcm_out_drv.ptr_open(callback, userdata);
}

void pcm_out_close(int handle)
{
	assert(handle);
	pcm_out_drv.ptr_close(handle);
}

void pcm_out_state_set(int snd_state)
{
	pcm_out_drv.ptr_state_set(snd_state);
}

int pcm_out_state_get(void)
{
	return pcm_out_drv.ptr_state_get();
}

void pcm_out_lock(void)
{
	pcm_out_drv.ptr_lock();
}

void pcm_out_unlock(void)
{
	pcm_out_drv.ptr_unlock();
}

