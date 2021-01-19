struct view_node_struct
{
	//struct view_node_struct *next;	// 0
	u8 num;					// 2 view_num
	u8 *data;				// 3 data
};
typedef struct view_node_struct VIEW_NODE;


extern void view_list_init(void);
extern void view_list_free(void);
extern void view_list_new_room(void);
extern VIEW_NODE *view_find(u16 num);
extern u8 *cmd_load_view(u8 *c);
extern u8 *cmd_load_view_v(u8 *c);
extern VIEW_NODE *view_load(u16 num, u16 force_load);
extern u8 *cmd_set_view(u8 *c);
extern u8 *cmd_set_view_v(u8 *c);
extern void obj_view_set(VIEW *v, u16 num);

extern u8 *cmd_set_loop(u8 *c);
extern u8 *cmd_set_loop_v(u8 *c);
extern void obj_loop_set(VIEW *v, u16 loop_num);

extern u8 *cmd_set_cel(u8 *c);
extern u8 *cmd_set_cel_v(u8 *c);
extern void obj_cel_set(VIEW *v, u16 loop_num);


extern u8 *cmd_last_cel(u8 *c);
extern u8 *cmd_current_cel(u8 *c);
extern u8 *cmd_current_loop(u8 *c);
extern u8 *cmd_current_view(u8 *c);
extern u8 *cmd_number_of_loops(u8 *c);

extern u8 *cmd_discard_view(u8 *c);
extern u8 *cmd_discard_view_v(u8 *c);
extern void view_discard(u16 num);
