#ifndef tone_pcm_h_file
#define tone_pcm_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */


extern void tone_pcm_drv_init(void *drv);
extern int tone_pcm_init(void);
extern void tone_pcm_shutdown(void);
extern int tone_pcm_open(int ch);
extern void tone_pcm_close(int handle);
extern void tone_pcm_state_set(int tone_state);
extern int tone_pcm_state_get(void);
extern void tone_pcm_lock(void);
extern void tone_pcm_unlock(void);

#endif
