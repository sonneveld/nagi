#ifndef NAGI_VIEW_OBJ_MOTION_CMD_H
#define NAGI_VIEW_OBJ_MOTION_CMD_H

extern u8 *cmd_move_obj(u8 *c);
extern u8 *cmd_move_obj_v(u8 *c);
extern u8 *cmd_follow_ego(u8 *c);
extern u8 *cmd_wander(u8 *c);
extern u8 *cmd_normal_motion(u8 *c);
extern u8 *cmd_stop_motion(u8 *c);
extern u8 *cmd_start_motion(u8 *c);
extern u8 *cmd_step_size(u8 *c);
extern u8 *cmd_step_time(u8 *c);
extern u8 *cmd_set_dir(u8 *c);
extern u8 *cmd_get_dir(u8 *c);
extern u8 *cmd_program_control(u8 *c);
extern u8 *cmd_player_control(u8 *c);

#endif /* NAGI_VIEW_OBJ_MOTION_CMD_H */
