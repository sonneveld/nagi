/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

//~ RaDIaT1oN:
//~ fix const declaration in compare func
//~ cast in strtok_r
//~ lower string for linux
//~ unnamed union members

#ifdef RAD_LINUX
#include <unistd.h>
#endif

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
#include <assert.h>

/* OTHER headers	---	---	---	---	---	---	--- */
#include "../sys/ini_config.h"
#include "../config.h"
#include "../ui/string.h"
#include "../sys/agi_file.h"
#include "agi_crc.h"
#include "../sys/mem_wrap.h"
#include "../sys/vstring.h"
#include "../sys/sys_dir.h"
#include "../base.h"
#include "../ui/msg.h"
#include "../list.h"
#include "../ui/list_box.h"
#include "../ui/events.h"
#include "../ui/window.h"
#include "../ui/agi_text.h"
#include "../ui/printf.h"

#include "../sys/drv_video.h"
#include "../sys/gfx.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);


// two structs.. one for stuff i just need to help ident and
// the other to help load up/print menu
struct gameinfo_struct
{
	struct gameinfo_struct *next;
		
	u8 name[81];	// 80 + null
	VSTRING *dir;
	
	u8 *standard;
	u8 file_id[ID_SIZE+1];
	u8 dir_type;
	u8 ver_type;	// used to pick defaults
};
typedef struct gameinfo_struct GAMEINFO;

struct agicrc_struct
{
	u32 object;
	u32 words;
	
#ifndef RAD_LINUX
	union
	{
#endif
		u32 dir_comb;
		
		struct
		{
			u32 log;
			u32 pic;
			u32 view;
			u32 snd;
		} dir;
#ifndef RAD_LINUX
	};
#endif

	u32 vol[16];
};
typedef struct agicrc_struct AGICRC;

/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


// ---------------------------------------- LIST INIT ----------------------------------------

// generate a crc from a file
int file_crc_gen(u8 *file_name, u32 *crc32)
{
	u8 *buf;
	
	assert(file_name != 0);
	assert(crc32 != 0);
	
	buf = file_to_buf(file_name);
	
	if (buf == 0)
	{
		*crc32 = 0;
		return 1;
	}
	else
	{
		// generate crc	
		*crc32 = crc_generate(buf, file_buf_size);
		
		a_free(buf);
		return 0;
	}
}


u8 st_end_char;
u8 *st_end = 0;

// hopefully a pointer to the standard you wanna sep
void stand_sep(u8 *ptr)
{
	assert(ptr != 0);
	
	st_end = strstr(ptr, ",\0\n\r");
	if (st_end != 0)
	{
		st_end_char = *st_end;
		*st_end = 0;
	}
}

void stand_rejoin(void)
{
	if (st_end != 0)
	{
		*st_end = st_end_char;
		st_end = 0;
	}
}

// read the crc/game type/directory type and determine if the game is a proper agi game
// 0 = ok
// anything else is a failure
int dir_get_info(AGICRC *agicrc, GAMEINFO *info)
{
	int i;
	u8 name[ID_SIZE + 20];
	
	assert(agicrc != 0);
	assert(info != 0);
	
	// search for object
	if (file_crc_gen("object", &agicrc->object) != 0)
		return 1;	// NO OBJECT
	
	// search for words.tok
	if (file_crc_gen("words.tok", &agicrc->words) != 0)
		return 2;	// NO WORDS.TOK
	
	// search for vol.0
	if (file_crc_gen("vol.0", &agicrc->vol[0]) == 0)
		info->file_id[0] = 0;	// if found then no file FILEID  
	// else search for FILEIDvol.0
	// if found set file_id (from func)
	else
	{
		FIND find_vol;
		u8 *fname, *tail;
		int ok = 0;
		
		fname = find_first(&find_vol, "*vol.0");
//		printf("Found vol: %s\n", fname);
		while (fname != 0)
		{
			fname = string_lower(fname);	// to lower the name
			tail = strstr(fname, "vol.0");	// get id
			
			// check size of it
			if (((tail - fname) < ID_SIZE) && (tail != 0))
				// get crc
				if (file_crc_gen(fname, &agicrc->vol[0]) == 0)
				{
					*tail = 0;
					strcpy(info->file_id, fname);
					ok = 1;
					break;
				}
			
			fname = find_next(&find_vol);
//			printf("Found vol: %s\n", fname);
		}
		find_close(&find_vol);
		
		if (!ok)
			return 3;	// NO *VOL.0
	}
	
	// search for logdir, snddir, viewdir, picdir
	if ( (file_crc_gen("logdir", &agicrc->dir.log) |
		file_crc_gen("snddir", &agicrc->dir.snd) |
		file_crc_gen("viewdir", &agicrc->dir.view) |
		file_crc_gen("picdir", &agicrc->dir.pic)) == 0) 
	{
		// if found then 4 dirs
			info->dir_type = DIR_SEP;
			info->ver_type = 2;
	}
	// else search for dirs
	else if (file_crc_gen("dirs", &agicrc->dir_comb) == 0)
	{
		info->dir_type = DIR_AMIGA; // if found then amiga v3 dir
		info->ver_type = 3;
	}
	else 
	{
		// else search for FILEIDdir
		sprintf(name, "%sdir", info->file_id);
		if (file_crc_gen(name, &agicrc->dir_comb) == 0)
		{
			info->ver_type = 3;
			info->dir_type = DIR_COMB;
		}
		else
			return 4;	// NO DIRs
	}
	
	// after that.. the rest of the volumes 1-15
	// not necessary to detect the game. but we need the crc's
	for (i=1; i<=15; i++)
	{
		sprintf(name, "%svol.%d", info->file_id, i);
		file_crc_gen(name, &(agicrc->vol[i]));
	}
	
	return 0; 	// everything ok.
}


