/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
//#include "agi.h"
#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
//#include <errno.h>

/* OTHER headers	---	---	---	---	---	---	--- */
//#include "view/crap.h"
#include "mem_wrap.h"
#include "ini_config.h"

/* PROTOTYPES	---	---	---	---	---	---	--- */
//void test_function(void);



/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


INI *ini_open(u8 *ini_name);
void ini_close(INI *inistate);

INI *ini_open(u8 *ini_name)
{
	FILE *file_stream;
	INI *ini_new;
	
	// open file
	if ( (file_stream=fopen(ini_name, "rb")) == 0)
		return 0;
	
	// malloc an INI
	ini_new = (INI *)a_malloc(sizeof(INI));
	
	// get size
	fseek(file_stream, 0, SEEK_END);
	fgetpos(file_stream, &(ini_new->size));
	fseek(file_stream, 0, SEEK_SET);
	
	// read file
	ini_new->data = a_malloc(ini_new->size + 1);
	if ( fread(ini_new->data, sizeof(u8), ini_new->size, file_stream) !=(size_t)ini_new->size)
	{
		ini_close(ini_new);
		return 0;
	}
	
	// add a newline char at the end of the file so it can properly read the last key
	ini_new->size ++;
	ini_new->data[ini_new->size - 1] = '\n';
	
	// zero other pointers
	ini_new->last = ini_new->data + ini_new->size - 1;
	ini_new->cur_ptr = ini_new->data;
	ini_new->sect_ptr = 0;
	ini_new->isol_ptr = 0;

	// close ini file
	fclose(file_stream);
	
	//return ini
	return ini_new;
}

void ini_close(INI *ini)
{
	if (ini != 0)
	{
		if (ini->data != 0)
			a_free(ini->data);
		a_free(ini);
	}
}

// return 0 if ok
// return 1 if reach end of file or line
// starts from ini->cur_ptr
// don't trash ini_cur ptr if stuffed.. because we can still continue
int string_isolate(INI *ini, u8 ch)
{
	u8 *cur;
	
	cur = ini->cur_ptr;

	// check for eof, eoln
	while ( (cur <= ini->last) && (*cur!='\n') && (*cur!='\r') )
	{
		if (*cur == ch)
		{
			ini->isol_ptr = cur;
			ini->isol_ch = *cur;
			*cur = 0;	// null at the end of string
			return 0;  // alright!!  you better run string_rejoin later on matey
		}
		cur++;
	}
	
	return -1; // i failed.. nothing has been touched
}

// this means if you have a key on the last line.. it better have a eoln at the end
int line_isolate(INI *ini)
{
	u8 *cur;
	
	if (ini != 0)
	{
		cur = ini->cur_ptr;
	
		// check for eof
		while (cur <= ini->last) 
		{
			if ((*cur=='\n')||(*cur=='\r')) 	// eoln
			{
				ini->isol_ptr = cur;
				ini->isol_ch = *cur;
				*cur = 0;	// null at the end of string
				return 0;  // alright!!  you better run string_rejoin later on matey
			}
			cur++;
		}
	}
	
	return -1; // i failed.. nothing has been touched
}

void string_rejoin(INI *ini)
{
	if ((ini!=0) && (ini->isol_ptr != 0) )
	{
		*(ini->isol_ptr) = ini->isol_ch;
		ini->isol_ptr = 0;
	}
}

// we can trash this (cur_ptr) if we want.. we need ini->cur and we save ini->cur
// if we can't get to the next line.. we don't need the data.. we just finish
void line_next(INI *ini)
{
	u8 *cur;
	cur = ini->cur_ptr;
	
	// step through text
	while ( (cur <= ini->last) && (*cur!='\n') && (*cur!='\r') )
		cur++;
	
	// step through new line chars
	while ( (cur <= ini->last) && ((*cur=='\n')||(*cur=='\r')) )
		cur++;
	
	// set cur ptr
	if (cur <= ini->last)
		ini->cur_ptr = cur;
	else
		ini->cur_ptr = 0;
}


