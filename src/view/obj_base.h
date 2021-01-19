// #include obj_blit.h as well

extern u16 objtable_size;
extern VIEW *objtable;
extern VIEW *objtable_tail;

extern BLIT blitlist_updated;
extern BLIT blitlist_static;

extern void objtable_new(u16 max);
extern u16 blitlist_free(BLIT *b);
extern u16 blitlist_erase(BLIT *b);
extern BLIT *blitlist_build( u16(*f)(VIEW *) , BLIT *head);

extern u8 *cmd_unknown_174(u8 *c);


extern BLIT *blit_new(VIEW *v);
extern void blitlist_draw(BLIT *h);
extern void blitlist_update(BLIT *h);

extern u8 *cmd_animate_obj(u8 *c);

extern u8 *cmd_unanimate_all(u8 *c);

extern void objtable_update(void);

