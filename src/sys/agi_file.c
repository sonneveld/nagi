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

#ifdef RAD_LINUX
#include <sys/types.h>
#include <dirent.h>
#include <string.h>
#else
#include <io.h>
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

// todo later on  .. ignore directories.. case.. 

u8 *find_first(FIND *token, u8 *name)
{
#ifndef RAD_LINUX
	token->handle = _findfirst(name, &token->winfile_info);
	if (token->handle != -1)
		return token->winfile_info.name;
	else 
		return 0;
#else
	token->dir = opendir(".");
	strcpy(token->name, name);

	while((token->file = readdir(token->dir))) {
		char *tok, *found;
		
		if(!strcmp(token->file->d_name, name)) 
			return token->file->d_name;
			
		tok = strtok(name, "*");
		if(tok == NULL) continue;
		found = strstr(token->file->d_name, tok);
		if(found == NULL) continue;

		while((tok = strtok(NULL, "*"))) {
			if(!(found = strstr(found, tok))) break;
		}
		if(found) return token->file->d_name;
	}
	
	return NULL;
#endif
}

u8 *find_next(FIND *token)
{
#ifndef RAD_LINUX
	if (_findnext(token->handle, &token->winfile_info) == 0)
		return token->winfile_info.name;
	else
		return 0;
#else
	while((token->file = readdir(token->dir))) {
		char *tok, *found;
		
		if(!strcmp(token->file->d_name, token->name)) 
			return token->file->d_name;

		tok = strtok(token->name, "*");
		if(tok == NULL) continue;
		found = strstr(token->file->d_name, tok);
		if(found == NULL) continue;

		while((tok = strtok(NULL, "*"))) {
			if(!(found = strstr(found, tok))) break;
		}
		if(found) return token->file->d_name;
	}
	
	return NULL;
#endif
}

void find_close(FIND *token)
{
#ifndef RAD_LINUX
	if (token->handle != -1)
		_findclose(token->handle);
#else
	if(token->dir) closedir(token->dir);
	token->dir = NULL;
#endif
}



FILE *fopen_nocase(u8 *name)
{
#ifdef RAD_LINUX
	DIR *dir;
	struct dirent *fileent;
	FILE *ret;
	
	dir = opendir(".");
	string_lower(name);

	while((fileent = readdir(dir))) {
		char *testname;
		
		testname = strdupa(fileent->d_name);
		string_lower(testname);
		
		if(!strcmp(testname, name)) {
			ret = fopen(fileent->d_name, "rb");
			closedir(dir);
			return ret;
		}
	}
	
	closedir(dir);
	return NULL;
#else
	return fopen(name, "rb");
#endif
}



// file writen
// file close
// file read
// file size

// open should use find_next 'n all that to find all lower and upper case of files

u32 file_buf_size;

u8 *file_to_buf(u8 *file_name)
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

