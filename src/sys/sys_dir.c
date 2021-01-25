/* FUNCTION list 	---	---	---	---	---	---	---

*/

//~ RaDIaT1oN (2002-04-29):
//~ include unistd.h


/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"
#include "vstring.h"
#include "sys_dir.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#else
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#endif

/* OTHER headers	---	---	---	---	---	---	--- */
#include "mem_wrap.h"


/* PROTOTYPES	---	---	---	---	---	---	--- */
void dir_init(int argc, char *argv[]);
void dir_shutdown(void);
void dir_preset_set(int preset_id, u8 *dir);
void dir_preset_set_cwd(int preset_id);
u8 *dir_preset_get(int preset_id);
int dir_preset_change(int preset_id);

/* VARIABLES	---	---	---	---	---	---	--- */

static const char *dir_preset_names[DIR_PRESET_LEN] = {
	"DIR_PRESET_ORIG",
	"DIR_PRESET_NAGI",
	"DIR_PRESET_HOME",
	"DIR_PRESET_GAME",
	"DIR_PRESET_LASTSAVE"
};

static VSTRING *dir_preset[DIR_PRESET_LEN];


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


// Configure preset paths
// DIR_PRESET_ORIG should point towards the current directory (or first cmdline arg)
// DIR_PRESET_NAGI should point towards where the nagi binary is stored, to find config files.
// NOTE: these paths have to be absolute for now.
void dir_init(int argc, char *argv[])
{
	memset(dir_preset, 0, sizeof(dir_preset));

	// the location of nagi binary and data
	char *base_path = SDL_GetBasePath();
	dir_preset_set(DIR_PRESET_NAGI, base_path);
	SDL_free(base_path);
	
	// if directory is passed in, pretend nagi was run from that directory
	if (argc >= 2) {
		int chdirres = chdir(argv[1]);
		if (chdirres != 0) {
			printf("WARNING: Could not change directory to %s\n", argv[1]);
		}
	}
	dir_preset_set_cwd(DIR_PRESET_ORIG);
	
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
	int result = -1;
	if (dir_preset[preset_id] != 0)
		if (dir_preset[preset_id]->data != 0)
			result = chdir(dir_preset[preset_id]->data);

	if (result == -1) {
		printf("WARNING: Could not change directory to %s\n", dir_preset[preset_id]->data);
	}

	return result;
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


// case insenstive file existence check 
int file_exists(u8 *f_name)
{
#ifdef _WIN32
	DWORD res = GetFileAttributes(f_name);
	return (res != INVALID_FILE_ATTRIBUTES) && ((res & FILE_ATTRIBUTE_DIRECTORY) == 0);
#else
	DIR *dp;
	struct dirent *ep;
	struct stat fs;
	int result = 0;
	
	dp = opendir (".");
	if (!dp) { return 0; }
	
	for (;;) {
		ep = readdir(dp);
		if (!ep) { break; }

		if (!S_ISREG(fs.st_mode)) { continue; }
		if (strcasecmp(ep->d_name, f_name) != 0) { continue; }

		result = 1;
		break;
	}

	(void) closedir (dp);
	return result;
#endif
}


// case insenstive directory existence check 
int dir_exists(u8 *d_name)
{
#ifdef _WIN32
	DWORD res = GetFileAttributes(d_name);
	return (res != INVALID_FILE_ATTRIBUTES) && (res & FILE_ATTRIBUTE_DIRECTORY);
#else	
	DIR *dp;
	dp = opendir(d_name);
	if (!dp) { return 0; }
	(void) closedir(dp);
	return 1;
#endif
}


void dir_dump_preset_values(void)
{
	for (int i = 0; i < DIR_PRESET_LEN; i++) {
		char *value = dir_preset_get(i);
		if (value == 0) {
			value = "not set";
		}
		printf("%s=%d: %s\n", dir_preset_names[i], i, value);
	}
}
