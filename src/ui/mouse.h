#ifndef NAGI_UI_MOUSE_H
#define NAGI_UI_MOUSE_H

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
extern void mstack_push(int button, int x, int y);
extern u8 *cmd_brian_poll_mouse(u8 *c);

extern void mouse_event_handle(AGI_EVENT *event);
extern void mouse_init(void);
extern void mouse_shutdown(void);

#endif /* NAGI_UI_MOUSE_H */
