/* FUNCTION list 	---	---	---	---	---	---	---
_AgiInit                         cseg     0000115C 00000057
_GameInit                        cseg     000011B3 0000012B
_RoomInit                        cseg     000012DE 00000015
*/

//~ RaDIaT1oN (2002-04-29):
//~ don't redirect console for linux
//~ definately credit radiation for his fine linux porting
//~ use setenv for setting environment under linux
//~ warning placement

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
#include "sound/sound_base.h"
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

#include "logic/cmd_table.h"
#include "ui/mouse.h"

#include "sys/ini_config.h"
#include "config.h"

#include "sys/sys_dir.h"

#include "sound/sound_gen.h"
#include "base.h"
#include "sys/mem_wrap.h"

#include "log.h"


/* PROTOTYPES	---	---	---	---	---	---	--- */
// reads ini file and inits nagi
void nagi_init(void);
// after reading the version, inits this particular version of agi
void agi_init(void);
// for a new game.. can be called later by restart/restore
void game_init(void);
// refresh lists for new room
void room_init(void);


/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


void nagi_init()
{	
#ifdef  _WIN32
	u8 env_value[50];
#endif
	INI *ini_nagi;

	memset( &state, 0, sizeof(AGI_STATE) );
	state.word_13f = 0x0F;
	state.script_size = 0x32;
	state.input_pos = 0x17;
	state.status_line_row = 0x15;
	state.menu_state = 1;
	computer_type = 0;	// ibm pc
	drives_found = 1;	// yes, drives are known of
	display_type = 0;		// ega
	//text_mode = 0;

	// read nagi.ini
	dir_preset_change(DIR_PRESET_NAGI);
	ini_nagi = ini_open("nagi.ini");
	config_load(config_nagi, ini_nagi);
	ini_close(ini_nagi);

	printf("New Adventure Game Interpreter (NAGI) %s\n", NAGI_VERSION);
	printf("Copyright (C) 2000-2002 Nick Sonneveld & Gareth McMullin\n");
	printf("Author: Nick Sonneveld (sonneveld@hotmail.com)\n");
	printf("Linux port by RaDIaT1oN (g_mcm@mweb.co.za)\n");
	printf("SDL 2.0 port by Ritchie Swann (ritchieswann@gmail.com)\n\n");
	
	printf("Based upon the Adventure Game Interpreter (AGI) v2.917 and v3.002.149\n");
	printf("Copyright (C) 1984-1988 Sierra On-Line, Inc.\n");
	printf("Authors: Jeff Stephenson & Chris Iden\n\n");

	// initialise SDL
	printf("Initialising Simple DirectMedia Layer (SDL)...");

	// setting environment vars is touchy...
#ifdef NAGI_SDLENV
	// video_dirver
	if (strlen(c_sdl_drv_video) > 30)
		c_sdl_drv_video[30] = 0;
	if (strlen(c_sdl_drv_sound) > 30)
		c_sdl_drv_sound[30] = 0;

#ifdef  _WIN32
	sprintf(env_value, "SDL_VIDEODRIVER=%s", c_sdl_drv_video);
	putenv(env_value);
	// audio driver
	sprintf(env_value, "SDL_AUDIODRIVER=%s", c_sdl_drv_sound);
	putenv(env_value);
#else	
	if(c_sdl_drv_video[0]) 
		setenv("SDL_VIDEODRIVER", c_sdl_drv_video, 1);

	if(c_sdl_drv_sound[0])
		setenv("SDL_AUDIODRIVER", c_sdl_drv_sound, 1);
#endif
	
#endif

	//SDL_INIT_EVENTTHREAD SDL_INIT_AUDIO|
	if ( SDL_Init(SDL_INIT_TIMER|SDL_INIT_VIDEO|SDL_INIT_AUDIO) != 0 )
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
	sndgen_init();

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

		
	// printat support
	
	// cmd176 if it's EVER used
	
	mouse_init();
	
	/* load directories for the various resources */
	/* logics, pictures, views, sound */
	dir_load();
	
	dir_preset_change(DIR_PRESET_GAME);
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
	if (object_file_load())
		agi_exit();
	memset(state.var, 0, sizeof(state.var));	// clear variables
	flags_clear();
	control_state_clear();
	room_init();				// reset file data stuff to 0
	blists_erase();

	state.var[V20_COMPUTER] = computer_type;
	state.var[V26_MONITORTYPE] = display_type;
	state.var[V24_INPUTLEN] = 0x29;
	state.var[V08_FREEMEM] = 10;
	
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


void agi_shutdown(void)
{
	// anything opened/alloced in game_init and during the game
	
	// object_free
	object_file_unload();
	
	// blists free
	blists_free();
	
	// log file
	//close_log_file();
	
	// menus
	// res
	// window name
	// TODO
	
	// the agi info that is init'd every game
	
	//pic_list_free();
	view_list_free();
	//logic_list_free();
	//sound_list_free();
	
	// words.tok free
	a_free(words_tok_data);
	words_tok_data = 0;
	
	// dir_load free
	dir_unload();
	
	// mouse shutdown
	mouse_shutdown();
	
	log_close();
}

void nagi_shutdown(void)
{
	// lzw dict
	if (lzw_dict != 0)
		free(lzw_dict);
	lzw_dict = 0;

	//sound_shutdown
	sndgen_shutdown();
	// clock_shutdown
	clock_denit();

	// events_shutdown
	// TODO during events rewrite
	
	// gfx_shutdown
	gfx_shutdown();
	
	// sdl_shutdown
	SDL_Quit();
	
	// console close
	// config write
	// TODO
}
