#ifndef SRC_SYS_INI_CONFIG
#define SRC_SYS_INI_CONFIG

/* STRUCTURES	---	---	---	---	---	---	--- */
struct ini_struct
{
	FILE *stream;
	int size;
	
	u8 *data;
	u8 *last;
	u8 *cur_ptr;
	
	u8 *sect_ptr;
	
	u8 *isol_ptr;
	u8 isol_ch;
};
typedef struct ini_struct INI;
	
/* VARIABLES	---	---	---	---	---	---	--- */

/* FUNCTIONS	---	---	---	---	---	---	--- */
extern INI *ini_open(const u8 *ini_name);
extern void ini_close(INI *ini_state);
extern int ini_section(INI *ini, const u8 *sect_name);
extern u8 *ini_key(INI *ini, const u8 *key_name);
extern u8 *ini_recursive(INI *ini, const u8 *sect_name, const u8 *key_name);


extern int ini_int(INI *ini, u8 *key, int def);
extern int ini_boolean(INI *ini, u8 *key, int def);
extern u8 *ini_string(INI *ini, u8 *key, u8 *def);

#endif /* SRC_SYS_INI_CONFIG */
