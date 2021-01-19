struct inv_obj_struct
{
	u16 name;
	u8 location;
};
typedef struct inv_obj_struct INV_OBJ;

extern int object_file_load(void);
extern void object_file_unload(void);

extern u8 *cmd_get(u8 *c);
extern u8 *cmd_get_v(u8 *c);
extern u8 *cmd_drop(u8 *c);
extern u8 *cmd_put(u8 *c);
extern u8 *cmd_put_v(u8 *c);
extern u8 *cmd_get_room_v(u8 *c);

extern INV_OBJ *inv_obj_table;
extern int inv_obj_table_size;
extern u8 *inv_obj_string;
extern int inv_obj_string_size;
