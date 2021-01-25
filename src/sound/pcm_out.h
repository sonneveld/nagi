#ifndef NAGI_SOUND_PCM_OUT_H
#define NAGI_SOUND_PCM_OUT_H

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */



#define PCM_OUT_NONE 0
#define PCM_OUT_SDL 1
#define PCM_OUT_SPEAKER 2

struct pcm_out_driver_struct
{
	u8 type;
	
	int (*ptr_init)(int freq, int format);
	void (*ptr_shutdown)(void);
	void (*ptr_avail)(void);
	int (*ptr_open)( int (*callback)(void *userdata, Uint8 *stream, int len), void *userdata);
	void (*ptr_close)(int handle);
	void (*ptr_state_set)(int );
	int (*ptr_state_get)(void);
	void (*ptr_lock)(void);
	void (*ptr_unlock)(void);
};
typedef struct pcm_out_driver_struct PCM_OUT_DRIVER;
	
extern int pcm_out_init(int freq, int format);
extern void pcm_out_shutdown(void);
extern void pcm_out_avail(void);
extern int pcm_out_open( int (*callback)(void *userdata, Uint8 *stream, int len), void *userdata);
extern void pcm_out_close(int handle);
extern void pcm_out_state_set(int);
extern int pcm_out_state_get(void);
extern void pcm_out_lock(void);
extern void pcm_out_unlock(void);

#endif /* NAGI_SOUND_PCM_OUT_H */
