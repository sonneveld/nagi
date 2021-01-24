/* FUNCTION list 	---	---	---	---	---	---	---

*/


//~ RaDIaT1oN: remove unnamed unions members
/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>


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

//#define WAVE_HEIGHT  (0x7FFF)


static s16 vol_table[16];

struct tone_chan_struct
{
	int handle;	// for removing chan
	int agi_ch;	// for calling the agi soundgen callback
	int avail;
	
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
			int count;
			int scale;
			int sign;
			unsigned int noise_state;		/* noise generator      */
			int feedback;		/* noise feedback mask */
		} n;
		
	};
	
};
typedef struct tone_chan_struct TONECHAN;

static int tone_pcm_callback(void *tpcm, u8 *stream, int len);
static int noise_fill(TONECHAN *t, s16 *buf, int len);
static int square_fill(TONECHAN *t, s16 *buf, int len);
static void tone_pcm_state_set(int tone_state);
static int tone_pcm_state_get(void);
static void tone_pcm_lock(void);
static void tone_pcm_unlock(void);
static void vol_table_init(void);
static void tone_pcm_shutdown(void);
static int tone_pcm_open(int ch);
static void tone_pcm_close(int handle);

/* VARIABLES	---	---	---	---	---	---	--- */

static LIST *list_ch = 0;

/* CODE	---	---	---	---	---	---	---	--- */

static void vol_table_init()
{
	double value;
	int i;

	value = c_snd_volume;
	
	for (i=0; i<0xF; i++)
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
static void tone_pcm_shutdown(void)
{
	tone_pcm_state_set(0);
	
	// all channels must be closed
	if (list_ch)
		list_free(list_ch);
	list_ch = 0;
	
	// shutdown pcm out
	pcm_out_shutdown();
}

// open
// return 0 on error
static int tone_pcm_open(int agi_ch)
{
	TONECHAN ch;
	TONECHAN *new_ch;
	
	if (list_ch==NULL)
		list_ch = list_new(sizeof(TONECHAN));
	
	ch.atten = 0xF;	// silence
	ch.agi_ch = agi_ch;
	ch.freq_count = 250;
	ch.freq_count_prev = -1;
	ch.gen_type = GEN_TONE;
	ch.gen_type_prev = -1;
	ch.note_count = 0;
	ch.avail = 1;
	
	new_ch = list_add(list_ch);
	memcpy(new_ch, &ch, sizeof(TONECHAN) );
	
	new_ch->handle = pcm_out_open(tone_pcm_callback, (void *)new_ch);
	
	if (new_ch->handle == 0)
	{
		list_remove(list_ch, new_ch);
		return 0;
	}
	
	return new_ch->handle;
}

static void tone_pcm_close(int handle)
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

static void tone_pcm_state_set(int tone_state)
{
	pcm_out_state_set(tone_state);
}

static int tone_pcm_state_get(void)
{
	return pcm_out_state_get();
}

static void tone_pcm_lock(void)
{
	pcm_out_lock();
}

static void tone_pcm_unlock(void)
{
	pcm_out_unlock();
}


#define FREQ_DIV 111844
#define MULT FREQ_DIV

// fill buff
// return -1 if channel is complete.
static int tone_pcm_callback(void *userdata, u8 *stream, int len)
{
	TONECHAN *tpcm;
	s16 *stream_cur;
	TONE new_tone;
	int fill_size;
	int ret_val;

	tpcm = (TONECHAN*)userdata;
	stream_cur = (s16 *)stream;

	len /= 2;
	ret_val = 0;    // assume channel is okay and still going
	
	while (len > 0)
	{
		if (tpcm->note_count <= 0)
		{
			// get new tone data	
			new_tone.freq_count = 0;
			new_tone.atten = 0xF;
			new_tone.type = GEN_TONE;
			if ( (tpcm->avail) &&
				(sndgen_callback(tpcm->agi_ch, &new_tone) == 0))
			{
				tpcm->atten = new_tone.atten;
				tpcm->freq_count = new_tone.freq_count;
				tpcm->gen_type = new_tone.type;
				
				// setup counters 'n stuff
				// 44100 samples per sec.. tone changes 60 times per sec
				tpcm->note_count = 44100 / 60;
			}
			else
			{
				// if it doesn't return an
				tpcm->gen_type = GEN_SILENCE;
				tpcm->note_count = len;
				tpcm->avail = 0;
				ret_val = -1;
			}
		}
		
		// write nothing
		if ((tpcm->freq_count == 0)||(tpcm->atten==0xF))
		{
			tpcm->gen_type = GEN_SILENCE;
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
			case GEN_SILENCE:
			default:
				// fill with whitespace
				memset(stream_cur, 0, fill_size * sizeof(s16));
				break;
		}
		
		tpcm->note_count -= fill_size;
		stream_cur += fill_size;
		len -= fill_size;
	}
	
	return ret_val;
}



static int square_fill(TONECHAN *t, s16 *buf, int len)
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
	
	return len;
}

static int noise_fill(TONECHAN *t, s16 *buf, int len)
{
	int count;
	
	if (t->gen_type != t->gen_type_prev)
	{
		// make sure the freq_count is checked
		t->freq_count_prev = -1;
		t->gen_type_prev = t->gen_type;
	}
	
	if (t->freq_count != t->freq_count_prev)
	{
		//t->scale = (int)( (double)t->samp->freq*t->freq_count/FREQ_DIV * MULT + 0.5);
		t->n.scale = (44100/2) * t->freq_count;
		t->n.count = t->n.scale;
		t->freq_count_prev = t->freq_count;	
		
		t->n.feedback = (t->gen_type==GEN_WHITE) ? FB_WNOISE : FB_PNOISE;
		/* reset noise shifter */
		t->n.noise_state = NG_PRESET;
		t->n.sign = t->n.noise_state & 1;
	}

	count = len;
	
	while (count > 0)
	{		
		*(buf++) = t->n.sign ? vol_table[t->atten] : -vol_table[t->atten];
		count--;
		
		// get next sample
		t->n.count -= MULT;
		while (t->n.count <= 0)
		{
			if (t->n.noise_state & 1)
				t->n.noise_state ^= t->n.feedback;
			t->n.noise_state >>= 1;
			t->n.sign = t->n.noise_state & 1;
			t->n.count += t->n.scale;
		}
	}
	
	return len;
}
