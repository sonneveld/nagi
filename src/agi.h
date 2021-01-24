#ifndef NAGI_AGI_H
#define NAGI_AGI_H

// Important include files that just about every file uses
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "SDL.h"

//#define alloca __builtin_alloca

#ifdef _MSC_VER
//not #if defined(_WIN32) || defined(_WIN64) because we have strncasecmp in mingw
#define strncasecmp _strnicmp
#define strcasecmp _stricmp
#define strtok_r strtok_s
#endif

// ok no warning here

#ifndef NAGI_LIST_H_STRUCT
#define NAGI_LIST_H_STRUCT

/* STRUCTURES	---	---	---	---	---	---	--- */
struct node_struct
{
	struct node_struct *next;
	struct node_struct *prev;
	unsigned char contents[1];
};
typedef struct node_struct NODE;

struct list_struct
{
	// public to read
	NODE *head;
	NODE *tail;
	
	// private
	int contents_size;
};
typedef struct list_struct LIST;
typedef struct list_struct STACK;

#endif /* NAGI_LIST_H_STRUCT */



typedef uint8_t		u8;
typedef uint16_t	u16;
typedef uint32_t	u32;
typedef int8_t		s8;
typedef int16_t		s16;
typedef int32_t		s32;

#define V00_ROOM0		0
#define V01_OLDROOM		1
#define V02_BORDER		2
#define V03_SCORE		3
#define V04_OBJECT		4
#define V05_OBJBORDER	5
#define V06_DIRECTION	6
#define V07_MAXSCORE	7
#define V08_FREEMEM		8
#define V09_BADWORD		9
#define V10_DELAY		10
#define V11_SECONDS		11
#define V12_MINUTES		12
#define V13_HOURS		13
#define V14_DAYS		14
#define V15_JOYSENS		15
#define V16_EGOVIEWRES	16
#define V17_ERROR		17
#define V18_ERROR2		18
#define V19_KEYPRESSED	19
#define V20_COMPUTER	20
#define V21_WINDOWTIMER	21
#define V22_SNDTYPE		22
#define V23_SNDVOL		23
#define V24_INPUTLEN		24
#define V25_ITEM		25
#define V26_MONITORTYPE	26

#define F00_EGOWATER	0
#define F01_EGOINVIS		1
#define F02_PLAYERCMD	2
#define F03_EGOSIGNAL	3
#define F04_SAIDACCEPT	4
#define F05_NEWROOM		5
#define F06_RESTART		6
#define F07_SCRIPTBLOCK	7
#define F08_JOYSENS		8
#define F09_SOUND		9
#define F10_DEBUG		10
#define F11_NEWLOGIC0	11
#define F12_RESTORE		12
#define F13_STATSELECT	13
#define F14_MENU		14
#define F15_PRINTMODE	15
#define F16_RESTARTMODE   16

// VIEW OBJECTS FLAGS

// 0 - object has been drawn
#define O_DRAWN		0x1
// 1 - ignore blocks and condition lines
#define O_BLOCKIGNORE	0x2
// 2 - fixes priority.   agi cannot change it based on position
#define O_PRIFIXED		0x4
// 3 - ignore horizon
#define O_HORIZONIGNORE	0x8
// 4 - update every cycle
#define O_UPDATE		0x10
// 5 - the object cycles
#define O_CYCLE			0x20
// 6 - animated
#define O_ANIMATE		0x40
// 7 - resting on a block
#define O_BLOCK			0x80
// 8 - only allowed on water
#define O_WATER		0x100
// 9 - ignore other objects when determining contacts
#define O_OBJIGNORE		0x200
// 10	// set whenever a obj is repositioned
	// that way the interpeter doesn't check it's next movement for one cycle
#define O_REPOS			0x400
// 11 - only allowed on land
#define O_LAND			0x800
// 12 - does not update obj for one cycle
#define O_SKIPUPDATE		0x1000
// 13 - agi cannot set the loop depending on direction
#define O_LOOPFIXED		0x2000
// 14 - no movement.  if position is same as position in last cycle then
	// this flag is set.  follow/wander code can then create a new direction
	// (ie, if it hits a wall or something)
#define O_MOTIONLESS	0x4000
#define O_UNUSED		0x8000

// MOTION TYPES
#define MT_NORM	0
#define MT_WANDER	1
#define MT_FOLLOW	2
#define MT_MOVE	3
#define MT_EGO		4

// CYCLE TYPES
// normal cycle
#define CY_NORM	0
// stop at the end
#define CY_END		1
// reverse.. stop at the end
#define CY_REVEND	2
// reversed cycle
#define CY_REV		3

#define NAGI_VERSION "v2.07 beta"


struct position_struct
{
	int x;
	int y;
};
typedef struct position_struct POS;
	
struct text_position_struct
{
	int row;
	int col;
};
typedef struct text_position_struct TPOS;
	