void crc_print(AGICRC *agicrc, GAMEINFO *info)
{
	u8 key_vol[20];	// twice as much needed
	int i;
	
	assert(agicrc != 0);
	assert(info != 0);
	
	// print object
	if (agicrc->object)
		printf("%s=0x%08X\n", "crc_object", (unsigned int)agicrc->object);
	
	// print words
	if (agicrc->words)
		printf("%s=0x%08X\n", "crc_words", (unsigned int)agicrc->words);
	
	// print vol[0] - vol[15]
	for (i=0; i<16; i++)
	{
		sprintf(key_vol, "crc_vol_%d", i);
		if (agicrc->vol[i])
			printf("%s=0x%08X\n", key_vol, (unsigned int)agicrc->vol[i]);
	}
	
	switch (info->dir_type)
	{
		case DIR_COMB:
		case DIR_AMIGA:
			// print dir_comb
			if (agicrc->dir_comb)
				printf("%s=0x%08X\n", "crc_dir", (unsigned int)agicrc->dir_comb);
			break;
		
		default:
		case DIR_SEP:
			// print dir.log
			if (agicrc->dir.log)
				printf("%s=0x%08X\n", "crc_dir_log", (unsigned int)agicrc->dir.log);
			// print dir.pic
			if (agicrc->dir.pic)
				printf("%s=0x%08X\n", "crc_dir_pic", (unsigned int)agicrc->dir.pic);
			// print dir.view
			if (agicrc->dir.view)
				printf("%s=0x%08X\n", "crc_dir_view", (unsigned int)agicrc->dir.view);
			// print dir.snd
			if (agicrc->dir.snd)
				printf("%s=0x%08X\n", "crc_dir_snd", (unsigned int)agicrc->dir.snd);
			break;
	}
}


#define CRC_FUDGE(x, y, z) key=ini_key(ini, (x) ); key_touched |= (int)key; \
					if ( (key!= 0) \
					&& (agicrc->y != strtoul(key, 0, 16)) ) \
						return (z)


// compare each crc found for the game with the one's AVAILABLE in the standard.ini
// hopefully the ini file has been set to the appropriate section
// otherwise we'll crash 'n burn and destroy the whole world.
// return 0 if ok
int crc_compare(AGICRC *agicrc, GAMEINFO *info, INI *ini)
{
	u8 *key;
	u8 key_vol[20];	// twice as much needed
	int key_touched = 0;
	int i;
	
	assert(agicrc != 0);
	assert(info != 0);
	assert(ini != 0);
	
	// compare object
	CRC_FUDGE("crc_object", object, 1);
	
	// compare words
	CRC_FUDGE("crc_words", words, 2);
	
	// compare vol[0] - vol[15]
	for (i=0; i<16; i++)
	{
		sprintf(key_vol, "crc_vol_%d", i);
		CRC_FUDGE(key_vol, vol[i], i+3);
	}
	
	switch (info->dir_type)
	{
		case DIR_COMB:
		case DIR_AMIGA:
			// compare dir_comb
			CRC_FUDGE("crc_dir", dir_comb, 19);
			break;
		
		default:
		case DIR_SEP:
			// compare dir.log
			CRC_FUDGE("crc_dir_log", dir.log, 20);
			// compare dir.pic
			CRC_FUDGE("crc_dir_pic", dir.pic, 21);
			// compare dir.view
			CRC_FUDGE("crc_dir_view", dir.view, 22);
			// compare dir.snd
			CRC_FUDGE("crc_dir_snd", dir.snd, 23);
			break;
	}
	// no keys to match!
	if (key_touched == 0)
		return 24;
	return 0;
}


