extern u8 *cmd_cancel_line(u8 *c);
extern u8 *cmd_echo_line(u8 *c);
extern u8 *cmd_prevent_input(u8 *c);
extern u8 *cmd_accept_input(u8 *c);
extern u8 *cmd_set_cursor_char(u8 *c);
extern u8 *cmd_open_dialogue(u8 *c);
extern u8 *cmd_close_dialogue(u8 *c);

extern void input_poll(void);
extern void input_put_char(u16 key_char);

extern void input_echo(void);
extern void input_edit_off(void);
extern void input_edit_on(void);
extern u16 input_edit_status(void);

extern void input_redraw(void);


extern u8 input_prev[42];
