extern u8 cmd_equal_n(void);
extern u8 cmd_equal_v(void);
extern u8 cmd_less_n(void);
extern u8 cmd_less_v(void);
extern u8 cmd_greater_n(void);
extern u8 cmd_greater_v(void);
extern u8 cmd_isset(void);
extern u8 cmd_isset_v(void);
extern u8 cmd_has(void);

extern u8 cmd_posn(void);
extern u8 cmd_center_posn(void);
extern u8 cmd_right_posn(void);
extern u8 cmd_obj_in_box(void);
u8 is_obj_inside(u16 left, u16 right, u16 y);
extern u8 cmd_controller(void);
extern u8 cmd_obj_in_room(void);
extern u8 cmd_said(void);
extern u8 cmd_have_key(void);
// compare strings
extern u8 cmd_compare_strings(void);