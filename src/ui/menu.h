#ifndef NAGI_UI_MENU_H
#define NAGI_UI_MENU_H

// Used to tell poll_input whether it needs to pass events to the menu subsystem
extern int get_menu_requires_input_events(void);

extern u8 *cmd_set_menu(u8 *c);
extern u8 *cmd_set_menu_item(u8 *c);
extern u8 *cmd_submit_menu(u8 *c);
extern u8 *cmd_enable_item(u8 *c);
extern void menu_enable_all(void);
extern u8 *cmd_disable_item(u8 *c);
extern u8 *cmd_menu_input(u8 *c);
extern u8 *cmd_unknown_177(u8 *c);

extern void menu_input(void);

#endif /* NAGI_UI_MENU_H */
