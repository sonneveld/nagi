#ifndef NAGI_SYS_SCRIPT_H
#define NAGI_SYS_SCRIPT_H

extern u8 *script_head;

extern void script_block(void);
extern void script_allow(void);
extern u8 *script_new(void);
extern void script_write(u16 var8, u16 vara);
extern void script_first(void);
extern u8 *script_get_next(void);
extern u8 *cmd_script_size(u8 *c);
extern u8 *cmd_unknown_171(u8 *c);
extern u8 *cmd_unknown_172(u8 *c);

#endif /* NAGI_SYS_SCRIPT_H */
