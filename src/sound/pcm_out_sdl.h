#ifndef NAGI_SOUND_PCM_OUT_SDL_H
#define NAGI_SOUND_PCM_OUT_SDL_H

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
extern void pcm_out_sdl_drv_init(void *drv);
extern int pcm_out_sdl_init(int freq, int format);
extern void pcm_out_sdl_shutdown(void);
extern void pcm_out_sdl_avail(void);
extern int pcm_out_sdl_open( int (*callback)(void *userdata, Uint8 *stream, int len), void *userdata);
extern void pcm_out_sdl_close(int handle);
extern void pcm_out_sdl_state_set(int );
extern int pcm_out_sdl_state_get(void);
extern void pcm_out_sdl_lock(void);
extern void pcm_out_sdl_unlock(void);
extern void sdl_callback(void *userdata, u8 *stream, int len);

#endif /* NAGI_SOUND_PCM_OUT_SDL_H */
