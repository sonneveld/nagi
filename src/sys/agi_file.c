/* FUNCTION list 	---	---	---	---	---	---	---

*/

//~ RaDIaT1oN (2002-04-29):
//~ lowercase file search routines for linux

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#ifdef _WIN32
#include <Windows.h>
#include <direct.h>
#else
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#endif

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "mem_wrap.h"
#include "agi_file.h"

#include "../ui/string.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */

// file system wrapper

// file first
// file next
// file open

// TODO: later on  .. ignore directories.. case.. 

#ifdef _WIN32

struct dir_list_struct
{
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	int first;
}; 

struct dir_list_struct *agi_open_cwd(void)
{
	struct dir_list_struct *d = malloc(sizeof(struct dir_list_struct));
	d->hFind = INVALID_HANDLE_VALUE;
	d->first = 1;

	d->hFind = FindFirstFile("*", &d->FindFileData);
	if (d->hFind == INVALID_HANDLE_VALUE) {
		free(d);
		d = 0;
	}
	return d;
}

void agi_close_dir(struct dir_list_struct *d)
{
	if (d == 0) { return; }
	if (d->hFind != INVALID_HANDLE_VALUE)
	{
		FindClose(d->hFind);
	}
	free(d);
}

const char *agi_read_dir(struct dir_list_struct *d)
{
	if (d == 0) { return 0; }
	if (d->hFind == INVALID_HANDLE_VALUE) { return 0; }
	if (d->first) {
		d->first = 0;
		return d->FindFileData.cFileName;
	}
	int res = FindNextFile(d->hFind, &d->FindFileData);
	if (res == 0) {
		return 0;
	}
	return d->FindFileData.cFileName;
}

FILE *fopen_nocase(const char *name)
{
	return fopen(name, "rb");
}

#else

struct dir_list_struct
{
	DIR *dir;	
	struct dirent *file;
}; 

struct dir_list_struct *agi_open_cwd(void)
{
	struct dir_list_struct *d = malloc(sizeof(struct dir_list_struct));
	d->dir = 0;
	d->file = 0;

	d->dir = opendir(".");
	if (d->dir == 0) {
		free(d);
		d = 0;
	}
	return d;
}

void agi_close_dir(struct dir_list_struct *d)
{
	if (d == 0) { return; }
	if (d->dir != 0) 
	{
		closedir(d->dir);
	}
	free(d);
}

const char *agi_read_dir(struct dir_list_struct *d)
{
	if (d == 0) { return 0; }
	if (d->dir == 0) { return 0; }
	d->file = readdir(d->dir);
	if (d->file == 0) { return 0; }
	return d->file->d_name;
}

FILE *fopen_nocase(const char *name)
{
	DIR *dir;
	struct dirent *fileent;
	FILE *ret;
	char *name_copy;
	
	dir = opendir(".");
	name_copy = strdupa(name);
	string_lower( name_copy );

	while((fileent = readdir(dir))) {
		char *testname;
		
		// TODO: strdupa uses alloca which is dangerous in a loop.
		testname = strdupa(fileent->d_name);
		string_lower(testname);
		
		if(!strcmp(testname, name_copy)) {
			ret = fopen(fileent->d_name, "rb");
			closedir(dir);
			return ret;
		}
	}
	
	closedir(dir);
	return NULL;
}

#endif



// file writen
// file close
// file read
// file size

// open should use find_next 'n all that to find all lower and upper case of files

u32 file_buf_size;

u8 *file_to_buf(const char *file_name)
{
	int file_size;
	FILE *file_stream;
	u8 *buf;
	
//	file_stream=fopen(file_name, "rb");
	file_stream=fopen_nocase(file_name);
	if (file_stream == 0)
		return 0;

	fseek(file_stream, 0, SEEK_END);
	//fgetpos(file_stream, &file_size);
	file_size = ftell(file_stream);
	fseek(file_stream, 0, SEEK_SET);
	file_buf_size = (u32)file_size;
	
	buf = (u8 *)a_malloc(file_buf_size);
	
	if ( fread(buf, sizeof(u8), file_buf_size, file_stream) != file_buf_size)
	{
		a_free(buf);
		return 0;
	}
		
	fclose(file_stream);
	return buf;
}


/*
// generate crc from first file that has that tail
// characters before tail can be no more than 5 chars
// no CASE RESTRICTIONS EITHER!!
int filetail_crc_gen(u8 *file_tail, u32 *crc32, u8 *file_id)
{
	DIR *dp;
	struct dirent *ep;
	struct stat fs;
	
	dp = opendir (".");
	if (dp != 0)
	{
		// find <FILEID>vol.0
		// there's not guarantee that <FILEID>dir exists
		
		while ( (ep=readdir (dp)) != 0)
			if (stat(ep->d_name, &fs) == 0)
				if (S_ISREG(fs.st_mode))
					// make sure it's got more than 3 chars
					if (strlen(ep->d_name) > strlen(file_tail))
						// check last 3 characters
						if (strcasecmp (file_tail, ep->d_name+strlen(ep->d_name)-strlen(file_tail)) == 0)
							if (strlen(ep->d_name) <= strlen(file_tail) + strlen(file_tail))
								if (file_crc_gen(ep->d_name, crc32) == 0)
								{
									
									// get file_id
									// .........
									return 0;
								}
	}

	return 1;
}
*/

