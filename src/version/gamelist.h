#ifndef game_list_h_file
#define game_list_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
struct game_info_struct
{
	struct game_info_struct *next;
	u8 *name;		// name of the game
	u8 *id;		// id of the game
	u8 *standard;	// standard of agi
	u8 *location;	// directory location
	u8 format;		// res format
	u8 crc_match; 	// 0 no match, 1=crc
};
typedef struct game_info_struct GAMEINFO;
	

/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
extern GAMEINFO *game_list_new(u8 *dir_name);
extern void game_list_delete(GAMEINFO *g_info);
extern GAMEINFO *game_list_force(u8 *f_standard);

#endif