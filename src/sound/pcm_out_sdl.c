
/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define WRITE_TO_DISK 0

#if WRITE_TO_DISK
#include <io.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#endif

/* OTHER headers	---	---	---	---	---	---	--- */
#include "../list.h"

#include "pcm_out.h"
#include "pcm_out_sdl.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */

struct sdl_channel_struct
{
	int handle;
	void (*callback)(void *userdata, u8 *stream, int len);
	void *userdata;
};
typedef struct sdl_channel_struct SDL_CHAN;

/* VARIABLES	---	---	---	---	---	---	--- */
int handle_next = 1;
LIST *chan_list = 0;


#if WRITE_TO_DISK
struct data_struct
{
	u8 *data;
	int len;
};
typedef struct data_struct DATA;
	
LIST *list_data = 0;
int file_handle = 0;
#endif


/* CODE	---	---	---	---	---	---	---	--- */


// initialises the driver pointers
void pcm_out_sdl_drv_init(void *void_ptr)
{
	PCM_OUT_DRIVER *drv;
	
	drv = (PCM_OUT_DRIVER *) void_ptr;
	assert(drv);
	
	drv->ptr_init = pcm_out_sdl_init;
	drv->ptr_shutdown = pcm_out_sdl_shutdown;
	drv->ptr_avail = pcm_out_sdl_avail;
	drv->ptr_open = pcm_out_sdl_open;
	drv->ptr_close = pcm_out_sdl_close;
	drv->ptr_state_set = pcm_out_sdl_state_set;
	drv->ptr_state_get = pcm_out_sdl_state_get;
	drv->ptr_lock = pcm_out_sdl_lock;
	drv->ptr_unlock = pcm_out_sdl_unlock;
	drv->type = PCM_OUT_SDL;
}


int pcm_out_sdl_init(int freq, int format)
{
	SDL_AudioSpec wanted;
	
	(void) freq;
	(void) format;
	
	printf("pcm_out_sdl_init(): Initialising SDL audio subsystem... ");
	
	if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
	{
		printf("\npcm_out_sdl_init(): unable to initialise SDL audio subsystem.\n");
		printf("%s\n", SDL_GetError());
		return -1;
	}
	
	// sets the freq/type
	wanted.freq = 44100;
	wanted.format = AUDIO_S16LSB;
	wanted.samples = 256;
	wanted.channels = 1;
	wanted.callback = sdl_callback;
	wanted.userdata = 0;
	
	// opens up calback
	if (SDL_OpenAudio(&wanted, 0) != 0)
	{
		printf("\npcm_out_sdl_init(): unable to open audio.\n");
		return -1;
	}
	
	// pauses callback
	pcm_out_sdl_state_set(0);
	
	#if WRITE_TO_DISK
	list_data = list_new(sizeof(DATA));
	#endif
	
	printf("done.\n");
	
	return 0;
}

void pcm_out_sdl_shutdown(void)
{

	// set state to stop
	pcm_out_sdl_state_set(0);	
	
	// close all remaining channels
	if (chan_list)
	{
		list_free(chan_list);
		chan_list=0;
	}
	
	// shutdown audio subsystem
	SDL_CloseAudio();
	
	SDL_QuitSubSystem(SDL_INIT_AUDIO);
	
	#if WRITE_TO_DISK
	if (list_data)
	{
		DATA *cur;
		int amount;
		
		cur = list_element_head(list_data);
		file_handle = open("sound.raw", O_BINARY|O_RDWR|O_CREAT, S_IREAD|S_IWRITE);
		printf("handle = %d\n", file_handle);
		while (cur)
		{
			printf("writing %d bytes...\n", cur->len);
			amount = write(file_handle, cur->data, cur->len);
			if ((amount != cur->len) || (amount <= -1))
			{
				printf("error! %d\n", amount);
				print_err_code();
			}
			cur = node_next(cur);
		}
		close(file_handle);
	}
	#endif
}

// get available options
void pcm_out_sdl_avail(void)
{
	// return 16bit, 8bit, 22khz, 41khz
}

// returns 0 if failed
// else returns handle
int pcm_out_sdl_open( void (*callback)(void *userdata, Uint8 *stream, int len), void *userdata)
{
	SDL_CHAN chan;
	SDL_CHAN *chan_new;
	
	pcm_out_sdl_lock();
	
	if (chan_list == 0)
		chan_list = list_new(sizeof(SDL_CHAN));

	chan.handle = handle_next++;
	chan.callback = callback;
	chan.userdata = userdata;
	
	chan_new = list_add(chan_list);
	
	memcpy(chan_new, &chan, sizeof(SDL_CHAN) );
	
	pcm_out_sdl_unlock();
	return chan.handle;
}

void pcm_out_sdl_close(int handle)
{
	SDL_CHAN *ch;
	(void) handle;
	
	pcm_out_sdl_lock();
	
	if (chan_list)
	{
		// find the one with a handle
		ch = list_element_head(chan_list);
		while ((ch) && (ch->handle != handle))
			ch=node_next(ch);
		
		// kill it
		if (ch)
			list_remove(chan_list, ch);
	}

	pcm_out_sdl_unlock();
}

// 1 = playing
// 0 = stopped
void pcm_out_sdl_state_set(int sound_state)
{
	SDL_PauseAudio(sound_state?0:1);
}

int pcm_out_sdl_state_get(void)
{
	return (SDL_GetAudioStatus() == SDL_AUDIO_PLAYING);
}

// lock audio thread so data can be changed
void pcm_out_sdl_lock(void)
{
	SDL_LockAudio();
}

// unlock thread
void pcm_out_sdl_unlock(void)
{
	SDL_UnlockAudio();
}



void sdl_callback(void *userdata, u8 *stream, int len)
{
	s16 chan_data[len / (int)sizeof(s16)];
	int stream_len, stream_count;
	s16 *s_ptr, *c_ptr;
	
	SDL_CHAN *ch;
	int num_chan;
	
	#if WRITE_TO_DISK
	DATA *new_data;
	#endif
	
	(void) userdata;
	
	assert(stream);
	
	if (chan_list != 0)
	{
		num_chan = list_length(chan_list);
		ch = list_element_head(chan_list);
		
		if (num_chan <= 1)
		{
			ch->callback( ch->userdata, (u8 *)&chan_data, len );
			memcpy(stream, chan_data, len);
		}
		else
		{
			stream_len = len / (int)sizeof(s16);
			
			while (ch)
			{
				// get channel data(chan.userdata)
				ch->callback( ch->userdata, (u8 *)&chan_data, len );
				
				// divide by number of channels then add to stream
				stream_count = stream_len;
				s_ptr = (s16*)stream;
				c_ptr = (s16*)chan_data;
				
				while(stream_count--)
					*(s_ptr++) += *(c_ptr++) / num_chan ;
				
				ch = node_next(ch);
			}
		}
	}
	
	
	#if WRITE_TO_DISK
	new_data = list_add(list_data);
	new_data->data = (u8 *)a_malloc(len);
	memcpy(new_data->data, stream, len);
	new_data->len = len;
	#endif
	
}

