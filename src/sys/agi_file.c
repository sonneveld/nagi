/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <io.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "mem_wrap.h"
#include "agi_file.h"

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
	token->handle = _findfirst(name, &token->winfile_info);
	if (token->handle != -1)
		return token->winfile_info.name;
	else 
		return 0;
}

u8 *find_next(FIND *token)
{
	if (_findnext(token->handle, &token->winfile_info) == 0)
		return token->winfile_info.name;
	else 
		return 0;
}

void find_close(FIND *token)
{
	if (token->handle != -1)
		_findclose(token->handle);
}




// file writen
// file close
// file read
// file size

// open should use find_next 'n all that to find all lower and upper case of files

u32 file_buf_size;

u8 *file_to_buf(u8 *file_name)
{
	fpos_t file_size;
	FILE *file_stream;
	u8 *buf;
	
	file_stream=fopen(file_name, "rb");
	if (file_stream == 0)
		return 0;

	fseek(file_stream, 0, SEEK_END);
	fgetpos(file_stream, &file_size);
	fseek(file_stream, 0, SEEK_SET);
	file_buf_size = (u32)file_size;
	
	buf = (u8 *)a_malloc(file_size);
	
	if ( fread(buf, sizeof(u8), file_size, file_stream) != file_size)
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