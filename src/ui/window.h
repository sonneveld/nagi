#ifndef NAGI_UI_WINDOW_H
#define NAGI_UI_WINDOW_H

extern void window_put_char(u16 given_char);
extern void goto_row_col(u16 row, u16 col);
extern void push_row_col(void);
extern void pop_row_col(void);
extern void window_portion_clear(u16 upper_row, u16 lower_row, u16 attrib);
extern void window_line_clear(u16 row, u16 attrib);
extern void window_clear(u16 upper_row, u16 upper_col, u16 lower_row, u16 lower_col, u16 attrib);
extern void window_scroll(u16 upper_row, u16 upper_col, u16 lower_row, u16 lower_col, u16 attrib);

extern u8 window_col;	// set by messagebox so it wrap inside the window
extern u8 window_row;
extern u16 text_shade;

#endif /* NAGI_UI_WINDOW_H */
