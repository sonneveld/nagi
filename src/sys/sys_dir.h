#ifndef sys_dir_h_file
#define sys_dir_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */

#define DIR_PRESET_ORIG 0
#define DIR_PRESET_NAGI 1
#define DIR_PRESET_HOME 2
#define DIR_PRESET_GAME 3
#define DIR_PRESET_LASTSAVE 4


int file_exists(u8 *f_name);
void vstring_getcwd(VSTRING *buff);
extern int dir_exists(u8 *d_name);


extern void dir_init(u8 *argv0);
extern void dir_shutdown(void);
extern void dir_preset_set(int preset_id, u8 *dir);
extern void dir_preset_set_cwd(int preset_id);
extern u8 *dir_preset_get(int preset_id);
extern int dir_preset_change(int preset_id);


#endif
