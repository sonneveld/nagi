/* FUNCTION list 	---	---	---	---	---	---	---
verinit()
*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"
#include "gamelist.h"


/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
//#include <errno.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "../sys/ini_config.h"
#include "ver_init.h"
#include "../ui/msg.h"
#include "../ui/printf.h"
#include "../ui/window.h"
#include "../ui/agi_text.h"

#include "../base.h"


/* PROTOTYPES	---	---	---	---	---	---	--- */
GAMEINFO *standard_select(void);
void version_stat_write(void);
void dir_iterate(void);
void crc_search(u8 *search_dir, GAMEINFO *g_info);
u8 *crc_check(u8 *section);
void standard_init(GAMEINFO *g_info);


/* VARIABLES	---	---	---	---	---	---	--- */
GAMEINFO *default_game_info;

INI *ini_standard;


/* CODE	---	---	---	---	---	---	---	--- */



// dialog boxes needed:
// progress box
// option box
// selector box




GAMEINFO *standard_select()
{
	version_stat_write();
	memset(&default_game_info, 0, sizeof(default_game_info));
	dir_iterate();
	//print("     Welcome to NAGI\n\n<Press ENTER to continue>");


	// after this.. free up any data we have left
	
	return default_game_info;
}

void version_stat_write()
{
	push_row_col();
	text_attrib_push();

	window_line_clear(0, 0xFF);
	text_colour(0, 0x0F);
	goto_row_col(0, 5);
	agi_printf("New Adventure Game Interpreter");
 
	text_attrib_pop();
	pop_row_col();
}

void dir_iterate()
{
	u8 *key_value;
	ini_section(ini_standard, "standard");
	if ((key_value=ini_key(ini_standard, "force")) != 0)
		default_game_info = game_list_force(key_value);
	else
		default_game_info = game_list_new(".");
}


void standard_init(GAMEINFO *g_info)
{
	u8 *key_value;
	u8 *parent = 0;
	
	if (g_info == 0)
	{
		printf("standard_init() error - g_info is null\n");
		agi_exit();
	}
	if (g_info->format == 0)
	{
		printf("standard_init() error - no agi data (v2 and v3) detected\n");
		agi_exit();
	}
	
	if (g_info->standard != 0)
		printf("Reading and initialising \"%s\" AGI standard...", g_info->standard);
	else
		printf("Initialising default AGI standard...");
	
	
	// read_strng

	// inherited parent
	//if ( (key_value=ini_read(g_info->standard, "inherits")) != 0)
	//{
	//	parent = strdup(key_value);
	//	//printf("  Standard has parent \"%s\".\n", parent);
	//}
	
	ini_section(ini_standard, g_info->standard);
	
	// game_id
	// if crc match then use the standard.ini game.id
	// if a resource guess then use the guessed id
	// otherwise, use gameIDX
	standard.game_id[0] = 0;
	
	if (g_info->crc_match != 0)
	{
		if ( (key_value=ini_key(ini_standard, "game_id")) != 0)
		{
			//printf("  Using standard defined game id (%s).\n", key_value);
			strncpy(standard.game_id, key_value, 7);
			standard.game_id[strlen(key_value)] = 0;
		}
	}
	
	if ( (g_info->id != 0) && (standard.game_id[0] == 0) )
	{
		//printf("  Using *guessed* game id (%s).\n", g_info->id);
		strncpy(standard.game_id, g_info->id, 7);
		standard.game_id[strlen(g_info->id)] = 0;
	}
	
	if (standard.game_id[0] == 0)
	{
		if ( (key_value=ini_key(ini_standard, "game_id")) != 0)
		{
			//printf("  Using default standard defined game id (%s).\n", key_value);
			strncpy(standard.game_id, key_value, 7);
			standard.game_id[strlen(key_value)] = 0;
		}
	}
	
	if (standard.game_id[0] == 0)
	{
		//printf("  Using *DEFAULT* game id (gameIDX).\n");
		strncpy(standard.game_id, "gameIDX", 7);
		standard.game_id[strlen("gameIDX")] = 0;
	}
	
	// game_id_honour
	standard.game_id_honour = ini_boolean(ini_standard, "game_id_honour", 0);
	// object_decrypt
	standard.object_decrypt = ini_boolean(ini_standard, "object_decrypt", 1);
	// mouse_support
	standard.mouse = ini_int(ini_standard, "mouse_support", 1);
	
	// res_type
	switch(g_info->format)
	{
		case 2:
		case 3:
			standard.cmd_max = ini_int(ini_standard, "cmd_max", 181);
			standard.res_type = ini_int(ini_standard, "res_type", RES_V3);
			standard.ver_major = ini_int(ini_standard, "ver_major", 3);
			standard.ver_minor = ini_int(ini_standard, "ver_minor", 149);
			standard.update_loop = ini_int(ini_standard, "update_loop", L_FLAG);
			break;
		case 1:
		default:
			standard.cmd_max = ini_int(ini_standard, "cmd_max", 175);
			standard.res_type = ini_int(ini_standard, "res_type", RES_V2);
			standard.ver_major = ini_int(ini_standard, "ver_major", 2);
			standard.ver_minor = ini_int(ini_standard, "ver_minor", 936);
			standard.update_loop = ini_int(ini_standard, "update_loop", L_FOUR);
	}
	if (standard.cmd_max > 0xFB)
		standard.cmd_max = 0xFB;
	
	if (parent!=0)
	{
		free(parent);
		parent=0;
	}
	
	printf("done.\n\n");
	
}



