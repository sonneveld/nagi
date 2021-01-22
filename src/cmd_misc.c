/*
_Rand                            cseg     0000753E 0000002D
CmdGetNum                        cseg     0000756B 0000007E
CmdObjStatusV                    cseg     000075E9 00000066
CmdShowPriScrn                   cseg     0000764F 00000021
CmdVersion                       cseg     00007670 00000016
*/

#include "agi.h"
#include "ui/msg.h"

#include "sys/drv_video.h"
#include "sys/gfx.h"

u8 *cmd_version(u8 *c)
{
	message_box("            NAGI\n" "     By Nick Sonneveld\n\n"
		"A complete reimplementation of Sierra's AGI interpreter made "
		"popular by games like Space Quest and Leisure Suit Larry.");	
	message_box("Want to make your own AGI based game?\n\n" "Visit:\n"
		"http://agidev.com/\n" "http://mega-tokyo.com/forum/\n");
	
	// TODO: add interpreter version type.
	
	return c;
}

u8 *cmd_shake_screen(u8 *c)
{
	gfx_shake(*(c++));
	return c;
}
