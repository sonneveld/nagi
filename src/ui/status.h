
struct invent_item_struct
{
	u16 num;	// 0
	u8 *name;	// 2
	u16 row;	// 4
	u16 col;	// 6
};
typedef struct invent_item_struct INVENT;

extern u8 *cmd_status(u8 *c);
extern void inventory(void);
extern void invent_display(INVENT *invent, INVENT *last, INVENT *current);
extern INVENT *invent_key(INVENT *invent, INVENT *last, INVENT *current, u16 direction);
extern INVENT *invent_swap_colour(INVENT *item_cur, INVENT *item_new);

extern void status_line_write(void);
extern u8 *cmd_status_line_on(u8 *c);
extern u8 *cmd_status_line_off(u8 *c); 
