#ifndef NAGI_SYS_SDL_VID_H
#define NAGI_SYS_SDL_VID_H

#include "drv_video.h"

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */

extern void vid_init(void);
extern void vid_shutdown(void);
extern void vid_display(AGISIZE *screen_size, int fullscreen_state);
extern void vid_free(void);
extern void vid_lock(void);
extern void vid_unlock(void);
extern void vid_update(POS *pos, AGISIZE *size);
extern void vid_notify_window_size_changed(Uint32 windowID);
extern void vid_palette_set(PCOLOUR *palette, u8 num);
extern void vid_fill(POS *pos, AGISIZE *size, u32 colour);
extern void vid_shake(int count);

extern void *vid_getbuf(void);
extern int vid_getlinesize(void);
extern SDL_Window* vid_get_main_window(void);

#endif /* NAGI_SYS_SDL_VID_H */
