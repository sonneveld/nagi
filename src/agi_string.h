



extern u8 *cmd_get_string(u8 *c);
extern u8 *cmd_set_string(u8 *c);
extern u8 *cmd_word_to_string(u8 *c);
extern u8 string_edit(u8 *str, u16 str_size, u16 disp_size);

extern u8 *cmd_set_game_id(u8 *c);

extern u16 agi_string_compare(u16 var8, u16 vara);
extern void agi_string_clean(u16 str_agi, u8 *str_buff);

extern u8 *cmd_get_num(u8 *c);
