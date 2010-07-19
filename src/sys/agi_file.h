#ifndef file_h_file
#define file_h_file

//~ RaDIaT1oN (2002-04-29):
//~ lowercase file search routines for linux

#ifdef RAD_LINUX
#include <sys/types.h>
#include <dirent.h>
#else
#include <io.h>
#endif

#define NAME_MAX 300

/* STRUCTURES	---	---	---	---	---	---	--- */
struct find_struct
{
#ifndef RAD_LINUX
	struct _finddata_t winfile_info;
	int handle;
#else
	DIR *dir;	
	struct dirent *file;
	char name[NAME_MAX];
#endif
};
typedef struct find_struct FIND;
	
/* VARIABLES	---	---	---	---	---	---	--- */
extern u32 file_buf_size;

/* FUNCTIONS	---	---	---	---	---	---	--- */
extern u8 *file_to_buf(u8 *file_name);

extern u8 *find_first(FIND *token, u8 *name);
extern u8 *find_next(FIND *token);
extern void find_close(FIND *token);

extern FILE *fopen_nocase(u8 *name);

#endif
