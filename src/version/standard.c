/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "../sys/ini_config.h"
#include "../config.h"
#include "../ui/string.h"
#include "../sys/agi_file.h"
#include "agi_crc.h"
#include "../sys/mem_wrap.h"
#include "../sys/vstring.h"
#include "../sys/sys_dir.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);

#define DIR_SEP 1
#define DIR_COMB 2
#define DIR_AMIGA 3

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
	
	union
	{
		u32 dir_comb;
		
		struct
		{
			u32 log;
			u32 pic;
			u32 view;
			u32 snd;
		} dir;
	};

	u32 vol[16];
};
typedef struct agicrc_struct AGICRC;

/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


// generate a crc from a file
int file_crc_gen(u8 *file_name, u32 *crc32)
{
	u8 *buf;
	
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
	if (ptr != 0)
	{
		st_end = strstr(ptr, ",\0\n\r");
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
	
	// print object
	if (agicrc->object)
		printf("%s=0x%08X\n", "crc_object", agicrc->object);
	
	// print words
	if (agicrc->words)
		printf("%s=0x%08X\n", "crc_words", agicrc->words);
	
	// print vol[0] - vol[15]
	for (i=0; i<16; i++)
	{
		sprintf(key_vol, "crc_vol_%d", i);
		if (agicrc->vol[i])
			printf("%s=0x%08X\n", key_vol, agicrc->vol[i]);
	}
	
	switch (info->dir_type)
	{
		case DIR_COMB:
		case DIR_AMIGA:
			// print dir_comb
			if (agicrc->dir_comb)
				printf("%s=0x%08X\n", "crc_dir", agicrc->dir_comb);
			break;
		
		default:
		case DIR_SEP:
			// print dir.log
			if (agicrc->dir.log)
				printf("%s=0x%08X\n", "crc_dir_log", agicrc->dir.log);
			// print dir.pic
			if (agicrc->dir.pic)
				printf("%s=0x%08X\n", "crc_dir_pic", agicrc->dir.pic);
			// print dir.view
			if (agicrc->dir.view)
				printf("%s=0x%08X\n", "crc_dir_view", agicrc->dir.view);
			// print dir.snd
			if (agicrc->dir.snd)
				printf("%s=0x%08X\n", "crc_dir_snd", agicrc->dir.snd);
			break;
	}
	return 0;
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
	
	crc_list = strdupa(c_standard_crc_list);
	
	token = strtok_r(crc_list, ",", &running);
	while (token != 0)
	{
		if (ini_section(ini, token) == 0)
		{
			if (crc_compare(agicrc, info, ini)== 0)
				return c_standard_crc_list + (token - crc_list);
		}
		token = strtok_r(0, ",", &running);
	}

	return 0;	
}

// from the amiga gr vol.0
// replicated comp and uncomp.. lzw comp!!
// 12 34 00 2F 00 2F 00

// generate a name for the game info from standard.ini or just from other available resrouces
void gameinfo_namegen(GAMEINFO *info, INI *ini)
{
	u8 *name = 0;
	
	// standard separate
	if (info->standard != 0)
	{
		stand_sep(info->standard);
		
		// go to section
		ini_section(ini, info->standard);
		
		// read name
		name = ini_key(ini, "name");
	}
	
	if (name != 0)
	{
		strncpy(info->name, name, 80);
		if (strlen(name) >= 80)
			info->name[80] = 0;
		else
			info->name[strlen(name)] = 0;
	}
	else
		sprintf(info->name, "AGI version %d - %s", info->ver_type, info->file_id);
	
	stand_rejoin();
}


// create a new gameinfo struct for a given directory if a game exists in it.. 
// called on for each dir
GAMEINFO *gameinfo_new(u8 *dir, INI *ini)
{
	AGICRC agicrc;
	GAMEINFO info_new;
	
	memset(&info_new, 0, sizeof(GAMEINFO) );
	
	// change directory
	if (dir_change(dir)) return 0;	// fail
	
	if (dir_get_info(&agicrc, &info_new) != 0)
		return 0;	// fail
	
	if (c_nagi_crc_print)
		crc_print(&agicrc, &info_new);
	
	if (ini != 0)
		info_new.standard = crc_search(&agicrc, &info_new, ini);
	else
		info_new.standard = 0;
	
	//if (standard == 0)
	// don't worry... we'll just read the conf values.. don't have to dup strings all the time	
	
	// get directory
	info_new.dir = vstring_new(0, 100);
	vstring_getcwd(info_new.dir);
	
	// get name
	gameinfo_namegen(&info_new, ini);
	
	// restore old directory
	dir_nagi();
	
	// if everything ok
	// ADD TO LIST
	return memcpy(a_malloc(sizeof(GAMEINFO)), &info_new, sizeof(GAMEINFO));
}

GAMEINFO *gameinfo_head = 0;

// create a list of game infos starting from gameinfo_head from the dirlist in standard.ini
// search one level into it too if possible
int gi_list_init(INI *ini)
{
	GAMEINFO *info_cur=0, *info_prev=0;
	u8 *dir_list;
	u8 *token, *running;
	
	// for each dir
	dir_list = strdupa(c_standard_dir_list);
	token = strtok_r(dir_list, ";", &running);
	while (token != 0)
	{
		// for EACH DIRECTORY IN THAT DIR
		// {
			info_cur = gameinfo_new(token, ini);
			if (info_cur != 0)
			{
				if (gameinfo_head == 0)
					gameinfo_head = info_cur;
				if (info_prev != 0)
					info_prev->next = info_cur;
				info_prev = info_cur;
			}
		// }
		token = strtok_r(0, ";", &running);
	}
	
	if (gameinfo_head == 0)
		return 1;
	
	info_prev->next = 0;
	return 0;
}

// display nice menu of games and allow user to select game to play
GAMEINFO *gi_list_menu(void)
{
	// if number of games == 1
	// selectedgame = that one
	// else
	// display menu
	// selectedgame = menu()
	
	// init game stuff
	
	GAMEINFO *info_cur;
	info_cur = gameinfo_head;
	while (info_cur != 0)
	{
		printf("%s\n", info_cur->name);
		info_cur = info_cur->next;
	}
	
	return gameinfo_head;
}

// destory list
void gi_list_destroy(void)
{
	GAMEINFO *info_cur, *next;
	info_cur = gameinfo_head;
	while (info_cur != 0)
	{
		vstring_free(info_cur->dir);
		next = info_cur->next;
		a_free(info_cur);
		info_cur = next;
	}
}

u8 *window_caption = "NAGI";

void standard_init_ng(GAMEINFO *game, INI *ini)
{
	// set the defaults

	// res type.. use detected as default.
	switch(game->dir_type)
	{
		case DIR_COMB:
			config_game[4].i.def = RES_V3;
			break;
		case DIR_AMIGA:
			config_game[4].i.def = RES_V3_AMIGA;
			break;
		
		default:
		case DIR_SEP:
			if (game->file_id[0] == 0)
				config_game[4].i.def = RES_V2;
			else
				config_game[4].i.def = RES_V3_4;
			break;
	}
	
	switch (game->ver_type)
	{
		case 3:
			// version info
			config_game[1].s.def = "v3.002.149";
			// loop update
			config_game[3].i.def = L_FLAG;	//v3 loop default!!! FIXME
			break;
		case 2:
		default:
			// version info
			config_game[1].s.def = "v2.917";
			// loop update
			config_game[3].i.def = L_FOUR;	//v2 loop default!!!; FIXME
			break;
	}
	
	// change to appropriate section
	// use the v2 or v3 section defaults if available
	
	if (game->standard != 0)
	{
		stand_sep(game->standard);
		ini_section(ini, game->standard);
		stand_rejoin();
	}
	else
	{
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
	c_game_location = vstring_new(game->dir->data, 10);
	
	// need to setup game.id if you want to load up savegames before agi is init'd
	// standard.ini, or the file_id, or the default ""
	config_game[5].s.def = c_game_file_id;
	
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

void standard_select_ng(void)
{
	INI *ini_standard;
	GAMEINFO *game_selected;
	
	ini_standard = ini_open("standard.ini");

	// read in standard parameters
	config_load(config_standard, ini_standard);
	if (gi_list_init(ini_standard))
	{
		AGI_TRACE
		printf("no games detected\n");
		agi_exit();
	}
	game_selected = gi_list_menu();

	// init game
	standard_init_ng(game_selected, ini_standard);

	// close ini file
	ini_close(ini_standard);

	gi_list_destroy();
}

// TODO
// menu
// standard_separation DONE
// what happens if a dir doesn't exist on list?
// what happens if no game is set 
// what happens if there's no crc's on the list
// if standard.ini doesn't exist.. where's dir_list come from?
// what if ini_key is called without claling ini_section?
// instead of c_game_location.. use the dir_preset idea?  (nagi, user, game)