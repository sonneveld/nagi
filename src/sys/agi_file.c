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
	_findclose(token->handle);
}




// file writen
// file close
// file read
// file size

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