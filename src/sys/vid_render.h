#ifndef vid_render_h_file
#define vid_render_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */

	
/* VARIABLES	---	---	---	---	---	---	--- */
extern RSTATE rstate;

/* FUNCTIONS	---	---	---	---	---	---	--- */
extern void render_init(void);
extern void render_shutdown(void);
extern void render_update(u16 x, u16 y, u16 width, u16 height);
extern void render_rect(u16 x, u16 y, u16 width, u16 height, u8 colour);
extern void render_colour(u8 col, COLOUR *col_dith);
extern void render_view_dither(u8 *view_data);


#endif