#undef CRC_FUDGE

// open up each section on the list and compare available crc's with one's calculated before
// return pointer to section in section list if a match is found.
u8 *crc_search(AGICRC *agicrc, GAMEINFO *info, INI *ini)
{
	u8 *crc_list; 
	u8 *token, *running;
	
	assert(agicrc != 0);
	assert(info != 0);
	assert(ini != 0);
	
	crc_list = strdupa(c_standard_crc_list);
	
	token = strtok_r(crc_list, ",", (char**)&running);
	while (token != 0)
	{
		if (ini_section(ini, token) == 0)
		{
			if (crc_compare(agicrc, info, ini)== 0)
				return c_standard_crc_list + (token - crc_list);
		}
		token = strtok_r(0, ",", (char**)&running);
	}

	return 0;	
}

// from the amiga gr vol.0
// replicated comp and uncomp.. lzw comp!!
// 12 34 00 2F 00 2F 00

// generate a name for the game info from standard.ini or just from other available resrouces
void gameinfo_namegen(GAMEINFO *info, INI *ini, u8 *dir_sub, u8 *dir)
{
	u8 *name = 0;

	assert(info != 0);
	assert(dir != 0);
	assert(dir_sub != 0);
	
	// standard separate
	if ( (info->standard != 0) && (ini != 0) )
	{
		stand_sep(info->standard);
		// go to section
		ini_section(ini, info->standard);
		// read name
		name = ini_key(ini, "name");
	}
	
	if (name == 0)
	{
		int str_size;
		
		if (info->file_id[0] == 0)
		{
			// "v3_kq4/games - AGI v2"
			str_size = strlen(dir_sub) + strlen(dir) + 10 + strlen("/ - AGI v");
			name = alloca(str_size);
			sprintf(name, "%s/%s - AGI v%d", dir_sub, dir, info->ver_type);
		}
		else
		{
			// "KQ4 @ v3_kq4/games - AGI v2"
			str_size = strlen(info->file_id) + strlen(dir_sub) + strlen(dir) + 10 + strlen(" @ / - AGI v");
			name = alloca(str_size);
			sprintf(name, "%s @ %s/%s - AGI v%d", info->file_id, dir_sub, dir, info->ver_type);
		}
	}
	
	assert(name != 0);
	
	strncpy(info->name, name, 80);
	if (strlen(name) >= 80)
		info->name[80] = 0;
	else
		info->name[strlen(name)] = 0;

	stand_rejoin();
}

int game_count = 0;


// create a new gameinfo struct for a given directory if a game exists in it.. 
// called on for each dir
void gameinfo_add(LIST *list, INI *ini, u8 *dir_sub, u8 *dir)
{
	AGICRC agicrc;
	GAMEINFO info_new;
	GAMEINFO *info;
	u8 msg[strlen("Games found: XXXXXXXXXXXX")];
	
	assert(list != 0);
	
	memset(&info_new, 0, sizeof(GAMEINFO) );
	
	if (dir_get_info(&agicrc, &info_new) == 0)
	{
		if (ini != 0)
			info_new.standard = crc_search(&agicrc, &info_new, ini);
		
		//if (standard == 0)
		// don't worry... we'll just read the conf values.. don't have to dup strings all the time	
		
		// get directory
		info_new.dir = vstring_new(0, 100);
		vstring_getcwd(info_new.dir);
		
		// get name
		gameinfo_namegen(&info_new, ini, dir_sub, dir);

		if (c_nagi_crc_print)
		{
			printf("%s\n-----------------------------------------\n", info_new.name);
			crc_print(&agicrc, &info_new);
			printf("\n");
		}
		
		// if everything ok
		// ADD TO LIST
		info = list_add(list);
		game_count++;
		sprintf(msg, "Games found: %d", game_count);
		message_box_draw(msg, 0, 0, 0);
		memcpy( info, &info_new, sizeof(GAMEINFO) );
	}
}

