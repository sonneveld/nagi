


struct menu_item_struct
{
	struct menu_item_struct *next;	// 0-1	next
	struct menu_item_struct *prev; 	// 2-3 prev
	u8 *name;				// 4-5
	u16 row; 				// 6-7
	u16 col;				// 8-9
	u16 status;				// a-b	// 1 = enable 0 = disable
	
	u16 control;			// c-d
};

typedef struct menu_item_struct MENU_ITEM;

struct menu_struct
{
	struct menu_struct *next;	// 0-1 next
	struct menu_struct *prev;	// 2-3 prev
	u8 *name;				// 0x4-5
	u16 row;				// 6-7	// guesing it's row
	u16 col;				// 0x8-9
	u16 status;				// 0xA-b  -  0 = no items.. empty
	
	struct menu_item_struct *head;	// 0xC-0xD - menu_item head
	struct menu_item_struct *cur;	// 0xE-0xF - menu_item current
	u16 size;				// 0x10-0x11 - number of items
};

typedef struct menu_struct MENU;

extern u16 menu_next_input;

extern u8 *cmd_set_menu(u8 *c);
extern u8 *cmd_set_menu_item(u8 *c);
extern u8 *cmd_submit_menu(u8 *c);
extern u8 *cmd_enable_item(u8 *c);
extern void menu_enable_all(void);
extern u8 *cmd_disable_item(u8 *c);
extern void menu_item_set(u16 cont, u16 new_state);
extern u8 *cmd_menu_input(u8 *c);
extern u8 *cmd_unknown_177(u8 *c);

extern void menu_input(void);
extern void menu_draw(MENU *var8);
extern void menu_clear(MENU *var8, MENU_ITEM *vara);

extern void menu_item_name_invert(MENU_ITEM *mi);
extern void menu_name_invert(MENU *m);
extern void menu_item_name(MENU_ITEM *mi);
extern void menu_name(MENU *m);

extern void menu_calc_size(MENU *var8);


extern void r_menu930a(void);