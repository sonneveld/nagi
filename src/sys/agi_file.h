#ifndef file_h_file
#define file_h_file


#include <io.h>

/* STRUCTURES	---	---	---	---	---	---	--- */
struct find_struct
{
	struct _finddata_t winfile_info;
	int handle;
};
typedef struct find_struct FIND;
	
/* VARIABLES	---	---	---	---	---	---	--- */
extern u32 file_buf_size;

/* FUNCTIONS	---	---	---	---	---	---	--- */
extern u8 *file_to_buf(u8 *file_name);

extern u8 *find_first(FIND *token, u8 *name);
extern u8 *find_next(FIND *token);
extern void find_close(FIND *token);


#endif