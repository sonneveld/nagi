#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "../agi.h"

#include "../sound/sound.h"  
#include "../flags.h"


#define CHAN_MAX 4
#define CARD_FREQ 44100
// create a 60hz sample
// this is 44100/60 samples I guess = 735 samples ( word)
// 44100 / 60 * 2 bytes
#define SND_BUFF_SIZE 1470

// byte-order support
#include "../sys/endian.h"

#include "../sys/mem_wrap.h"
	
// "fade out" or possibly "dissolve"
u8 dissolve_data[0x44] = {	0xFE, 0xFD, 0xFE, 0xFF, 0x00, 0x00, 0x01, 0x01, 0x01,
					0x01, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02, 0x02,
					0x02, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03, 0x03,
					0x04, 0x04, 0x04, 0x04, 0x05, 0x05, 0x05, 0x05,
					0x06, 0x06, 0x06, 0x06, 0x06, 0x07, 0x07, 0x07,
					0x07, 0x08, 0x08, 0x08, 0x08, 0x09, 0x09, 0x09,
					0x09, 0x0A, 0x0A, 0x0A, 0x0A, 0x0B, 0x0B, 0x0B,
					0x0B, 0x0B, 0x0B, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C,
					0x0C, 0x0D, 0x80 }; 

// this is used to pass the attenuation to the pcjr chip
// bits relating to the channel number.. (R0, R1, R2)
// bit 7 is also set.. this is "command byte!!"
u16 register_address[CHAN_MAX] = { 0x9F, 0xBF, 0xDF, 0xFF };

CHANNEL channel[CHAN_MAX];
u8 channels_left = 0;
u16 chan_cur = 0;	// BX

u8 snd_buff[SND_BUFF_SIZE];
int snd_off;

u8 nagi_sound_disable = 0;

void sound_driver_init()
{
	SDL_AudioSpec *wanted;
	
	if (nagi_sound_disable != 1)
	{
		wanted = (SDL_AudioSpec *)a_malloc(sizeof(SDL_AudioSpec));
		
		// turn on sdl driver
		wanted->freq = CARD_FREQ;
		wanted->format = AUDIO_S16LSB;
		wanted->channels = 1;	// mono
		wanted->samples =  512;  //Good low-latency value for callback 
		wanted->callback = sound_fill_buff;
		wanted->userdata = NULL;
	
		// Open the audio device, forcing the desired format 
		if ( SDL_OpenAudio(wanted, NULL) < 0 )
		{
			printf("Couldn't open audio: %s\n", SDL_GetError());
			nagi_sound_disable = 1;
			a_free(wanted);
			return;
		}
		
		a_free(wanted);
		
		// initialise samples
		sample_init();
	}
}

void sound_driver_denit()
{
	if (nagi_sound_disable != 1)
	{
		// turn off sdl driver
		printf("Waiting for sound thread to die...");
		sound_stop_sdl();
		SDL_UnlockAudio();
		SDL_CloseAudio();
		// denitialise samples
		sample_denit();
		printf("done.\n");
	}
}

int my_cnt = 0;
// initialise.. only called once per sound
void sound_new(SOUND *snd)
{	u8 i;
	my_cnt = 0;
	//printf("new sound!\n");

	if (nagi_sound_disable != 1)
	{
		channel[0].sample_num = 0;
		channel[1].sample_num = 0;
		channel[2].sample_num = 0;
		channel[3].sample_num = 1;
		
		for (i=0; i<CHAN_MAX ; i++)
		{
			channel[i].data = snd->channel[i];
			// set to 1 because it gets decremented  and when it's 0 it loads up new data
			channel[i].duration = 1;
			channel[i].dissolve = dissolve_data;
			channel[i].word1796 = 0xFFFF;
			channel[i].toggle = 0xFFFF;
			channel[i].freq_count = 0;
			channel[i].freq_prev = 0;
			channel[i].sample_cur = sample[channel[i].sample_num]->data;
		}
		
		// we're assuming 4 channel tandy/pcjr here anyways
		channels_left = CHAN_MAX;	// channels
		
		snd_off = 0;
		sound_playing = 1;
		
		sound_vector();
		
		SDL_PauseAudio(0);
	}
	else
	{
		flag_set(sound_flag);
	}
}


u32 tickz = 0;
 