// starts from the start on the ini.
// if found sect_ptr = ptr
// else sect_ptr = 0;
// returns 0 if found.. so we don't have to call key_get
int ini_section(INI *ini, u8 *sect_name)
{
	if (ini == 0)
		return -1;
	
	string_rejoin(ini);		// in case a key was read
	
	ini->cur_ptr = ini->data;
	
	while (ini->cur_ptr != 0)
	{
		// find a line starting with [
		if ( *(ini->cur_ptr) == '[' )
		{
			ini->cur_ptr++;
			if (string_isolate(ini, ']') == 0)	// isolate the section name
			{
				// if isolated : compare and then rejoin the string
				if (strcmp(sect_name, ini->cur_ptr) == 0) 
				{
					// if equal then return with sect_ptr to next line
					string_rejoin(ini);
					line_next(ini);
					// if the line_next points to nothing.. it will be set to 0 so no further
					// processing will happen anyways
					break;				// RETURN
				}
				else
					string_rejoin(ini);
			}
		}
		
		// try the next line 
		line_next(ini);
	}
	
	ini->sect_ptr = ini->cur_ptr;	// sect_ptr is SET!
	return (ini->sect_ptr == 0);
}

u8 *ini_key(INI *ini, u8 *key_name)
{
	if (ini == 0)
		return 0;
	string_rejoin(ini);	// in case we're reading a new key
	
	// always search from start of section
	ini->cur_ptr = ini->sect_ptr;
	
	// keep on going until end of file or line starts with [ (assumed vaild sect name)
	while ( (ini->cur_ptr != 0) && (*(ini->cur_ptr) != '[') )
	{
		if (string_isolate(ini, '=') == 0)	// isolate the key name
		{
			// if isolated : compare and then rejoin the string
			if (strcmp(key_name, ini->cur_ptr) == 0) 
			{
				// if equal then return with ptr to key data
				string_rejoin(ini);
				
				// since we're here.. we KNOW there's a '=' 				
				ini->cur_ptr = strchr(ini->cur_ptr, '=') + 1;
				
				if (line_isolate(ini) == 0)
					return ini->cur_ptr;
				// else keep on looking for a line which does work.. which won't exist since
				// it's at the end of the file if this error occurs
			}
			else
				string_rejoin(ini);
		}

		// try the next line 
		line_next(ini);
	}

	if ( (ini->cur_ptr != 0) && (*(ini->cur_ptr) == '[') )
		ini->cur_ptr = 0;	// no use..  might as well eof
	return ini->cur_ptr;
}


// if sect_name == 0
// then use the current one
u8 *ini_recursive(INI *ini, u8 *sect_name, u8 *key_name)
{
	u8 *sect_orig;
	u8 *key_data;
	u8 *inherits;
	
	//remember old section
	sect_orig = ini->sect_ptr;
	
	if (sect_name != 0)
		ini_section(ini, sect_name);
	
	key_data = ini_key(ini, key_name);
	if (key_data == 0)
	{
		//read inherits key (have to copy it since the null char is removed 
		key_data = ini_key(ini, "inherits");
		if (key_data != 0)
		{
			inherits = alloca(strlen(key_data) + 1);
			strcpy(inherits, key_data);
			key_data = ini_recursive(ini, inherits, key_name);
		}
	}
	
	ini->sect_ptr = sect_orig;
	
	//return to original section
	return key_data;	
}





// FIXME
// it probably won't read key data that starts with [ (returning 0)  FIXED
// crashes if key data ends at EOF  FIXED
// read keys at the end of the file without a newline FIXED

// bugs fixed:
// no longer puts 0's everywhere at the end of lines
// looks cleaner / solid
// put some often used code into functions
// made it reentrant
// doesn't save the ini when you close it
// more than one ini can be opened at a time.

//  recursive inhertitance actually works more than one level
