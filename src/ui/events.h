#ifndef NAGI_UI_EVENTS_H
#define NAGI_UI_EVENTS_H

// --------- AN AGI EVENT STRUCT
struct agi_event_struct
{
	u16 type;	// the type of event
			// 2 = direction
			// 1 = ascii char	(ah = bios scan code)
			// 			(al = ascii char)
			// 3 = controller event
			// 10 = mouse
	u16 data;	// direction num, or controller num
	
	u16 x;	//mouse stuff
	u16 y;
	//SDL_keysym keysym;	// used if an ascii key
};
typedef struct agi_event_struct AGI_EVENT;
	
// ---------------- AN AGI SPECIAL KEY STRUCT USED BY THIS MODULE

struct key_struct
{
	SDL_Keycode symbol;
	u16 value;
};
typedef struct key_struct KEY;


u16 dir_keymap(SDL_Keysym *keysym);
AGI_EVENT *key_parse(SDL_Keysym *keysym);
extern AGI_EVENT *event_read(void);
extern void events_init(void);
extern u8 *cmd_unknown_173(u8 *c);
extern u8 *cmd_unknown_181(u8 *c);

extern void events_clear(void);
extern u16 char_poll(void);
extern u16 char_wait(void);
extern u16 has_user_reply(void);
extern u16 user_bolean_poll(void);

extern void joy_button_map(AGI_EVENT *agi_event);
extern AGI_EVENT *event_wait(void);

extern AGI_EVENT *user_event_decode(void *data);
extern u16 event_write(u16 type, u16 data);

// need event_write(type, data)

extern u8 *cmd_extension_171(u8 *c);

#endif /* NAGI_UI_EVENTS_H */
