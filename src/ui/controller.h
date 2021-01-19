
#define CONTROL_MAX 50
extern u8 control_state[CONTROL_MAX];


extern void control_state_clear(void);
extern u8 *cmd_set_key(u8 *c);
extern AGI_EVENT *control_key_map(AGI_EVENT *agi_event);
