/* FUNCTION list 	---	---	---	---	---	---	---
_AgiInit                         cseg     0000115C 00000057
_GameInit                        cseg     000011B3 0000012B
_RoomInit                        cseg     000012DE 00000015
*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "agi.h"
#include "initialise.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h> 

/* OTHER headers	---	---	---	---	---	---	--- */
#include "sys/drv_video.h"
#include "sys/gfx.h"
#include "sys/glob_sys.h"
#include "picture/pic_res.h"
#include "game_id.h"
#include "flags.h"
#include "decrypt.h"
#include "res/res.h"
#include "logic/logic_base.h"
#include "picture/sbuf_util.h"
// for blists_erase
#include "view/obj_update.h"
#include "view/obj_base.h"
#include "view/view_base.h"
#include "sound/sound.h"
// words.tok.data
#include "ui/parse.h"
// event_init
#include "ui/events.h"
// controller
#include "ui/controller.h"
// text colour
#include "ui/agi_text.h"
// input_redraw
#include "ui/cmd_input.h"
// byte-order support
#include "sys/endian.h"
#include "objects.h"
#include "sys/time.h"
#include "sys/ini_config.h"

#include "logic/cmd_table.h"
#include "ui/mouse.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
// reads ini file and inits nagi
void nagi_init(void);
// after reading the version, inits this particular version of agi
void agi_init(void);
// for a new game.. can be called later by restart/restore
void game_init(void);
// refresh lists for new room
void room_init(void);

void config_read(void);
void arrange_mem(void);
void get_hardware_info(void);


/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


void nagi_init()
{
	memset( &state, 0, sizeof(AGI_STATE) );
	state.word_13f = 0x0F;
	state.script_size = 0x32;
	state.input_pos = 0x17;
	state.status_line_row = 0x15;
	state.menu_state = 1;
	
	// for the console window thingy
	freopen( "CON", "w", stdout );	
	
	printf("New Adventure Game Interpreter (NAGI) %s\n", NAGI_VERSION);
	printf("Copyright (C) 2000-2001 Nick Sonneveld\n");
	printf("Author: Nick Sonneveld\n\n");
	
	printf("Based upon the Adventure Game Interpreter (AGI) v2.917 and v3.002.149\n");
	printf("Copyright (C) 1984-1988 Sierra On-Line, Inc.\n");
	printf("Authors: Jeff Stephenson & Chris Iden\n\n");
	
	// read nagi.ini
	config_read();

	// get hardware info
	get_hardware_info();

	// initialise SDL
	printf("Initialising Simple DirectMedia Layer (SDL)...");
	//SDL_INIT_EVENTTHREAD SDL_INIT_AUDIO|
	if ( SDL_Init(SDL_INIT_TIMER) < 0 )
	{
		printf("Unable to init SDL: %s\n", SDL_GetError());
		exit(1);
	}
	atexit(SDL_Quit);
	printf("done.\n");
	
	gfx_init();
	
	// clear keyboard input
	//clear_input();
	#warning keyboard needs a bit of cleanup
	events_init();
	
	//load_vectors();	// keyboard, timer, critical error
	
	// call do_clock at 20Hz.
	clock_init();
	sound_driver_init();
	
	/*
	input_init();	// inits joystick
	*/
	text_colour(15, 0);
	input_redraw();
	
	// allocate memory for the LZW dictionary
	lzw_dict = malloc(0x800 * sizeof(DICT));
}

void agi_init()
{
	// initialise any function changes from the norm 
	// quit 0-1 param
	
	// cmd171 for brian mouse support
	if (standard.mouse == M_BRIAN)
	{
		cmd_table[171].func_name = "cmd.poll.mouse";
		cmd_table[171].func = cmd_brian_poll_mouse;
		cmd_table[171].param_total = 0;
		cmd_table[171].param_flag = 0;
	}
		
	// printat support
	
	// cmd176 if it's EVER used
	
	
	
	/* load directories for the various resources */
	/* logics, pictures, views, sound */
	dir_load();

	words_tok_data = file_load("words.tok", 0);
	logic_list_init();
	view_list_init();
	sound_list_init();
	pic_list_init();

	game_init();	
	
	logic_load_2(0);
	//_SetMemRm0();	// pointer to data AFTER loaded logic 0

	flag_set(9);	// turn sound on
}

void game_init(void)
{
	u16 name_offset;					// invent offset
	u16 anim_obj_max;
	
	if ( object != 0 )
		object -= 3;
	object = file_load("object", object);
	if (standard.object_decrypt != 0)
	{
		printf("Decrypting 'object' file...");
		decrypt_string(object, object + res_size);
		printf("done.\n");
	}
	object_size = res_size - 3;
	
	name_offset = load_le_16(object);
	anim_obj_max = object[2] + 1;
	object += 3;					// skip the header
	object_name = object + name_offset;

	objtable_new(anim_obj_max);			// new view table baby
	memset(state.var, 0, sizeof(state.var));	// clear variables
	flags_clear();
	control_state_clear();
	room_init();				// reset file data stuff to 0
	blists_erase();

	state.var[V20_COMPUTER] = computer_type;
	state.var[V26_MONITORTYPE] = display_type;
	state.var[V24_INPUTLEN] = 0x29;
	flag_set(F05_NEWROOM);			// first time in room.

	state.ego_control_state = 1;
	state.pic_num = 0;				// used in drawPic (dseg:13b)
	state.block_state = 0;				// a block is not set

	if (computer_type == 0)
		state.var[V22_SNDTYPE] = 1;	// pc
	else
	{
		state.var[V22_SNDTYPE] = 3;	// tandy
		flag_set(F11_NEWLOGIC0);		// logic 0 executed for first time.
	}
}

void room_init(void)
{
	logic_list_new_room();
	view_list_new_room();
	sound_list_new_room();
	pic_list_new_room();
}




#warning fix arrange_mem() for the stack-based mem to work
void arrange_mem()
{
}

void get_hardware_info()
{
	computer_type = 0;	// ibm pc
	drives_found = 1;	// yes, drives are known of
	display_type = 3;		// ega
	//text_mode = 0;
}


void config_read(void)
{
	u8 *key_value;
	u8 env_value[50];
	
	printf("\nReading NAGI configuration file...");
	// read_strng
	ini_open("nagi.ini");

	// video_driver
	if ( (key_value=ini_read("video", "video_driver")) != 0)
	{
		// mingw does not have snprintf... shocking
		if (strlen(key_value) < 15)
		{
			sprintf(env_value, "%s=%s", "SDL_VIDEODRIVER", key_value);
			putenv(env_value);
		}
	}
	else
		putenv("SDL_VIDEODRIVER=windib");
	// video fullscreen
	gfx_fullscreen = ini_boolean("video", 0, "video_fullscreen", 0);
	// video scale
	gfx_scale = ini_int("video", 0, "video_scale", 1);
	if (gfx_scale < 1)
			gfx_scale = 1;
	
	// sound_driver
	if ( (key_value=ini_read("sound", "sound_driver")) != 0)
	{
		if (strlen(key_value) < 15)
		{
			sprintf(env_value, "%s=%s", "SDL_AUDIODRIVER", key_value);
			putenv(env_value);
		}
	}
	else
		putenv("SDL_AUDIODRIVER=waveout");
	// sound disable
	nagi_sound_disable = ini_boolean("sound", 0, "sound_disable", 0);
	
	// logic_debug
	// sample
	// sample freq
	ini_close();
	printf("done.\n");
}
