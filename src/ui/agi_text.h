#ifndef NAGI_UI_AGI_TEXT_H
#define NAGI_UI_AGI_TEXT_H

extern u8 *cmd_text_screen(u8 *c);
extern u8 *cmd_graphics(u8 *c);
extern u8 *cmd_clear_lines(u8 *c);
extern u8 *cmd_clear_text_rect(u8 *c);
extern u8 *cmd_set_text_attribute(u8 *c);
extern u8 *cmd_config_screen(u8 *c);
extern u8 *cmd_toggle_monitor(u8 *c);

extern void text_colour(u16 fg, u16 bg);
extern u16 do_nothing(u16 nothin_but_crap);
extern u16 calc_text_bg(u16 colour);
extern void screen_redraw(void);
extern void text_attrib_push(void);
extern void text_attrib_pop(void);

#endif /* NAGI_UI_AGI_TEXT_H */
