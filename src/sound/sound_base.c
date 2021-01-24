/*
_SndListInit                     cseg     00004FF5 0000000F
_SndListNewRoom                  cseg     00005004 0000000C
_SoundFind                       cseg     00005010 00000032
CmdLoadSound                     cseg     00005042 0000001C
_SoundLoad                       cseg     0000505E 000000AD
CmdSound                         cseg     0000510B 00000052
CmdStopSound                     cseg     0000515D 0000000F
_SoundStop                       cseg     0000516C 00000023
*/

#include <stdio.h>
#include <stdlib.h>

#include "../agi.h"

#include "sound_base.h"
#include "sound_gen.h"

// blists_update
#include "../view/obj_base.h" 
#include "../view/obj_update.h"

#include "../flags.h"
#include "../res/res.h"
// byte-order support
#include "../sys/endian.h"
#include "../sys/mem_wrap.h"
#include "../list.h"
#include "../sys/script.h"

#include <setjmp.h>
#include "../sys/error.h"

static SOUND *sound_find(u16 snd_num);

volatile int sound_state = 0;  // 0=silence  1=playing
u16 sound_flag = 0;		// the flag to set when the sound is finished

static LIST *sound_list = 0;

void sound_list_init(void)
{
	if (sound_list)
		list_clear(sound_list);
	else
		sound_list = list_new(sizeof(SOUND));	
}

void sound_list_new_room(void)
{
	sound_list_init();
}

static SOUND *sound_find(u16 snd_num)
{
	SOUND *s;
	
	s = list_element_head(sound_list);
	
	while ( (s) && (snd_num != s->num) )
		s = node_next(s);
	
	return s;
}

u8 *cmd_load_sound(u8 *c)
{
	sound_load( *(c++) );
	return c;
}

SOUND *sound_load(u16 snd_num)
{
	SOUND *snd;
	
	snd = sound_find(snd_num);
	if (snd==0) 
	{
		u16 c;
		u8 *dptr;

		blists_erase();
		
		snd = (SOUND*)list_add(sound_list);
		script_write(3, snd_num);
		snd->num = snd_num;
		snd->data = vol_res_load(dir_sound(snd_num), 0);
		
		dptr = snd->data;
		for (c=0; c<4; c++)
		{
			snd->channel[c] = snd->data + load_le_16(dptr);
			dptr += sizeof(u16);	// word.. dude
		}
		
		blists_draw();
	}
	return snd;
}

u8 *cmd_sound(u8 *c)
{
	SOUND *snd;
	u8 num;

	sound_stop();
	
	num = *(c++);
	sound_flag = *(c++);
	flag_reset(sound_flag);
	
	snd = sound_find(num);
	if ( snd == 0)
		set_agi_error(0, num);
	
	sndgen_play(snd);
	return c;
}

u8 *cmd_stop_sound(u8 *c)
{
	sound_stop();
	return c;
}

void sound_stop(void)
{
	if (sound_state)
	{
		sndgen_stop();
	}
}