// create a list of game infos starting from gameinfo_head from the dirlist in standard.ini
// search one level into it too if possible
void gi_list_init(LIST *list, INI *ini)
{
	u8 *dir_list;
	u8 *token, *running;
	DIR *dp;
	struct dirent *ep;
	struct stat fs;
	
	assert(list != 0);

	game_count = list_length(list);
	
	// for each dir
	
	dir_list = strdupa(c_nagi_dir_list);
	token = strtok_r(dir_list, ";", (char**)&running);
	while (token != 0)
	{
		dir_preset_change(DIR_PRESET_ORIG);
		
		dp = opendir(token);
		if (dp != 0)
		{
			while ((ep = readdir(dp))!= 0)
			{
				dir_preset_change(DIR_PRESET_ORIG);
				if ( (chdir(token) != -1) &&
					(stat(ep->d_name, &fs) == 0) &&
					(S_ISDIR(fs.st_mode)) &&
					(strcmp(ep->d_name, "..") != 0) &&
					(chdir(ep->d_name) != -1) )
				{
					//printf("trying %s\\%s...\n", token, ep->d_name);
					gameinfo_add(list, ini, ep->d_name, token);
				}
			}
			closedir(dp);
		}
		
		token = strtok_r(0, ";", (char**)&running);
	}
	
	dir_preset_change(DIR_PRESET_ORIG);
}














// ---------------------------------------- MENU ----------------------------------------

// display nice menu of games and allow user to select game to play
// returns 0 on error
GAMEINFO *gi_list_menu(LIST *list)
{
	int list_size, selection;
	u8 **str_list, **str_cur;
	u8 *msg;
	u8 newline_orig;	// d0d orig
	GAMEINFO *info;
	
	assert(list != 0);

	// get the size of the list
	list_size = list_length(list);

	// check for errors
	if (list_size == 0)
		return 0;
	if (list_size == 1)
		return list_element_head(list);
	
	// allocate list
	str_list = alloca(sizeof(u8 *) * (list_size+1) );
	// set the first item to instruction string
	str_list[0] = "Use the arrow keys to select the game which you wish to play.\nPress ENTER to play the game, ESC to not play a game.";
	
	// get the rest of the strings
	info = list_element_head(list);
	str_cur = str_list;
	while(info)
	{
		str_cur++;
		*str_cur = info->name;
		info = node_next(info);
	}
	
	// use list_box
	selection = 0;
top:
	selection = list_box(str_list, list_size, selection);
	cmd_close_window(0);
	
	if (selection == -1)
		return 0;
	
	// convert number to required info struct
	info = list_element_at(list, selection);
	
	assert(info != 0);

	// check user response.
	// uses malloc so I can free it again after.
	// i free again after in case the user decides to pick and cancel a whole bunch of games
	// i didn't want to fill up the stack
	msg = a_malloc(200 + strlen(info->name) + strlen(info->dir->data));
	newline_orig = msgstate.newline_char;
	msgstate.newline_char = '@';
	sprintf(msg, "About to execute the game\ndescribed as:\n\n%s\n\nfrom dir:\n %s\n\n%s",
		info->name, info->dir->data, "Press ENTER to continue.\nPress ESC to cancel.");
	message_box_draw(msg, 0, 0x23, 0);
	a_free(msg);
	msg = 0;
	msgstate.newline_char = newline_orig;
	if (user_bolean_poll() == 0)
		goto top;

	return info;
}


// ---------------------------------------- ITEM INIT ----------------------------------------

u8 *window_caption = 0;

