#ifndef NAGI_INITIALISE_H
#define NAGI_INITIALISE_H

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */

/* FUNCTIONS	---	---	---	---	---	---	--- */
// reads ini file and inits nagi
extern void nagi_init(void);
// after reading the version, inits this particular version of agi
extern void agi_init(void);
// for a new game.. can be called later by restart/restore
extern void game_init(void);
// refresh lists for new room
extern void room_init(void);

extern void agi_shutdown(void);
extern void nagi_shutdown(void);

#endif /* NAGI_INITIALISE_H */
