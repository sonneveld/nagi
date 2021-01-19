/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

/* OTHER headers	---	---	---	---	---	---	--- */
#include "sound_base.h"
#include "sound_gen.h"
#include "tone.h"

#include "../flags.h"
#include "../sys/endian.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */

struct sndgen_channel_struct
{
	u8 *data;
	u16 duration;
	u16 avail;	// turned on (1)  but when the channel's data runs out, it's set to (0)
	u16 dissolve_count;
	u8 attenuation;
	u8 attenuation_copy;
	
	int tone_handle;
	int gen_type;
	
	// for the sample mixer
	int freq_count;

};
typedef struct sndgen_channel_struct SNDGEN_CHAN;


/* VARIABLES	---	---	---	---	---	---	--- */
#define CHAN_MAX 4

// "fade out" or possibly "dissolve"
// v2.9xx
s8 dissolve_data_v2[] =
{
	-2, -3, -2, -1, 0x00, 0x00, 0x01, 0x01, 0x01,
	0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
	0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
	0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
	0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07,
	0x07, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09,
	0x09, 0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B,
	0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
	0x0C, 0x0D, -100
}; 

// v3
s8 dissolve_data_v3[] =
{
-2, -3, -2, -1,
0, 0, 0, 0, 0, 
1, 1, 1, 1, 1, 1,
2, 2, 2, 2, 2, 2, 2, 2, 2, 2, 
3, 3, 3, 3, 3, 3, 3, 3, 
4, 4, 4, 4, 4,
5, 5, 5, 5, 5, 
6, 6, 6, 6, 6, 
7, 7, 7, 7, 
8, 8, 8, 8,
9, 9, 9, 9,
0xA, 0x0A, 0x0A, 0xA,
0x0B, 0x0B, 0x0B, 0x0B, 0x0B, 0x0B,
0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
0x0D,
-100
};


//u8 channels_left = 0;
SNDGEN_CHAN channel[CHAN_MAX];

		
/* CODE	---	---	---	---	---	---	---	--- */

void sndgen_init(void)
{
	if (c_snd_enable)
	{
		// init tone_gen
		if (tone_init())
			c_snd_enable = 0;
		else
			tone_state_set(0);
	}
}

void sndgen_shutdown(void)
{
	// shutdown tone_gen
	if (c_snd_enable)
	{
		sndgen_stop();
		tone_shutdown();
	}
}

void sndgen_play(SOUND *snd)
{
	int i;
	
	assert(snd);
	
	if (c_snd_enable)
	{
		for (i=0; i<(c_snd_single?1:CHAN_MAX); i++)
		{
			channel[i].data = snd->channel[i];
			channel[i].duration = 0;
			channel[i].dissolve_count = 0xFFFF;
			channel[i].avail = 0xFFFF;
			channel[i].freq_count = 0;
			channel[i].tone_handle = tone_open(i);
			
			if (channel[i].tone_handle == 0)
			{
				printf("%s(): error opening tone channel.\n", __PRETTY_FUNCTION__);
				sndgen_shutdown();
				c_snd_enable = 0;
				return;
			}
		}
		
		// we're assuming 4 channel tandy/pcjr here anyways
		//channels_left = CHAN_MAX;	// channels
		
		sound_state = 1;
		
		//sound_vector();
		
		tone_state_set(1);
	}
	else
	{
		flag_set(sound_flag);
	}}


void sndgen_kill_thread(void)
{
	int i;
		
	sound_state = 0;
	tone_state_set(0);
	for (i=0; i<(c_snd_single?1:CHAN_MAX) ; i++)
	{
		if (channel[i].tone_handle != 0)
			tone_close(channel[i].tone_handle);
	}
	flag_set(sound_flag);
}


void sndgen_stop(void)
{
	if (c_snd_enable)
	{
		tone_lock();
		sndgen_kill_thread();
		tone_unlock();
	}}