struct size_struct
{
	int w;
	int h;
};
typedef struct size_struct AGISIZE;
	
struct rect_struct
{
	struct position_struct pos;
	struct size_struct size;
};
typedef struct rect_struct AGIRECT;

#define VAR_SIZE 256
// using individual bits
#define FLAG_SIZE 32
#define STRING_SIZE 40
#define STRING_LIST_SIZE 24
#define CONTROL_SIZE 50
// number of chars
#define ID_SIZE 20
// 00 .. 182
#define CMD_MAX 182
#define EVAL_MAX 19

struct cmap_struct
{
	u16 key;	// di[0]
	u16 num;	// di[2]	
};
typedef struct cmap_struct CMAP;
	
struct agi_state_struct
{
	u8 id[ID_SIZE+1];
	u8 var[VAR_SIZE];
	u8 flag[FLAG_SIZE];
	
	u32 ticks;
	u16 horizon;
	u16 word_12f;			// unused
	
	u16 block_x1;			// upper left
	u16 block_y1;
	u16 block_x2;			// lower right
	u16 block_y2;
	
	u16 ego_control_state;		// 1-player 0-computer control
	u16 pic_num;
	u16 block_state;			// a block is not set (0)
	u16 word_13f;			// unused
	u16 script_size;
	u16 script_count;			// number written in script
	CMAP control_map[CONTROL_SIZE];
	u8 string[STRING_LIST_SIZE][STRING_SIZE];
	
	u16 text_fg;
	u16 text_bg;
	u16 text_comb;
	
	u16 input_state;			// 1=accept input  0=prevent
	u16 input_pos;
	u16 cursor;
	u16 status_line_row;
	u16 status_state;		// 0 = off.. 1 = on
	u16 window_row_min;
	u16 window_row_max;
	u16 script_saved;		// temp of script_count
	u16 menu_state;	// menu state (on or off)
	u8 walk_mode;	// walk mode state
};
typedef struct agi_state_struct AGI_STATE;
	
extern AGI_STATE state;


enum protect_enum {P_NONE, LARRY, GR, KQ4, MH, MH2};
enum palette_enum {PALPC, PALAMIGA, PALVAR};
enum res_enum {RES_NONE=0, RES_V2=1, RES_V3_4=2, RES_V3=3, RES_V3_AMIGA=4};

enum dir_enum {DIR_NONE=0, DIR_SEP=1, DIR_COMB=2, DIR_AMIGA=3};

enum loop_enum {L_FOUR=1, L_ALL=2, L_FLAG=3};
enum mouse_enum {M_NONE=0, M_SIERRA_V2=1, M_SIERRA_V3=2, M_BRIAN=10, M_NICK=11};
enum system_enum {SYS_PC, SYS_APPLE, SYS_AMIGA, SYS_AGDS, SYS_TANDY};

/*
struct agi_standard_struct
{
	u8 *name;
	u8 game_id[ID_SIZE+1];	// different from the state game_id
				// this is what the 'exe' is set to.. what the v3dir
				// files are called
	u8 game_id_honour;

	int ver_major;
	int ver_minor;
	enum system_enum system;		// system type
	enum res_enum res_type;		// resource type (lzw or otherwise)
	
	int cmd_max;	
	int string_max;
	// if 1 then then object file gets decrypted after it's loaded
	u8 object_decrypt;
	enum protect_enum protection;	// remove copy protection
	enum palette_enum palette;		// palette to use
	enum loop_enum update_loop;	// loop settings for update.graf
	enum mouse_enum mouse;		// mouse type (brian or controller method)
};
typedef struct agi_standard_struct AGI_STANDARD;
*/
//extern AGI_STANDARD standard;



struct blit_struct
{
	// next and prev are *GUESSES* atm
	struct blit_struct *prev;	// 0-1
	struct blit_struct *next;	// 2-3
	struct view_struct *v;		// 4-5
	s16 x;			// 6-7
	s16 y;			// 8-9
	s16 x_size;			// A-B
	s16 y_size;			// C-D
	u8 *buffer;			// E-F
};

typedef struct blit_struct BLIT;

struct view_struct
{
	u8 step_time;		// 0
	u8 step_count;		// 1	// counts down until the next step
	u8 num;			// 2
	s16 x;			// 3-4
	s16 y;			// 5-6
	
	u8 view_cur;		// 7
	u8 *view_data;		// 8-9
	
	u8 loop_cur; 		// A
	u8 loop_total;		// B
	u8 *loop_data;		// C-D
	
	u8 cel_cur;			// E
	u8 cel_total;		// F
	u8 *cel_data; 		// 10-11
	u8 cel_prev_width;	// new ones added to prevent kq4 crashing
	u8 cel_prev_height;
	//u8 *cel_data_prev;	// 12-13
	BLIT *blit;			// 14-15
	
