#ifndef video_dummy_h_file
#define video_dummy_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
extern void dummyvid_init(RSTATE *rend_state);
extern void dummyvid_shutdown(void);
extern void dummyvid_update(u16 x, u16 y, u16 width, u16 height);
extern void dummyvid_clear(void);
extern void dummyvid_shake(u8 count);
extern void dummyvid_pos_get(POS *pos);
extern void dummyvid_pos_set(POS *pos);
extern void dummyvid_char_attrib(u8 colour, u16 flags);
extern void dummyvid_char_put(u8 ch);
extern void dummyvid_scroll(POS *pos1, POS *pos2, u16 scroll, u8 attrib);
extern void dummyvid_char_clear(POS *pos1, POS *pos2, u8 attrib);

#endif