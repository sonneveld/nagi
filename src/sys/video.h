#ifndef video_h_file
#define video_h_file


/* STRUCTURES	---	---	---	---	---	---	--- */
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
	u16 w;
	u16 h;
	
	u8 scale_x;  // scale from agi coordinates to render coordinates
	u8 scale_y;  // ""             ""                            ""              ""
	
	void (*func_update)(u16 x, u16 y, u16 width, u16 height);
	void (*func_rect)(u16 x, u16 y, u16 width, u16 height, u8 colour);
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

struct render_state_struct
{
	RDRIVER *drv;
	
	u8 *buf;
	int buf_size;
};
typedef struct render_state_struct RSTATE;


	
struct video_driver_struct
{
	u8 type;
	
	void (*init)(RSTATE *rend_state);
	void (*shutdown)(void);
	void (*update)(u16 x, u16 y, u16 width, u16 height);
	void (*clear)(void);
	void (*shake)(u8 count);
	
	void (*t_pos_get)(POS *pos);
	void (*t_pos_set)(POS *pos);
	void (*t_char_attrib)(u8 colour, u16 flags);
	void (*t_char_put)(u8 ch);
	void (*t_scroll)(POS *pos1, POS *pos2, u16 scroll, u8 attrib);
	void (*t_clear)(POS *pos1, POS *pos2, u8 attrib);
};
typedef struct video_driver_struct VDRIVER;

struct video_state_struct
{
	VDRIVER *drv;
	
	u8 *pal;
	u8 pal_size;
	
	int scale;
	int scale_fill;
	int fullscreen;
	
	u16 w;
	u16 h;
	
	u8 *pic_buf;
	u8 pic_buf_row;
	u8 text_mode;

};
typedef struct video_state_struct VSTATE;

extern VSTATE vstate;



// video types
#define VID_NONE	0
#define VID_SDL		1

// unsupported.. just ideas
#define VID_WINDIB	2
#define VID_DIRECTX	3
#define VID_OPENGL 	4
#define VID_VGA		5

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
extern void vid_update(u16 x, u16 y, u16 width, u16 height);
extern void vid_shake(u8 count);
extern void t_pos_get(POS *pos);
extern void t_pos_set(POS *pos);
extern void t_char_attrib(u8 colour, u16 flags);
extern void t_char_put(u8 ch);
extern void t_scroll(POS *pos1, POS *pos2, u16 scroll, u8 attrib);
extern void t_clear(POS *pos1, POS *pos2, u8 attrib);



/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */

#endif