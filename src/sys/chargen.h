#ifndef chargen_h_file
#define chargen_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
extern u8 chgen_textmode;
extern SIZE font_size;
//extern FONT *agi_font;
/* FUNCTIONS	---	---	---	---	---	---	--- */
extern void ch_init(void);
extern void ch_shutdown(void);

extern void ch_update(void);
extern void ch_pos_get(POS *pos);
extern void ch_pos_set(POS *pos);
extern void ch_attrib( u8 colour, u16 flags );
extern void ch_put(u8 ch);
extern void ch_scroll(POS *pos1, POS *pos2, u16 scroll, u8 attrib);
extern void ch_clear(POS *pos1, POS *pos2, u8 attrib);
#endif
