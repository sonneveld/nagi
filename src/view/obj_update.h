#ifndef NAGI_VIEW_OBJ_UPDATE_H
#define NAGI_VIEW_OBJ_UPDATE_H

extern u16 is_updated(VIEW *v);
extern u16 is_not_updated(VIEW *v);
extern BLIT *build_updated_list(void);
extern BLIT *build_static_list(void);

extern void blists_erase(void);
extern void blists_free(void);
extern void blists_draw(void);
extern void blists_update(void);

extern u8 *cmd_stop_update(u8 *c);
extern u8 *cmd_start_update(u8 *c);
extern u8 *cmd_force_update(u8 *c);
extern void obj_stop_update(VIEW *v);
extern void obj_start_update(VIEW *v);

#endif /* NAGI_VIEW_OBJ_UPDATE_H */
