#ifndef NAGI_SYS_SYS_DIR_H
#define NAGI_SYS_SYS_DIR_H

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */

enum
{
	DIR_PRESET_ORIG,
	DIR_PRESET_NAGI,
	DIR_PRESET_HOME,
	DIR_PRESET_GAME,
	DIR_PRESET_LASTSAVE,
	DIR_PRESET_LEN
};

int file_exists(const char *f_name);
void vstring_getcwd(VSTRING *buff);
extern int dir_exists(const char *d_name);


extern void dir_init(int argc, char *argv[]);
extern void dir_shutdown(void);
extern void dir_preset_set(int preset_id, const char *dir);
extern void dir_preset_set_cwd(int preset_id);
extern const char *dir_preset_get(int preset_id);
extern int dir_preset_change(int preset_id);

// dump all current DIR presets for debugging
extern void dir_dump_preset_values(void);

#endif /* NAGI_SYS_SYS_DIR_H */
