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
#include "../sys/drvpick.h"


#include "sound_base.h"
#include "sound_gen.h"
#include "tone.h"

#include "tone_pcm.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */


/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

// tone  interface

DRVINIT tone_init_list[] =
{
	/*{"speaker", 0},
	{"pcm_16bit", 0},
	{"pcm_8bit", 0},
	{"midi", 0}*/
	{"pcm_16bit", tone_pcm_drv_init}
};

u8 c_snd_tone_drv[] = "pcm_16bit";

TONE_DRIVER tone_drv;

int tone_init(void)
{
	DRVINITSTATE initstate;
	int drv_avail;
	
	drv_avail = drvpick_first(&initstate, tone_init_list, 
			sizeof(tone_init_list) / sizeof(DRVINIT), 
			c_snd_tone_drv, &tone_drv);
	
	while (!drv_avail)
	{
		if (!tone_drv.ptr_init())
			return 0;
		drv_avail = drvpick_next(&initstate);
	}
	return -1;
}

void tone_shutdown(void)
{
	// has to be able to shutdown even if it didn't init
	tone_drv.ptr_shutdown();
}

// returns handle
int tone_open(int ch, int type)
{
	return tone_drv.ptr_open(ch, type);
}

void tone_close(int handle)
{
	assert(handle);
	tone_drv.ptr_close(handle);
}

void tone_state_set(int sound_state)
{
	tone_drv.ptr_state_set(sound_state);
}

int tone_state_get(void)
{
	return tone_drv.ptr_state_get();
}

void tone_lock(void)
{
	tone_drv.ptr_lock();
}

void tone_unlock(void)
{
	tone_drv.ptr_unlock();
}

