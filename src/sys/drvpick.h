#ifndef NAGI_SYS_DRVPICK_H
#define NAGI_SYS_DRVPICK_H

/* STRUCTURES	---	---	---	---	---	---	--- */
struct drv_init_struct
{
	u8 *name;
	void (*expose_ptrs)(void *);
};
typedef struct drv_init_struct DRVINIT;
	
struct drv_init_state_struct
{
	DRVINIT *init_list;
	int size;
	void *ptr_list;
	
	DRVINIT *cur;
	DRVINIT *def;
};
typedef struct drv_init_state_struct DRVINITSTATE;
	
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */

extern int drvpick_first(DRVINITSTATE *, DRVINIT *init_list, int size, u8 *def, void *ptr_list);
extern int drvpick_next(DRVINITSTATE *);

#endif /* NAGI_SYS_DRVPICK_H */

