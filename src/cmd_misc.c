/*
_Rand                            cseg     0000753E 0000002D
CmdGetNum                        cseg     0000756B 0000007E
CmdObjStatusV                    cseg     000075E9 00000066
CmdShowPriScrn                   cseg     0000764F 00000021
CmdVersion                       cseg     00007670 00000016
*/

#include "agi.h"
#include "ui/msg.h"

u8 *cmd_version(u8 *c)
{
	//message_box("New Adventure Game Interpreter (NAGI)\n        By Nick Sonneveld");
	//message_box("Adventure Game Interpreter\n      Version 3.002.149");
	message_box("      NAGI\nBy Nick Sonneveld");
	//message_box("feh");
	
	// nagi
	// by nick sonneveld
	// emulating version type..
	
	
	return c;
	
}