void sound_fill_buff(void *udata, Uint8 *stream, int len)
{
	int cur=0;
	
	(void) udata;
	while (cur < len)
	{
		if (snd_off >= SND_BUFF_SIZE)
		{
			sound_vector();
			snd_off = 0;
		}
		stream[cur] =  snd_buff[snd_off];
		stream[cur+1] =  snd_buff[snd_off+1];
		snd_off += 2;
		cur += 2;
	}

}



// this would be some sorta callback I guess
// this is called at 60Hz man... remember this
void sound_vector()
{
	u8 *data;	// pointer for sound data
	u16 duration;
	
	if (sound_playing != 0)
	{

		if ( flag_test(0x9) == 0) 
			sound_stop_sdl();
		else
		{
			chan_cur = 0;
			
			while (chan_cur < CHAN_MAX)
			{
				if (channel[chan_cur].toggle == 0) 
				{
					chan_cur++;
					continue;
				}
				if (channel[chan_cur].duration>0)
					channel[chan_cur].duration--;
				if (channel[chan_cur].duration == 0)
				{
					if (chan_cur != 3)	// != noise??
						channel[chan_cur].word1796 = 0;
				//	else
					//	printf("**");
					// read the next note from the data
			
					data = channel[chan_cur].data;
					duration = load_le_16(data);	// duration
					data +=2;
					if ( duration == 0xFFFF) 
					{
						channels_left --;
						channel[chan_cur].toggle = 0;
						channel[chan_cur].attenuation = 0x0F;	// silent
						channel[chan_cur].attenuation_copy = 0;	// dunno really
					}
					else
					{
						// read and send the data to the sound device
						channel[chan_cur].duration = duration;
						if (chan_cur != (CHAN_MAX-1))
							channel[chan_cur].freq_count = (((u16)data[0] & 0x3F) << 4) + (data[1] & 0x0F);
						else
						{
							channel[chan_cur].freq_count = data[1] & 0x03;
							switch (channel[chan_cur].freq_count)
							{
								case 0:
									channel[chan_cur].freq_count = 512;
									break;
								case 1:
									channel[chan_cur].freq_count = 1024;
									break;
								case 2:
									channel[chan_cur].freq_count = 2048;
									break;
								default:
									channel[chan_cur].freq_count = 0 ;
							}
						}
						data +=2;
						channel[chan_cur].attenuation = *(data++) & 0xF;	// attenuation (volume)
						channel[chan_cur].data = data;
						//if (channel[chan_cur].freq_count != 0)
						//if ((111860 / channel[chan_cur].freq_count) < 50)
						//	channel[chan_cur].attenuation = 0xF;
					}
				}
				#warning fix the volume some day
				//sound_volume();
				chan_cur++;
			}	
			
			if (channels_left == 0)
				sound_stop_sdl();
		}
		mix_new(snd_buff, &channel[0], SND_BUFF_SIZE);
		mix_add(snd_buff, &channel[1], SND_BUFF_SIZE);
		mix_add(snd_buff, &channel[2], SND_BUFF_SIZE);
		//if (channel[3].toggle != 0)
			//printf("%d - dur=%d frqc=%d att=%d\n", my_cnt++, channel[3].duration, channel[3].freq_count,channel[3].attenuation);
		
	}

}


void sound_stop_sdl()
{
	if (nagi_sound_disable != 1)
	{	
		sound_playing = 0;
		SDL_PauseAudio(1);
		
		flag_set(sound_flag);
	}
}


/*
void sound_volume()
{
	if (  (computer_type = 0) && (channel[bx].attenuation = 0x0F)  )
	{
		al  = port(0x61);
		al = al & 0xFC;
		port(0x61) = al;
	}
	al = channel[bx].attenuation;
	if ( al != 0x0F)
	{
		di = channel[bx].word1796;
		if ( di != 0xFFFF) 
		{
			ah = *(channel[bx].dissolve + di);
			if ( ah == 0x80)	// if at end of list
			{
				channel[bx].word1796 = 0xFFFF;
				channel[bx].attenuation = channel[bx].attenuation_copy;
				al =channel[bx].attenuation;
			}
			else
			{
				channel[bx].word1796 ++;
				al += ah;
				if ( sign == 1)
					al = 0;
				if ( al > 0x0F)
					al = 0x0F;
				channel[bx].attenuation_copy = al;
				al = (al & 0x0F) + state.var[V23_SNDVOL];
				if ( al > 0x0F)
					al = 0x0F;
			}
		}
		if ( computer_type == 2) && ( al < 8)
			al += 2;
	}
	// al is the attenuation (1-f)  byte17f2 is the channel num
	port[0xC0] = al | (0xF0 & register_address[bx]);
}*/