int volume_calc(SNDGEN_CHAN *chan)
{
	s8 al, dissolve_value;
	
	u8 *dissolve_data;
	
	
	switch (c_snd_dissolve)
	{
		case 2:
			dissolve_data = dissolve_data_v2;
			break;
		case 3:
		default:
			dissolve_data = dissolve_data_v3;
			break;
	}
	
	// blah
	assert(chan);

	al = chan->attenuation;
	if (al != 0x0F)		// != silence
	{
		if (chan->dissolve_count != 0xFFFF) 
		{
			dissolve_value = dissolve_data[chan->dissolve_count];
			if (dissolve_value == -100)	// if at end of list
			{
				chan->dissolve_count = 0xFFFF;
				chan->attenuation = chan->attenuation_copy;
				al = chan->attenuation;
			}
			else
			{
				chan->dissolve_count++;
				
				al += dissolve_value;
				if (al < 0)
					al = 0;
				if (al > 0x0F)
					al = 0x0F;
				
				chan->attenuation_copy = al;
				
				al &= 0x0F;
				if (c_snd_read_var)
					al += state.var[V23_SNDVOL];
				if (al > 0x0F)
					al = 0x0F;
			}
		}
		//if (computer_type == 2) && (al < 8)
		if (al < 8)
			al += 2;
	}

	return al;
}

// read the next channel data.. fill it in *tone
// if tone isn't touched.. it should be inited so it just plays silence
// return 0 if it's passing more data
// return -1 if it's passing nothing (end of data)
int sndgen_callback(int ch, TONE *tone)
{
	SNDGEN_CHAN *chan;
	void *data;

	assert(tone);
	assert(ch < (c_snd_single?1:CHAN_MAX));
	
	if ( !flag_test(F09_SOUND) )
		return -1;
	
	chan = &channel[ch];
	if (!chan->avail)
		return -1;

	while ( (chan->duration == 0) && (chan->duration != 0xFFFF) )
	{
		data = chan->data;
		
		// read the duration of the note
		chan->duration = load_le_16(data);	// duration
		
		// if it's 0 then it's not going to be played
		// if it's 0xFFFF then the channel data has finished.
		if ( (chan->duration != 0) && (chan->duration != 0xFFFF))
		{
			// only tone channels dissolve
			if ((ch != 3) && (c_snd_dissolve != 0))	// != noise??
				chan->dissolve_count = 0;
			
			// attenuation (volume)
			chan->attenuation = ((u8*)data)[4] & 0xF;	
			
			// frequency
			if (ch < (CHAN_MAX-1))
			{
				chan->freq_count = (u16)((u8*)data)[2] & 0x3F;
				chan->freq_count <<= 4;
				chan->freq_count |= ((u8*)data)[3] & 0x0F;
				
				chan->gen_type = GEN_TONE;
				
#warning if chan 2.. change noise chan if appropriate
			}
			else
			{
				int noise_freq;
				
				// check for white noise (1) or periodic (0)
				chan->gen_type = (((u8*)data)[3] & 0x04) ? 
						GEN_WHITE : GEN_PERIOD;
				
				noise_freq = ((u8*)data)[3] & 0x03;
				
				switch (noise_freq)
				{
					case 0:
						chan->freq_count = 32;
						break;
					case 1:
						chan->freq_count = 64;
						break;
					case 2:
						chan->freq_count = 128;
						break;
					case 3:
						chan->freq_count = channel[2].freq_count*2;
						break;
				}
			}
		}
		// data now points to the next data seg-a-ment
		chan->data += 5;
	}
	
	if (chan->duration != 0xFFFF)
	{
		tone->freq_count = chan->freq_count;
		tone->atten = volume_calc(chan);	// calc volume, sent vol is different from saved vol
		tone->type = chan->gen_type;
		chan->duration --;
	}
	else
	{
		// kill channel
		//channels_left --;
		chan->avail = 0;
		chan->attenuation = 0x0F;	// silent
		chan->attenuation_copy = 0x0F;	// dunno really
		return -1;
	}
	
	return 0;
}
	

