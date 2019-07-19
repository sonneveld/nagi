/* FUNCTION list 	---	---	---	---	---	---	---

*/

//~ RaDIaT1oN (2002-04-29):
//~ include unistd.h

#include <unistd.h>


/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"
#include "vstring.h"
#include "sys_dir.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <string.h>

/* OTHER headers	---	---	---	---	---	---	--- */
#include "mem_wrap.h"


/* PROTOTYPES	---	---	---	---	---	---	--- */
void dir_init(u8 *argv0);
void dir_shutdown(void);
void dir_preset_set(int preset_id, u8 *dir);
void dir_preset_set_cwd(int preset_id);
u8 *dir_preset_get(int preset_id);
int dir_preset_change(int preset_id);

/* VARIABLES	---	---	---	---	---	---	--- */


//VSTRING *directory_nagi  = 0;


VSTRING *dir_preset[10];


// home
/*
%HOME
user profile
win dir
nagi dir
agrc[0]
game base dir
variable in ini
application data

things to keep in mind
fan game authors (data always changing)
different versions with same game id
crcs of all files.. some might not actually be with that game (ie, a game copied on top of an other one)


use crc, standard name, game_id, standard.ini option for game



BLAH/.nagi/gameid/
blah/.nagi/agi-0xDA01FEAB/

*/



/* CODE	---	---	---	---	---	---	---	--- */

// pass it argv0 from the cmd line

void dir_init(u8 *argv0)
{
	memset(dir_preset, 0, sizeof(dir_preset));
	
	// set orig directory
	dir_preset_set_cwd(DIR_PRESET_ORIG);
	
	// set nagi main
	if ((argv0 != 0) && (argv0[0] != 0))
	{
		u8 *path_cpy;
		u8 *end;
		
		path_cpy = strdupa(argv0);
		// strip file name
		
		// '/' if win32
		// '\' if anything else
		end = path_cpy + strlen(path_cpy) - 1;
		while ((end >= path_cpy) && (end != 0))
		{
			if ( (*end == '\\') || (*end == '/') )
			{
				*end = 0;
				break;
			}
			else 
				end--;
		}

		// set preset
		dir_preset_set(DIR_PRESET_NAGI, path_cpy);
	}
	else
	{
		// what else can you do?
		dir_preset_set_cwd(DIR_PRESET_NAGI);
	}
	
	// set nagi home
	// find profile/home directory
	// create .nagi/ if necessary
	// pointer home to that
}

void dir_shutdown()
{
	int i;
	
	i = sizeof(dir_preset) / sizeof(VSTRING *);
	while (i)
	{
		i--;
		if (dir_preset[i] != 0)
		{
			vstring_free(dir_preset[i]);
			dir_preset[i] = 0;
		}
	}
	
}

void dir_preset_set(int preset_id, u8 *dir)
{
	if (dir_preset[preset_id] == 0)
		dir_preset[preset_id] = vstring_new(dir, 10);
	else
		vstring_set_text(dir_preset[preset_id], dir);
}

void dir_preset_set_cwd(int preset_id)
{
	if (dir_preset[preset_id] == 0)
		dir_preset[preset_id] = vstring_new(0, 50);
	vstring_getcwd(dir_preset[preset_id]);
}

u8 *dir_preset_get(int preset_id)
{
	if (dir_preset[preset_id] != 0)
		if (dir_preset[preset_id]->data != 0)
			return dir_preset[preset_id]->data;

	return 0;}

int dir_preset_change(int preset_id)
{
	if (dir_preset[preset_id] != 0)
		if (dir_preset[preset_id]->data != 0)
			return chdir(dir_preset[preset_id]->data);
	return -1;
}





void vstring_getcwd(VSTRING *buff)
{
	u8 *value;
	
	for(;;)
	{
		value = getcwd(buff->data, buff->size);
		if (value != 0)
			break;
		vstring_set_size(buff, buff->size * 2);
	}
}


// return 1 if exists
int file_exists(u8 *f_name)
{
	DIR *dp;
	struct dirent *ep;
	struct stat fs;
	
	dp = opendir (".");
	if (dp != 0)
	{
		while ( (ep=readdir (dp)) != 0)
			if (stat(ep->d_name, &fs) == 0)
				if (S_ISREG(fs.st_mode))
				{
					if (strcasecmp(ep->d_name, f_name) == 0)
					{
						(void) closedir (dp);
						return 1;
					}
				}
		(void) closedir (dp);
	}
	
	return 0;
}


int dir_exists(u8 *d_name)
{
	DIR *dp;
	
	dp = opendir(d_name);
	if (dp != 0)
	{
		(void) closedir(dp);
		return 1;
	}
	
	return 0;
}

