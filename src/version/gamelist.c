/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "../agi.h"
#include "gamelist.h"



/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>


/* OTHER headers	---	---	---	---	---	---	--- */
#include "../sys/vstring.h"
#include "../sys/sys_dir.h"
#include "../sys/ini_config.h"
#include "../sys/mem_wrap.h"
#include "agi_crc.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
GAMEINFO *game_list_new(u8 *dir_name);
void game_list_delete(GAMEINFO *g_info);
u8 *crc_check(u8 *section);
void res_detect(GAMEINFO *g_info);

/* VARIABLES	---	---	---	---	---	---	--- */
GAMEINFO *g_head = 0;

/* CODE	---	---	---	---	---	---	---	--- */

void game_list_delete(GAMEINFO *g_info)
{
	if (g_info != 0)
	{
		if (g_info->name != 0)
		{
			free(g_info->name);
			g_info->name=0;
		}
		if (g_info->id != 0)
		{
			free(g_info->id);
			g_info->id=0;
		}	
		if (g_info->standard != 0)
		{
			free(g_info->standard);
			g_info->standard=0;
		}
		if (g_info->location != 0)
		{
			free(g_info->location);
			g_info->location=0;
		}
		free (g_info);
	}
}

GAMEINFO *game_list_new(u8 *search_dir)
{
	u8 *key_value;
	u8 *crc_list = 0;
	u8 *token;
	GAMEINFO *g_info;
	
	// allocate memory for the game_info struct
	g_info = a_malloc(sizeof(GAMEINFO));
	memset(g_info, 0, sizeof(GAMEINFO));
	
	// store location of game
	g_info->location = strdup(search_dir);

	// compare against known crc's and make a match
	#warning test if standard.ini does not exist
	key_value=ini_read("standard", "crc_list");
	
	dir_change(g_info->location);
	if (key_value != 0)
	{
		if ((crc_list=strdup(key_value)) != 0)
		{
			token = strtok(crc_list, ",");
			while ((token!=0) && (g_info->standard==0))
			{
				g_info->standard = crc_check(token);
				token = strtok(0, ",");
			}
			free(crc_list);
			crc_list = 0;
		}
	}
	
	// if match found, then store it's resource type
	// duplicate the standard name
	// set the match type = crc
	if (g_info->standard != 0)
	{
		g_info->standard = strdup(g_info->standard);
		g_info->format = ini_int(g_info->standard, 0, "res_type", 0);
		g_info->crc_match = 1;
	}
	
	// if not restype found (because of no crc match) then guess
	if (g_info->format == 0)
	{
		printf("Guessing resource type: ");
		res_detect(g_info);
	}
	
	// if we haven't found a crc match, use the guessed res_type to use a default standard
	if (g_info->standard == 0)
	{
		switch(g_info->format)
		{
			case RES_V3:
			case RES_V3_4:
				key_value = ini_read("standard", "v3_default");
				if (key_value != 0)
					g_info->standard = strdup(key_value);
				break;
			case RES_V2:
				key_value = ini_read("standard", "v2_default");
				if (key_value != 0)
					g_info->standard = strdup(key_value);
				break;
		}
	}
	
	// if we've found a standard, read it's name
	if (g_info->standard != 0)
	{
		// NAME
		key_value = ini_read(g_info->standard, "name");
		if (key_value != 0)
			g_info->name = strdup(key_value);
	}
	
	dir_nagi();	// back to the original nagi directory	
	return g_info;
}

// assumes an ini file is open
// checks the current working directory and compares the section's crc with this directory's crc's
u8 *crc_check(u8 *section)
{
	u8 key_name[10];
	u8 *key_value;
	u8 *comma;
	u32 crc;
	int i;
	
	// 99 should be enough. considering there's 15 volumes max for v3 games
	for (i=0; i<=99; i++)
	{
		sprintf(key_name, "crc%d", i);
		key_value = ini_read(section, key_name);
		if (key_value == 0)
		{
			return section;	 // FOUND
		}
		
		// duplicated because we're modifying it
		key_value = strdup(key_value);
		
		comma = strchr(key_value, ',');
		if (( (comma+1) >= (key_value+strlen(key_value))) && (comma != 0))
		{
			printf("bad crc entry in ini file\n");
			free(key_value);
			return 0;	// NOT FOUND
		}

		crc = strtoul ((comma+1), 0, 16);
		*comma = 0;
		if (file_crc_compare(key_value, crc)==0)
		{
			free(key_value);
			return 0;	// NOT FOUND
		}
		free(key_value);
	}
	return section;  // too many crc's but they all checked out
}

