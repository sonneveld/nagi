#ifndef NAGI_BASE_H
#define NAGI_BASE_H

u8 *cmd_pause(u8 *logic_ptr);
u8 *cmd_quit(u8 *logic_ptr);

extern AGI_NO_RETURN void agi_exit(void);

#endif /* NAGI_BASE_H */
