#ifndef NAGI_STATE_IO_H
#define NAGI_STATE_IO_H

extern u8 *cmd_restart_game(u8 *c);
extern u8 *cmd_restore_game(u8 *c);
extern u16 state_read(FILE *data_stream, void *data_alloc);
extern u16 state_read_view(FILE *data_stream, VIEW *head, VIEW*tail);
extern u8 *cmd_unknown_170(u8 *c);
extern u8 *cmd_save_game(u8 *c);
extern u16 state_write(FILE *stream, void *write_data, u16 write_size);
extern void state_reload(void);

extern u8 state_name_auto[0x32];

#endif /* NAGI_STATE_IO_H */
