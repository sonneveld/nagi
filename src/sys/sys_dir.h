#ifndef sys_dir_h_file
#define sys_dir_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
int dir_change(u8 *d_name);
int dir_nagi(void);
int file_exists(u8 *f_name);
void vstring_getcwd(VSTRING *buff);
extern int dir_exists(u8 *d_name);

#endif