	s16 x_prev;			// 16-17
	s16 y_prev;		// 18-19
	s16 x_size;			// 1A-1B
	s16 y_size;			// 1C-1D
	u8 step_size;		// 1E
	u8 cycle_time; 		// 1F
	u8 cycle_count;		// 20	// counts down till next cycle
	u8 direction;		// 21
	u8 motion;			// 22
	u8 cycle;			// 23
	u8 priority;			// 24
	u16 flags;			// 25-26
	
	// 27-2A represent a union
	//u8 unknown27;		// 27	// these variables depend on the motion
	//u8 unknown28;		// 28	// type set by follow ego, move, obj.. stuff
	//u8 unknown29;		// 29	// like that
	//u8 unknown2A;		// 2A
	
	union {
		struct	// move_ego move_obj
		{
			s16 x;		// 27
			s16 y;		// 28
			u8 step_size;	// 29	// original stepsize
			u8 flag;	// 2A
		} move;
			
		struct	// follow_ego
		{
			u8 step_size;	// 27
			u8 flag;	// 28
			u8 count;	// 29
		} follow;
			
		// wander
		u8 wander_count;	// 27

		// reverse or end of loop
		u8 loop_flag;		// 27
	};
};

typedef struct view_struct VIEW;


struct vstring_struct
{
	u8 *data;
	u32 size;	// allocated mem size
	u32 min;	// minimum size
};

typedef struct vstring_struct VSTRING;

#define PBUF_MULT(width) ((( (width)<<2) + (width)) << 5)
#define SBUF_MULT(width) ((( (width)<<2) + (width)) << 6)


#define AGI_TRACE printf("trace at: file=%s, func=%s, line=%d\n", __FILE__,__func__, __LINE__);

#ifndef strdupa
#define strdupa(a) strcpy((char*)alloca(strlen(a) + 1), a)
#endif

#define CT_INT (0)
#define CT_BOOL (1)
#define CT_STRING (2)

typedef s32 CONF_INT;
typedef u32 CONF_BOOL;
typedef u8 *CONF_STRING;

struct conf_struct
{
	u8 *key;
	u8 *section;
	u8 type;
	
	union {
		struct
		{
			CONF_INT *ptr;
			CONF_INT def;
			s32 min;
			s32 max;
		} i;
			
		struct
		{
			CONF_BOOL *ptr;
			CONF_BOOL def;
		} b;
			
		struct
		{
			CONF_STRING *ptr;
			CONF_STRING def;
		} s;
	};
};
typedef struct conf_struct CONF;


extern CONF_BOOL c_nagi_log_debug;
extern CONF_BOOL c_nagi_console;
extern CONF_BOOL c_nagi_font_benchmark;
extern CONF_BOOL c_nagi_crc_print;
extern CONF_STRING c_nagi_dir_list;
extern CONF_STRING c_nagi_sort;
extern CONF_STRING c_vid_driver;
extern CONF_INT c_vid_scale;
extern CONF_BOOL c_vid_full_screen;
extern CONF_STRING c_vid_renderer;
extern CONF_STRING c_vid_pal_16;
extern CONF_STRING c_vid_pal_text;
extern CONF_STRING c_vid_pal_cga0;
extern CONF_STRING c_vid_pal_cga1;
extern CONF_STRING c_vid_pal_bw;
extern CONF_STRING c_vid_fonts_bitmap;
extern CONF_STRING c_vid_fonts_vector;
extern CONF_STRING c_snd_driver;
extern CONF_BOOL c_snd_enable;
extern CONF_BOOL c_snd_single;
extern CONF_INT c_snd_dissolve;
extern CONF_BOOL c_snd_read_var;
extern CONF_INT c_snd_volume;
extern CONF_STRING c_sdl_drv_video;
extern CONF_STRING c_sdl_drv_sound;

extern CONF_STRING c_standard_crc_list;
extern CONF_STRING c_standard_agi_list;
extern CONF_STRING c_standard_force;
extern CONF_STRING c_standard_v2_default;
extern CONF_STRING c_standard_v3_default;
extern CONF_STRING c_standard_amiga_v2_default;
extern CONF_STRING c_standard_amiga_v3_default;

extern CONF_STRING c_game_version_info;
extern CONF_INT c_game_mouse;
extern CONF_INT c_game_loop_update;
extern CONF_STRING c_game_id;
extern CONF_BOOL c_game_object_decrypt;
extern CONF_BOOL c_game_object_packed;
extern CONF_BOOL c_game_compression;
extern CONF_INT c_game_dir_type;

extern u8 c_game_file_id[ID_SIZE+1];
extern VSTRING *c_game_location;

extern CONF config_nagi[];
extern CONF config_standard[];
extern CONF config_game[];

#endif /* NAGI_AGI_H */
