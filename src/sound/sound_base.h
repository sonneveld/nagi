#ifndef sound_base_h_file
#define sound_base_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */

struct sound_struct
{
	struct sound_struct *next;	// 0-1
	u16 num;		// 2-3
	u8 *data;		// 4-5
	u8 *channel[4];	// 6-7, 8-9, A-B, C-D
};
typedef struct sound_struct SOUND;

/* VARIABLES	---	---	---	---	---	---	--- */
#define SS_CLOSED 0
#define SS_OPEN_PLAYING 1
#define SS_OPEN_STOPPED 2
extern int sound_state;
extern u16 sound_flag;

/* FUNCTIONS	---	---	---	---	---	---	--- */

extern void sound_list_init(void);
extern void sound_list_new_room(void);
extern SOUND *sound_find(u16 snd_num);
extern u8 *cmd_load_sound(u8 *c);
extern SOUND *sound_load(u16 snd_num);
extern u8 *cmd_sound(u8 *c);
extern u8 *cmd_stop_sound(u8 *c);
extern void sound_stop(void);

#endif