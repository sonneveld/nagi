/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <math.h>

/* OTHER headers	---	---	---	---	---	---	--- */
#include "sound_base.h"
#include "sound_gen.h"
#include "tone.h"
#include "tone_pcm.h"
#include "pcm_out.h"

#include "../sys/mem_wrap.h"
#include "../list.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */

/* Formulas for noise generator */
/* bit0 = output */

/* noise feedback for white noise mode */
#define FB_WNOISE 0x12000	/* bit15.d(16bits) = bit0(out) ^ bit2 */
/*#define FB_WNOISE 0x14000*/	/* bit15.d(16bits) = bit0(out) ^ bit1 */
/*#define FB_WNOISE 0x28000*/	/* bit16.d(17bits) = bit0(out) ^ bit2 (same to AY-3-8910) */
/*#define FB_WNOISE 0x50000*/	/* bit17.d(18bits) = bit0(out) ^ bit2 */

/* noise feedback for periodic noise mode */
/* it is correct maybe (it was in the Megadrive sound manual) */
/*#define FB_PNOISE 0x10000*/	/* 16bit rorate */
#define FB_PNOISE 0x08000

/* noise generator start preset (for periodic noise) */
#define NG_PRESET 0x0f35


#define M_PI 3.14159265358979323846264338327
// 
#define WAVE_HEIGHT  (0x7FFF)

struct sample_struct
{
	s16 *data;
	int samples;
	int freq;
};typedef struct sample_struct SAMPLE;

s16 vol_table[16];

struct tone_chan_struct
{
	int handle;	// for removing chan
	int agi_ch;	// for calling the agi soundgen callback
	
	int note_count; // length of tone.. duration
	
	int freq_count;
	int freq_count_prev;
	int atten;  // volume
	
	int gen_type;
	int gen_type_prev;
	
	union
	{
		struct 
		{
			SAMPLE *samp;
			int count;	// for scaling
			int scale;	// ""      ""
			u16 *samp_cur; // current sample point
		} s;
		
		struct
		{
			int count;
			int scale;
			int sign;
			unsigned int RNG;		/* noise generator      */
			int NoiseFB;		/* noise feedback mask */
		} n;
	};
	
};
typedef struct tone_chan_struct TONECHAN;

void tone_pcm_callback(void *tpcm, u8 *stream, int len);
void sample_free(SAMPLE *sample);
SAMPLE *sample_sine_new(int freq, int samp_rate);


/* VARIABLES	---	---	---	---	---	---	--- */

SAMPLE *sample_sine = 0;
SAMPLE *sample_triangle = 0;
SAMPLE *sample_square = 0;
SAMPLE *sample_noise = 0;

LIST *list_ch =0;

/* CODE	---	---	---	---	---	---	---	--- */

void vol_table_init()
{
	double value;
	int i;

	value = 0x7FFF;
	
	for (i=0; i<=16; i++)
	{
		vol_table[i] = (s16)value;
		// 2dB = 20*log(a/b)
		// 10^(2/20)*b = a;
		value /= 1.258925411794;
	}
	
	vol_table[0x0F] = 0;
}


//drv init
void tone_pcm_drv_init(void *drv)
{
	// set the pointers for the tone library
	TONE_DRIVER *tdrv;
	
	tdrv = (TONE_DRIVER *)drv;

	tdrv->ptr_init = tone_pcm_init;
	tdrv->ptr_shutdown = tone_pcm_shutdown;
	tdrv->ptr_open = tone_pcm_open;
	tdrv->ptr_close = tone_pcm_close;
	tdrv->ptr_state_set = tone_pcm_state_set;
	tdrv->ptr_state_get = tone_pcm_state_get;
	tdrv->ptr_lock = tone_pcm_lock;
	tdrv->ptr_unlock = tone_pcm_unlock;
}

//init
int tone_pcm_init(void)
{
	// init a pcm device
	if (pcm_out_init(44100, AUDIO_S16LSB))
		return -1;
	
	vol_table_init();
	
	return 0;
}

//shutdown
void tone_pcm_shutdown(void)
{
	tone_pcm_state_set(0);
	
	// all channels must be closed
	if (list_ch)
		list_free(list_ch);
	list_ch = 0;
	
	// close all samples
	if (sample_sine)
	{
		sample_free(sample_sine);
		sample_sine = 0;
	}
	
	// shutdown pcm out
	pcm_out_shutdown();
}

