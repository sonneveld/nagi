/*
_GameIdCopy                      cseg     00005EC2 00000015
_GameIdTest                      cseg     00005EDE 00000023
_Save5F01                        cseg     00005F01 0000006A
_Save5F6B                        cseg     00005F6B 000000CF
*/

#include "agi.h"
#include "base.h"
#include "game_id.h"

#include "res/res.h"

//u8 *agi_id = "gameIDX";
//u8 *agi_id = "MH2";
//u8 *agi_id = "gameIDX";

//u8 agi_id_honour = 0;

void game_id_test()
{
	u8 *si, *di, al;
	/*
	// have to check since v3 NEED the game id to be ok
	if (standard.game_id_honour != 0) 
	{
		si = state.id;
		di = standard.game_id;
		
		do
		{
			al = *(si++);
			if (al != *di)
			{
				printf("Interpreter's ID [%s] and cmd.set.game.id() [%s] do not match.\n", standard.game_id, state.id);
				agi_exit();
			}
			di++;
		} while ( al != 0);
	}*/
}

// used for VERSION 3 games
void game_id_copy()
{
	strcpy(dir_id, c_game_id);
}