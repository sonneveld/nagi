#ifndef vid_sdl_h_file
#define vid_sdl_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
extern void sdlvid_init(RSTATE *rend_state);
extern void sdlvid_shutdown(void);
extern void sdlvid_update(u16 x, u16 y, u16 width, u16 height);
extern void sdlvid_clear(void);
extern void sdlvid_shake(u8 count);
extern void sdlvid_pos_get(POS *pos);
extern void sdlvid_pos_set(POS *pos);
extern void sdlvid_char_attrib(u8 colour, u16 flags);
extern void sdlvid_char_put(u8 ch);
extern void sdlvid_scroll(POS *pos1, POS *pos2, u16 scroll, u8 attrib);
extern void sdlvid_char_clear(POS *pos1, POS *pos2, u8 attrib);
#endif