// open
int tone_pcm_open(int ch)
{
	TONECHAN *new_ch;
	
	if (list_ch==NULL)
		list_ch = list_new(sizeof(TONECHAN));
	
	new_ch = list_add(list_ch);
	new_ch->atten = 0xF;	// silence
	new_ch->agi_ch = ch;
	
	new_ch->freq_count = 1000;
	new_ch->freq_count_prev = -1;
	new_ch->gen_type = GEN_TONE;
	new_ch->gen_type_prev = -1;
	
	new_ch->note_count = 0;
	
	// init a sample for that specific type if it doesn't exist.

	if (sample_sine==NULL)
		sample_sine = sample_sine_new(50, 44100);
	
	new_ch->handle = pcm_out_open(tone_pcm_callback, (void *)new_ch);
	
	return new_ch->handle;
}

void tone_pcm_close(int handle)
{
	TONECHAN *ch;
	
	if (list_ch!=NULL)
	{
		// find the one with a handle
		ch = list_element_head(list_ch);
		while ((ch!=NULL) && (ch->handle != handle))
			ch=node_next(ch);
		
		// kill it
		if (ch!=NULL)
		{
			pcm_out_close(ch->handle);
			list_remove(list_ch, ch);
		}
	}
	
}

void tone_pcm_state_set(int tone_state)
{
	pcm_out_state_set(tone_state);
}

int tone_pcm_state_get(void)
{
	return pcm_out_state_get();
}

void tone_pcm_lock(void)
{
	pcm_out_lock();
}

void tone_pcm_unlock(void)
{
	pcm_out_unlock();
}
int sample_fill(TONECHAN *t, s16 *buf, int len);
#define FREQ_DIV 111844
#define MULT FREQ_DIV

// fill buff
void tone_pcm_callback(void *userdata, u8 *stream, int len)
{
	TONECHAN *tpcm;
	s16 *stream_cur;
	TONE new_tone;
	int fill_size;
	
	tpcm = (TONECHAN*)userdata;
	stream_cur = (s16 *)stream;

	len /= 2;
	
	while (len > 0)
	{
		if (tpcm->note_count <= 0)
		{
			// get new tone data	
			new_tone.freq_count = 0;
			new_tone.atten = 0xF;
			new_tone.type = GEN_TONE;
			sndgen_callback(tpcm->agi_ch, &new_tone);
	
			tpcm->atten = new_tone.atten;
			tpcm->freq_count = new_tone.freq_count;
			tpcm->gen_type = new_tone.type;
			
			// setup counters 'n stuff
			// 44100 samples per sec.. tone changes 60 times per sec
			tpcm->note_count = 44100 / 60;
		}
		
		// find which is smaller.. the buffer or the 
		fill_size = (tpcm->note_count <= len) ? tpcm->note_count: len;
		
		switch (tpcm->gen_type)
		{
			case GEN_TONE:
				fill_size = square_fill(tpcm, stream_cur, fill_size);
				break;
			case GEN_PERIOD:
			case GEN_WHITE:
				fill_size = noise_fill(tpcm, stream_cur, fill_size);
				break;
			default:
				// fill with whitespace
				memset(stream_cur, 0, fill_size * sizeof(s16));
				break;
		}
		
		tpcm->note_count -= fill_size;
		stream_cur += fill_size;
		len -= fill_size;
	}
}


int sample_fill(TONECHAN *t, s16 *buf, int len)
{
	s16 *samp_cur;
	int count;
	
	if (t->gen_type != t->gen_type_prev)
	{
		// set the samples
		t->s.samp = sample_sine;
		t->s.samp_cur = sample_sine->data;
		t->freq_count_prev = -1;
		t->gen_type_prev = t->gen_type;
	}
	
	if (t->freq_count != t->freq_count_prev)
	{
		//t->scale = (int)( (double)t->samp->freq*t->freq_count/FREQ_DIV * MULT + 0.5);
		t->s.scale = t->s.samp->freq * t->freq_count;
		t->s.count = t->s.scale;
		t->freq_count_prev = t->freq_count;	
	}

	samp_cur = t->s.samp_cur;
	count = len;
	
	while (count > 0)
	{		
		// write nothing
		if ((t->freq_count == 0)||(t->atten==0xF))
		{
			#warning use memset and skip past huge chunks
			*(buf++) = 0;
			count--;
		}
		else // write a sample to output device
		{
			*(buf++) = *samp_cur;
			count--;
			
			// get next sample
			t->s.count -= MULT;
			while (t->s.count <= 0)
			{
				samp_cur++;
				t->s.count += t->s.scale;
			}
			// check sample bounds
			while ((samp_cur - t->s.samp->data) >= t->s.samp->samples )
				samp_cur -= t->s.samp->samples;
		}
	}
	
	t->s.samp_cur = samp_cur;
	return len;
}



