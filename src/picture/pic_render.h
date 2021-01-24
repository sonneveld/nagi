#ifndef NAGI_PICTURE_PIC_RENDER_H
#define NAGI_PICTURE_PIC_RENDER_H

extern u8 pos_init_y;
extern u8 pos_init_x;
extern u8 pos_final_y;
extern u8 pos_final_x;

extern u8 col_even;
extern u8 col_odd;
extern u8 sbuff_drawmask;
extern u8 colour_pictpart;
extern u8 colour_pripart;
extern u8 *given_pic_data;

extern void render_pic(u8 overlay);
extern void render_overlay(void);

#endif /* NAGI_PICTURE_PIC_RENDER_H */
