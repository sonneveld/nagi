#ifndef config_h_file
#define config_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */

/* VARIABLES	---	---	---	---	---	---	--- */



/* FUNCTIONS	---	---	---	---	---	---	--- */

extern void config_load(CONF *config, INI *ini);
extern void config_print(CONF *config);
extern void config_unload(CONF *config);


#endif