#ifndef sound_gen_h_file
#define sound_gen_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */

struct tone_struct
{
	int freq_count;
	int atten;
	int type;
};
typedef struct tone_struct TONE;


struct sndgen_channel_struct
{
	u8 *data;
	u16 duration;
	u16 toggle;	// turned on (1)  but when the channel's data runs out, it's set to (0)
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
/* FUNCTIONS	---	---	---	---	---	---	--- */

extern void sndgen_init(void);
extern void sndgen_shutdown(void);
extern void sndgen_play(SOUND *snd);
extern void sndgen_stop(void);
extern void sndgen_callback(int ch, TONE *tone);

extern void sndgen_poll(void);

#endif