#ifndef file_h_file
#define file_h_file

//~ RaDIaT1oN (2002-04-29):
//~ lowercase file search routines for linux

#include <sys/types.h>
#include <dirent.h>

#define NAME_MAX 300

/* STRUCTURES	---	---	---	---	---	---	--- */
struct find_struct
{
	DIR *dir;	
	struct dirent *file;
	char name[NAME_MAX];
};
typedef struct find_struct FIND;
	
/* VARIABLES	---	---	---	---	---	---	--- */
extern u32 file_buf_size;

/* FUNCTIONS	---	---	---	---	---	---	--- */
extern u8 *file_to_buf(const u8 *file_name);

extern u8 *find_first(FIND *token, const u8 *name);
extern u8 *find_next(FIND *token);
extern void find_close(FIND *token);

extern FILE *fopen_nocase(const u8 *name);

#endif
