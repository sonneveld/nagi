/* FUNCTION list 	---	---	---	---	---	---	---

*/

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


/* VARIABLES	---	---	---	---	---	---	--- */


VSTRING *directory_nagi  = 0;


// dir_nagi
// dir_home
// dir_game

/* CODE	---	---	---	---	---	---	---	--- */


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

int dir_change(u8 *d_name)
{
	if (directory_nagi == 0)
	{
		directory_nagi = vstring_new(0, 100);
		vstring_getcwd(directory_nagi);
	}
	if (d_name != 0)
		return chdir(d_name);
	else
		return 0;
}

int dir_nagi()
{
	if ( (directory_nagi != 0) && (directory_nagi->data != 0) ) 
		return chdir(directory_nagi->data);
	else
		return 0;	// assumed to be already in there
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

