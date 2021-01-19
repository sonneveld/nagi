struct logic_struct
{
	struct logic_struct *next;	// 0
	u8 num;				// 2
	u8 msg_total;			// 3
	u8 *data;				// pointer to free
	u8 *code;				// 4
	u8 *scan_start;			// 6
	u8 *msg;				// 8
};
// may need to check offsets

typedef struct logic_struct LOGIC;
	
extern LOGIC *logic_cur;
extern u16 scan_start_list[60];

extern void logic_list_init(void);
extern void logic_list_new_room(void);
extern LOGIC *logic_list_find(u16 logic_num);
extern u8 *cmd_load_logics(u8 *c);
extern u8 *cmd_load_logics_v(u8 *c);
extern void logic_load(u16 logic_num);
extern LOGIC *logic_load_2(u16 logic_num);

extern u8 *cmd_call(u8 *c);
extern u8 *cmd_call_v(u8 *c);
extern u8 *logic_call(u16 logic_num);
extern u8 *cmd_set_scan_start(u8 *c);
extern u8 *cmd_reset_scan_start(u8 *c);
extern u16 logic_save_scan_start(void);
extern void logic_restore_scan_start(LOGIC *log);
