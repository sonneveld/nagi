#ifndef NAGI_CONFIG_H
#define NAGI_CONFIG_H

/* STRUCTURES	---	---	---	---	---	---	--- */

/* VARIABLES	---	---	---	---	---	---	--- */



/* FUNCTIONS	---	---	---	---	---	---	--- */

extern void config_load(CONF *config, INI *ini);
extern void config_print(CONF *config);
extern void config_unload(CONF *config);

#endif /* NAGI_CONFIG_H */