#define F_VOL0 0x01
#define F_DIR_LOG 0x02
#define F_DIR_PIC 0x04
#define F_DIR_VIEW 0x8
#define F_DIR_SND 0x10
#define VER2 0x1F

// detect the res type
void res_detect(GAMEINFO *g_info)
{
	DIR *dp;
	struct dirent *ep;
	struct stat fs;
	u8 *id = 0;
	u8 file_state = 0;
	u8 vol_0_name[50];
	
	g_info->format = 0;
	
	// check "words.tok" exists
	if (file_exists("words.tok") == 0)
	{
		printf("none detected.\n");
		return;
	}
	// check "object" exists
	if (file_exists("object") == 0)
	{
		printf("none detected.\n");
		return;
	}
	
	dp = opendir (".");
	if (dp != 0)
	{
		// find game_id
		// find <GAMEID>dir and see if <GAMEID>vol.0 exists
		
		while ( (ep=readdir (dp)) != 0)
			if (stat(ep->d_name, &fs) == 0)
				if (S_ISREG(fs.st_mode))
					// make sure it's got more than 3 chars
					if (strlen(ep->d_name) > 3)
						// check last 3 characters
						if (strcasecmp ("dir", ep->d_name+strlen(ep->d_name)-3) == 0)
						{	
							// if 'dir' then search for 'dir'vol.0
							id = strdup(ep->d_name);
							if (id != 0)
							{
								id[strlen(id)-3] = 0;
								if (strlen(id) < 40)
								{
									sprintf(vol_0_name, "%svol.0", id);
									// if exist then store game id.. break;
									if (file_exists(vol_0_name) != 0)
									{
										(void) closedir (dp);
										g_info->format = 3;
										g_info->id = id;
										printf("version 3.\n");
										return;	// 1dir.. lzw comp assumed
									}
									free(id);
									id = 0;
								}
							}
						}
	}
	// go through directory contents
		//logdir, picdir, snddir, viewdir,. vol.0

	rewinddir(dp);	// start again
	while ( (ep=readdir (dp)) != 0)
		if (stat(ep->d_name, &fs) == 0)
			if (S_ISREG(fs.st_mode))
			{
				if (strcasecmp(ep->d_name, "logdir")==0)
					file_state |= F_DIR_LOG;
				else if (strcasecmp(ep->d_name, "picdir")==0)
					file_state |= F_DIR_PIC;
				else if (strcasecmp(ep->d_name, "viewdir")==0)
					file_state |= F_DIR_VIEW;
				else if (strcasecmp(ep->d_name, "snddir")==0)
					file_state |= F_DIR_SND;
				else if (strcasecmp(ep->d_name, "vol.0")==0)
					file_state |= F_VOL0;
			}		
					
	(void) closedir (dp);
			
	if ((file_state & VER2) == VER2)
	{
		g_info->format = 1;
		printf("version 2.\n");
	}
	else 	printf("none detected.\n");
}



GAMEINFO *game_list_force(u8 *f_standard)
{
	u8 *key_value;
	GAMEINFO *g_info;
	
	// allocate memory for the game_info struct
	g_info = a_malloc(sizeof(GAMEINFO));
	memset(g_info, 0, sizeof(GAMEINFO));
	
	// store location of game
	g_info->location = strdup(".");
	
	// if match found, then store it's resource type
	// duplicate the standard name
	// set the match type = crc
	g_info->standard = strdup(f_standard);
	g_info->format = ini_int(g_info->standard, 0, "res_type", 0);
	g_info->crc_match = 1;
	
	// if not restype found (because of no crc match) then guess
	if (g_info->format == 0)
	{
		printf("Guessing resource type: ");
		res_detect(g_info);
	}

	// if we've found a standard, read it's name
	// NAME
	key_value = ini_read(g_info->standard, "name");
	if (key_value != 0)
		g_info->name = strdup(key_value);

	return g_info;
}