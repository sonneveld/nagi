extern u8 *cmd_print(u8 *c);
extern u8 *cmd_print_v(u8 *c);
extern u8 *cmd_print_at(u8 *c);
extern u8 *cmd_print_at_v(u8 *c);
extern u8 *cmd_close_window(u8 *c);
extern u8 *cmd_display(u8 *c);
extern u8 *cmd_display_v(u8 *c);

extern u8 *print_at(u16 msg_num, u8 *c);
extern u16 message_box(u8 *var8);
extern void message_box_draw(u8 *str, u16 row, u16 w, u16 toggle);
extern u8 *str_wordwrap(u8 *msg, u8 *str, u16 w);
extern u8 *r_display1f93(u8 *given_source, u8 *given_msg);
extern u8 *logic_msg(u16 msg_num);
extern u8 *str_to_int_ptr(u8 *s, u16 *num);
extern void display_new_line(void);

struct msgstate_struct
{
	// writable settings --------------
	
	// wanted width and position.
	u16 wanted_width;
	TPOS wanted_pos;
	// related to hgc.. not really useful I don't think.
	u16 dialogue_open;
	// '\' character.. if this exists then the next character is automatically put in.
	// it probably changes since the directories would be a bugger to print in save/restore functions
	u8 newline_char;		// 0x40 or 0x5c
	
	// internal settings ---------------------
	
	// visible == 1 window is displayed over picture buffer
	// visible == 0 no window is displayed
	u16 active;
	
	// text in messagebox
	TPOS tpos;		// top left pos of text to display
	TPOS tpos_edge;	// lower right pos of text to display
	SIZE tsize;		// height and width of the text displayed
	
	// background graphic (white with red border)
	POS bgpos;		// lower left position of bg
	SIZE bgsize;	// size of bg
};
typedef struct msgstate_struct MSGSTATE;

#define HEIGHT_MAX 20
#define LINE_SIZE 8

extern u16 dialogue_open;
//extern u8 newline_char ;	// 0x40 or 0x5c
extern TPOS msg_tpos;
extern TPOS msg_tpos_edge;

extern MSGSTATE msgstate;
