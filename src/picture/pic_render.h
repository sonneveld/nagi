extern u8 pos_init_y;
extern u8 pos_init_x;
extern u8 pos_final_y;
extern u8 pos_final_x;

extern u8 col_even;
extern u8 col_odd;
extern u8 sbuff_drawmask;
extern u8 colour_pictpart;
extern u8 colour_pripart;
extern u8 *given_pic_data;

extern void render_pic(u8 overlay);
extern void render_overlay(void);
void pic_cmd_loop(void);
void enable_pic_draw(void);
void disable_pic_draw(void);
void enable_pri_draw(void);
void disable_pri_draw(void);
void plot_with_pen(void);
void read_pen_status(void);
void plot_with_pen_2(void);
void absolute_line(void);
void pic_fill(void);
int read_xy_pos(u8 *x, u8 *y);
int get_x_pos(u8 *x);
int get_y_pos(u8 *y);
void draw_line(void);
void draw_y_corner(void);
void draw_x_corner(void);
void relative_line(void);
void draw_corner(u8 type);

