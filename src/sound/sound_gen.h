#ifndef sound_gen_h_file
#define sound_gen_h_file

//~ RaDIaT1oN: fix warnings, add sndgen_kill

#include "sound_base.h"

/* STRUCTURES	---	---	---	---	---	---	--- */

struct tone_struct
{
	int freq_count;
	int atten;
	int type;
};
typedef struct tone_struct TONE;

/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */

extern void sndgen_init(void);
extern void sndgen_shutdown(void);
extern void sndgen_play(SOUND *snd);
extern void sndgen_stop(void);
extern int sndgen_callback(int ch, TONE *tone);
extern void sndgen_poll(void);
extern void sndgen_kill_thread(void);

#endif