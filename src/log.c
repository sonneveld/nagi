/* FUNCTION list 	---	---	---	---	---	---	---

*/

/* BASE headers	---	---	---	---	---	---	--- */
#include "agi.h"
//#include "../agi.h"

/* LIBRARY headers	---	---	---	---	---	---	--- */
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <assert.h>

#include "ui/cmd_input.h"
#include "ui/msg.h"
#include "sys/sys_dir.h"

/* OTHER headers	---	---	---	---	---	---	--- */

/* PROTOTYPES	---	---	---	---	---	---	--- */
static void log_open(void);


/* VARIABLES	---	---	---	---	---	---	--- */


/* CODE	---	---	---	---	---	---	---	--- */


static FILE *log_stream = NULL;

u8 *cmd_log(u8 *c)
{
	u16 str_num;
	char msg[1000];
	
	assert(c != NULL);
	
	str_num = *(c++);
	
	if (log_stream == NULL)
		log_open();
	
	if (log_stream != NULL)
	{
		//si = dup(log_stream);
	//	if (si != 0xFFFF)
	//	{
			sprintf(msg, "\n\nRoom %d\nInput line: %s\n", state.var[0],
				input_prev);
			fwrite(msg, strlen(msg), 1, log_stream);
			str_wordwrap(msg, logic_msg(str_num), 78);
			fwrite(msg, strlen(msg), 1, log_stream);
		//	close(si);
	//	}
	}

	return c;
}

static void log_open(void)
{
	if (log_stream == NULL)
	{
		dir_preset_change(DIR_PRESET_GAME);
		log_stream = fopen("logfile", "a");
		//if (log_stream == NULL)
		//	log_stream = create("logfile", 0);
		//seek(log_stream, 0, 0, 2);
	}
}

void log_close(void)
{
	if (log_stream != NULL)
	{
		fclose(log_stream);
		log_stream = NULL;
	}
}
