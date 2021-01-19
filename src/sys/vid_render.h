#ifndef NAGI_SYS_VID_RENDER_H
#define NAGI_SYS_VID_RENDER_H

/* STRUCTURES	---	---	---	---	---	---	--- */

	
/* VARIABLES	---	---	---	---	---	---	--- */
extern RDRIVER *rend_drv;
extern u8 *rend_buf;
extern int rend_buf_size;

/* FUNCTIONS	---	---	---	---	---	---	--- */
extern void render_init(void);
extern void render_shutdown(void);
extern void render_drv_rotate(void);
extern void render_update(int x, int y, int width, int height);
extern void render_rect(int x, int y, int width, int height, u8 colour);
extern void render_colour(u8 col, COLOUR *col_dith);
extern void render_view_dither(u8 *view_data);

#endif /* NAGI_SYS_VID_RENDER_H */
