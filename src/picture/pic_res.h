struct pic_struct
{
	struct pic_struct *next;
	u8 num;
	u8 *data;
};

typedef struct pic_struct PIC;

extern u8 *given_pic_data;
extern u16 pic_visible;


extern void pic_list_init(void);
extern void pic_list_new_room(void);
extern PIC *pic_find(u16 pic_num);
extern u8 *cmd_load_pic(u8 *c);
extern PIC *pic_load(u16 pic_num);
extern u8 *cmd_draw_pic(u8 *c);
extern void pic_draw(u16 pic_num);
extern u8 *cmd_overlay_pic(u8 *c);
extern void pic_overlay(u16 pic_num);
extern u8 *cmd_show_pic(u8 *c);
extern u8 *cmd_discard_pic(u8 *c);
extern void pic_discard(u16 pic_num);
extern u8 *cmd_set_upper_left(u8 *c);