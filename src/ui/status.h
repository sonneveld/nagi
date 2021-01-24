#ifndef NAGI_UI_STATUS_H
#define NAGI_UI_STATUS_H

extern u8 *cmd_status(u8 *c);

extern void status_line_write(void);
extern u8 *cmd_status_line_on(u8 *c);
extern u8 *cmd_status_line_off(u8 *c); 

#endif /* NAGI_UI_STATUS_H */
