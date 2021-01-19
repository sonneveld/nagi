#ifndef NAGI_VIEW_OBJ_BLIT_H
#define NAGI_VIEW_OBJ_BLIT_H

/*
f0 - the previous blit info struct?
next - the next blit info struct
*v = the view table entry
*buffer = a chunk of memory with size of  foo.x_size * foo.y_size
AGI allocates 0x10 bytes for this struct
*/

extern void blit_save(BLIT *b);
extern void blit_restore(BLIT *b);
extern void obj_blit(VIEW *v);

#endif /* NAGI_VIEW_OBJ_BLIT_H */
