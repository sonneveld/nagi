//CmdPause                       cseg     00000257 00000028
//CmdQuit                        cseg     0000027F 0000002F
//_Finish                        cseg     000002AE 00000015


#include <stdlib.h>
#include "agi.h"

#include "base.h"

#include "sound/sound.h"
 
#include "ui/events.h"
#include "ui/msg.h"

#warning base.c needs work

#include "sys/time.h"

#include "flags.h"
#include "res/res.h"

#include "sys/sys_dir.h"

#include "sys/drv_video.h"

u8 *cmd_pause(u8 *c)
{
	clock_state = 1;
	events_clear();
	sound_stop();
	message_box("      Game paused.\nPress Enter to continue." );
	clock_state = 0;

	return(c);
}

u8 *cmd_quit(u8 *c)
{
	sound_stop();
	if (*(c++) == 1)
		agi_exit();	
	if (message_box("Press ENTER to quit.\nPress ESC to keep playing.") == 1)
		agi_exit();	
	return c;
}


void agi_exit()
{
	//close_log_file();
	//restore_vectors();

	clock_denit();
	// includes sound, time, events
	//vid_mode_set(original_video_mode);
	sound_driver_denit();
	vid_shutdown();
	
	free(lzw_dict);
	
	dir_preset_change(DIR_PRESET_ORIG);
	
	exit(0);
}