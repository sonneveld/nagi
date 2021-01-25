#ifndef NAGI_SYS_INI_CONFIG_H
#define NAGI_SYS_INI_CONFIG_H

/* STRUCTURES	---	---	---	---	---	---	--- */
struct ini_struct
{
	FILE *stream;
	int size;
	
	char *data;
	char *last;
	char *cur_ptr;
	
	char *sect_ptr;
	
	char *isol_ptr;
	char isol_ch;
};
typedef struct ini_struct INI;
	
/* VARIABLES	---	---	---	---	---	---	--- */

/* FUNCTIONS	---	---	---	---	---	---	--- */
extern INI *ini_open(const char *ini_name);
extern void ini_close(INI *ini_state);
extern int ini_section(INI *ini, const char *sect_name);
extern const char *ini_key(INI *ini, const char *key_name);
extern const char *ini_recursive(INI *ini, const char *sect_name, const char *key_name);


extern int ini_int(INI *ini, const char *key, int def);
extern int ini_boolean(INI *ini, const char *key, int def);
extern const char *ini_string(INI *ini, const char *key, const char *def);

#endif /* NAGI_SYS_INI_CONFIG_H */
