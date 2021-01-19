#ifndef NAGI_SOUND_TONE_H
#define NAGI_SOUND_TONE_H

/* STRUCTURES	---	---	---	---	---	---	--- */

#define GEN_SILENCE 0
#define GEN_TONE 1
#define GEN_PERIOD 2
#define GEN_WHITE 3


struct tone_driver_struct
{
	u8 type;
	
	int (*ptr_init)(void);
	void (*ptr_shutdown)(void);
	int (*ptr_open)(int ch);
	void (*ptr_close)(int handle);
	void (*ptr_state_set)(int );
	int (*ptr_state_get)(void);
	void (*ptr_lock)(void);
	void (*ptr_unlock)(void);
};
typedef struct tone_driver_struct TONE_DRIVER;

/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */

extern int tone_init(void);
extern void tone_shutdown(void);
extern int tone_open(int ch);
extern void tone_close(int handle);
extern void tone_state_set(int);
extern int tone_state_get(void);
extern void tone_lock(void);
extern void tone_unlock(void);

#endif /* NAGI_SOUND_TONE_H */