void standard_init_ng(GAMEINFO *game, INI *ini)
{
	assert(game != 0);
	
	// set the defaults

	// obj_packed
	if (game->dir_type == DIR_AMIGA)
	{
		config_game[5].b.def = 1;
	}
	else
		config_game[5].b.def = 0;
	
	// dir_type
	config_game[7].i.def = game->dir_type;
	
	switch (game->ver_type)
	{
		case 3:
			// version info
			config_game[0].s.def = "v3.002.149";
			// loop update
			config_game[2].i.def = L_FLAG;
			// compression
			config_game[6].b.def = 1;
			break;
		case 2:
		default:
			// version info
			config_game[0].s.def = "v2.917";
			// loop update
			config_game[2].i.def = L_FOUR;
			// compression
			config_game[6].b.def = 0;
			break;
	}
	
	// change to appropriate section
	// use the v2 or v3 section defaults if available
	
	if (ini != 0)
	{
		if (game->standard != 0)
		{
			stand_sep(game->standard);
			ini_section(ini, game->standard);
			stand_rejoin();
		}
		else if (game->dir_type == DIR_AMIGA)
			// I know it won't read amiga_v2_Default.. but it's a start
			switch(game->ver_type)
			{
				case 3:
					ini_section(ini, c_standard_amiga_v3_default);
					break;
				case 2:
				default:
					ini_section(ini, c_standard_amiga_v2_default);
					break;
			}
		else
			switch(game->ver_type)
			{
				case 3:
					ini_section(ini, c_standard_v3_default);
					break;
				case 2:
				default:
					ini_section(ini, c_standard_v2_default);
					break;
			}
	}
	
	// read in file_id
	strcpy(c_game_file_id, game->file_id);
	
	// set up location
	//c_game_location = vstring_new(game->dir->data, 10);
	dir_preset_set(DIR_PRESET_GAME, game->dir->data);
	
	// need to setup game.id if you want to load up savegames before agi is init'd
	// standard.ini, or the file_id, or the default ""
	config_game[3].s.def = c_game_file_id;
	
	// use config_Read
	config_load(config_game, ini);
	
	// dont' store name.. use it for window only
	// init window name
	if ( (game->standard != 0) && (game->name[0] != 0) )
	{
		window_caption = a_malloc(strlen(game->name) + strlen("NAGI - ") + 10);
		sprintf(window_caption, "%s - NAGI", game->name);
	}
	else
		window_caption = "NAGI";
	SDL_WM_SetCaption(window_caption, 0);
}


// ---------------------------------------- MAIN ----------------------------------------


int gameinfo_compare(const void *a, const void *b)
{
	const GAMEINFO *info_a = *((const GAMEINFO **)a);
	const GAMEINFO *info_b = *((const GAMEINFO **)b);
	
	//~ info_a = *((GAMEINFO **)a);
	//~ info_b = *((GAMEINFO **)b);
	
	return strcasecmp(info_a->name, info_b->name);
}

// destory list
void gi_list_free(LIST *list)
{
	GAMEINFO *info;
	
	assert(list);
	
	info = list_element_head(list);
	while (info)
	{
		vstring_free(info->dir);
		info = node_next(info);
	}
}

void text_init(void)
{
	state.window_row_min = 2;
	state.window_row_max = 23;
	state.status_line_row = 0;
	
	push_row_col();
	text_attrib_push();

	window_line_clear(0, 0xFF);
	text_colour(0, 0x0F);
	goto_row_col(0, 5);
	agi_printf("New Adventure Game Interpreter");
 
	text_attrib_pop();
	pop_row_col();
}

void text_shutdown(void)
{
	// clear window
	gfx_clear();
	
	state.window_row_min = 0;
	state.window_row_max = 0;
	state.status_line_row = 0x15;
}


void standard_select_ng(void)
{
	INI *ini_standard;
	LIST *list_game;
	GAMEINFO *game_selected;
	
	dir_preset_change(DIR_PRESET_NAGI);
	ini_standard = ini_open("standard.ini");
	text_init();

	// read in standard parameters
	config_load(config_standard, ini_standard);
	
	list_game = list_new(sizeof(GAMEINFO));
	gi_list_init(list_game, ini_standard);
	
	if ( msgstate.active != 0)
		cmd_close_window(0);
	
	if (list_length(list_game) == 0)
	{
		printf("no games detected\n");
		agi_exit();
	}
	
	// SORT THE LIST
	list_sort(list_game, gameinfo_compare);
	
	// MENU
	game_selected = gi_list_menu(list_game);
	if (game_selected == 0)
	{
		printf("user exits.\n");
		agi_exit();
	}

	// init game
	standard_init_ng(game_selected, ini_standard);

	// close ini file
	dir_preset_change(DIR_PRESET_NAGI);
	ini_close(ini_standard);

	gi_list_free(list_game);	// free up any alloc'd data
	list_free(list_game);
	
	text_shutdown();
}

// TODO
// menu WHOOOOO DONE
// standard_separation DONE
// what happens if a dir doesn't exist on list? WORKS
// what happens if no game is set NOT CHECKED
// what happens if there's no crc's on the list WORKS
// if standard.ini doesn't exist.. where's dir_list come from? FROM THE CONFIG DEFAULT
// what if ini_key is called without claling ini_section? WORK
// instead of c_game_location.. use the dir_preset idea?  (nagi, user, game)  DONE BABY!!!