int square_fill(TONECHAN *t, s16 *buf, int len)
{
	int count;
	
	if (t->gen_type != t->gen_type_prev)
	{
		// make sure the freq_count is checked
		t->freq_count_prev = -1;
		t->n.sign = 1;
		t->gen_type_prev = t->gen_type;
	}
	
	if (t->freq_count != t->freq_count_prev)
	{
		//t->scale = (int)( (double)t->samp->freq*t->freq_count/FREQ_DIV * MULT + 0.5);
		t->n.scale = (44100/2) * t->freq_count;
		t->n.count = t->n.scale;
		t->freq_count_prev = t->freq_count;	
	}

	count = len;
	
	while (count > 0)
	{		
		// write nothing
		if ((t->freq_count == 0)||(t->atten==0xF))
		{
			#warning use memset and skip past huge chunks
			*(buf++) = 0;
			count--;
		}
		else // write a sample to output device
		{
			*(buf++) = t->n.sign ? vol_table[t->atten] : -vol_table[t->atten];
			count--;
			
			// get next sample
			t->n.count -= MULT;
			while (t->n.count <= 0)
			{
				t->n.sign ^= 1;
				t->n.count += t->n.scale;
			}
		}
	}
	
	return len;}

int noise_fill(TONECHAN *t, s16 *buf, int len)
{
	int count;
	
	if (t->gen_type != t->gen_type_prev)
	{
		// make sure the freq_count is checked
		t->freq_count_prev = -1;
		//t->n.sign = 1;
		t->gen_type_prev = t->gen_type;
	}
	
	if (t->freq_count != t->freq_count_prev)
	{
		//t->scale = (int)( (double)t->samp->freq*t->freq_count/FREQ_DIV * MULT + 0.5);
		t->n.scale = (44100/2) * t->freq_count;
		t->n.count = t->n.scale;
		t->freq_count_prev = t->freq_count;	
		
		t->n.NoiseFB = (t->gen_type==GEN_WHITE) ? FB_WNOISE : FB_PNOISE;
		/* reset noise shifter */
		t->n.RNG = NG_PRESET;
		t->n.sign = t->n.RNG & 1;
	}

	count = len;
	
	while (count > 0)
	{		
		// write nothing
		if ((t->freq_count == 0)||(t->atten==0xF))
		{
			#warning use memset and skip past huge chunks
			*(buf++) = 0;
			count--;
		}
		else // write a sample to output device
		{
			*(buf++) = t->n.sign ? vol_table[t->atten] : -vol_table[t->atten];
			count--;
			
			// get next sample
			t->n.count -= MULT;
			while (t->n.count <= 0)
			{
				if (t->n.RNG & 1)
					t->n.RNG ^= t->n.NoiseFB;
				t->n.RNG >>= 1;
				t->n.sign = t->n.RNG & 1;
				t->n.count += t->n.scale;
			}
		}
	}
	
	return len;
}

// ------------------------------------------------------------------------------------------


// in resampling.. if we have to skip through the wave file several times to get to the sample.. don't bother,
// mod it a few times to scale it down.




// samp rate/.. samples per second
SAMPLE *sample_sine_new(int freq, int samp_rate)
{
	int i;
	double mult;
	SAMPLE *s;
	s16 *wave;
	int wave_len;
	
	s = (SAMPLE *)a_malloc(sizeof(SAMPLE) );

	wave_len = samp_rate / freq;
	wave = a_malloc(sizeof(s16) * wave_len);	
	mult =  (2.0*M_PI) / (wave_len);
	
	for (i=0; i<wave_len; i++)
		wave[i] = (s16)(sin(i * mult)*WAVE_HEIGHT );
		
	s->freq = freq;
	s->data = wave;
	s->samples = wave_len;
	
	return s;
	
}

void sample_free(SAMPLE *sample)
{
	assert(sample);
	assert(sample->data);

	a_free(sample->data);
	a_free(sample);
}