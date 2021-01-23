#ifndef NAGI_SYS_AGI_FILE_H
#define NAGI_SYS_AGI_FILE_H

//~ RaDIaT1oN (2002-04-29):
//~ lowercase file search routines for linux

#define NAME_MAX 300

/* STRUCTURES	---	---	---	---	---	---	--- */

struct dir_list_struct;
	
/* VARIABLES	---	---	---	---	---	---	--- */
extern u32 file_buf_size;

/* FUNCTIONS	---	---	---	---	---	---	--- */

extern struct dir_list_struct *agi_open_cwd();
extern void agi_close_dir(struct dir_list_struct *);
extern const char *agi_read_dir(struct dir_list_struct *);

extern u8 *file_to_buf(const u8 *file_name);

extern FILE *fopen_nocase(const u8 *name);

#endif /* NAGI_SYS_AGI_FILE_H */
