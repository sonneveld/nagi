struct colour_struct
{
	u8 odd;	// al
	u8 even;	// ah
};
typedef struct colour_struct COLOUR;
 

struct render_driver_struct
{
	u8 type;
	u8 agi_type;
	u8 pal_type;
	u16 w;
	u16 h;
	
	u8 scale_x;  // scale from agi coordinates to render coordinates
	u8 scale_y;  // ""             ""                            ""              ""
	
	void (*func_update)(int x, int y, int width, int height);
	void (*func_rect)(int x, int y, int width, int height, u8 colour);
	void (*func_colour)(u8 col, COLOUR *col_dith);
	void (*func_view_dither)(u8 *view_data);
};
typedef struct render_driver_struct RDRIVER;

#define R_NONE	0
#define R_EGA	1
#define R_CGA0	2
#define R_CGA1	3

// unsupported
#define R_HERC	4
#define R_PALM	5



struct vsurface_struct
{
	u8 *pixels;
	SIZE size;
	u32 line_size;
	u32 pixel_size;
	
	void *system_surface;
};
typedef struct vsurface_struct VSURFACE;
	
struct pcolour_struct
{
	u8 r;
	u8 g;
	u8 b;
};
typedef struct pcolour_struct PCOLOUR;
	
struct video_driver_struct
{
	u8 type;
	
	void (*ptr_shutdown)(void);
	VSURFACE *(*ptr_display)(VSURFACE *vsurface, SIZE *screen_size, int fullscreen_state);
	void (*ptr_free)(VSURFACE *vsurface);
	void (*ptr_lock)(VSURFACE *vsurface);
	void (*ptr_unlock)(VSURFACE *vsurface);
	void (*ptr_update)(VSURFACE *vsurface, POS *pos, SIZE *size);
	void (*ptr_palette_set)(VSURFACE *vsurface, PCOLOUR *palette, u8 num);
	void (*ptr_fill)(VSURFACE *vsurface, POS *pos, SIZE *size, u32 colour);
	void (*ptr_shake)(VSURFACE *vsurface, int count);
};
typedef struct video_driver_struct VDRIVER;

struct font_struct
{
	u16 width;
	u16 height;
	u16 size;	// number of chars
	u8 *data;
	u32 align; // size of each char in bytes
	u32 line_size;  // size of each line in bytes
};
typedef struct font_struct FONT;

extern void vid_init(void);
extern void vid_shutdown(void);
extern VSURFACE *vid_display(VSURFACE *vsurface, SIZE *screen_size, int fullscreen_state);
extern void vid_free(VSURFACE *vsurface);
extern void vid_lock(VSURFACE *vsurface);
extern void vid_unlock(VSURFACE *vsurface);
extern void vid_update(VSURFACE *vsurface, POS *pos, SIZE *size);
extern void vid_palette_set(VSURFACE *vsurface, PCOLOUR *palette, u8 num);
extern void vid_fill(VSURFACE *vsurface, POS *pos, SIZE *size, u32 colour);
extern void vid_shake(VSURFACE *vsurface, int count);
