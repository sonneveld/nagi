#ifndef gfx_h_file
#define gfx_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
#define PICBUFF_WIDTH (160)
#define PICBUFF_HEIGHT (168)

#define PAL_16 0
#define PAL_TEXT 1
#define PAL_CGA0 2
#define PAL_CGA1 3
// for herc
#define PAL_BW 4
#define PAL_CUSTOM 100

extern VSURFACE *gfx_surface;

extern u8 gfx_paltype;
extern PCOLOUR *gfx_pal;	// set from config
extern u8 gfx_palsize;

//extern u8 gfx_scale;	// set from config

extern SIZE gfx_size;	// from render size * scale
extern u8 *gfx_picbuff;	// created in gfx_init();
extern u8 gfx_picbuffrow;	// set after fonts are init'd
extern int gfx_picbuffrotate; 

/* FUNCTIONS	---	---	---	---	---	---	--- */

extern void gfx_init(void);
extern void gfx_shutdown(void);
extern void gfx_update(u16 rect_x, u16 rect_y, u16 rect_w, u16 rect_h);
extern void gfx_shake(u8 count);
extern void gfx_msgbox(int x, int y, int w, int h, u8 bg, u8 line);
extern void gfx_palette_update(void);
extern void gfx_clear(void);
extern void gfx_picbuff_update(void);
extern void gfx_reinit(void);

#endif
