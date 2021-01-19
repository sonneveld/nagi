extern u8 *cmd_set(u8 *c);
extern u8 *cmd_reset(u8 *c);
extern u8 *cmd_toggle(u8 *c);
extern u8 *cmd_set_v(u8 *c);
extern u8 *cmd_reset_v(u8 *c);
extern u8 *cmd_toggle_v(u8 *c);

void flag_set(u8 flag_num);
void flag_reset(u8 flag_num);
void flag_toggle(u8 flag_num);
u8 flag_test(u8 flag_num);
void flags_clear(void);
