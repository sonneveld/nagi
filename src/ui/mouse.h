#ifndef mouse_h_file
#define mouse_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
extern void mstack_push(int button, int x, int y);
extern u8 *cmd_brian_poll_mouse(u8 *c);
#endif