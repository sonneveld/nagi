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

#include "../view/obj_base.h" 
#include "../view/obj_update.h"

#include "../sys/script.h"
#include <setjmp.h>
#include "../sys/error.h"

#include "../flags.h"
#include "../sound/sound.h"  


#include "../res/res.h"

// byte-order support
#include "../sys/endian.h"

#include "../sys/mem_wrap.h"

u16 sound_playing = 0;
SOUND sound_head = {0, 0, 0, {0,0,0,0}};
SOUND *sound_last = 0;
u16 sound_flag = 0;		// the flag to set when the sound is finished

void sound_list_init(void);
void sound_list_new_room(void);
SOUND *sound_find(u16 snd_num);
u8 *cmd_load_sound(u8 *c);
SOUND *sound_load(u16 snd_num);
u8 *cmd_sound(u8 *c);
u8 *cmd_stop_sound(u8 *c);
void sound_stop(void);


void sound_list_init()
{
	SOUND *cur;
	SOUND *next;
	
	cur = sound_head.next;
	
	while (cur != 0)
	{
		next = cur->next;
		if (cur->data != 0)
			a_free(cur->data);
		a_free(cur);
		cur = next;
	}
	
	sound_head.next = 0;
}

void sound_list_new_room()
{
	sound_list_init();
}

SOUND *sound_find(u16 snd_num)
{
	SOUND *cur, *prev;
	
	cur = sound_head.next;
	prev = &sound_head;
	goto loc502b;
loc5026:
	prev = cur;
	cur = cur->next;
loc502b:
	if ( cur == 0) goto loc5034;
	if ( snd_num == cur->num) goto loc5034;
	goto loc5026;
loc5034:
	sound_last = prev;
	return cur;
}

u8 *cmd_load_sound(u8 *c)
{
	sound_load( *(c++) );
	return c;
}

SOUND *sound_load(u16 snd_num)
{
	u16 c;
	u8 *di;
	SOUND *snd;
	
	snd = sound_find(snd_num);
	if ( snd == 0) 
	{
		blists_erase();
		
		if ( sound_last == 0)
			snd = &sound_head;
		else
		{
			snd = (SOUND *)a_malloc(sizeof(SOUND));
			sound_last->next = snd;
			snd->next = 0;
		}
		
		script_write(3, snd_num);
		snd->num = snd_num;
		snd->data = vol_res_load(dir_sound(snd_num), 0);
		
		c = 0;
		di = snd->data;
		while (c < 4)
		{
			snd->channel[c] = snd->data + load_le_16(di);
			c++;
			di += 2;	// word.. dude
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
	
	sound_new(snd);
	return c;
}

u8 *cmd_stop_sound(u8 *c)
{
	sound_stop();
	return c;
}

void sound_stop()
{
	if (sound_playing != 0)
	{
		sound_playing = 0;
		flag_set(sound_flag);
		sound_stop_sdl();
	}
}


