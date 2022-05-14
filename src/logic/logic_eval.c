/*
_LogicEval                       cseg     000007E3 00000040
CmdEqualN                        cseg     00000823 00000011
CmdEqualV                        cseg     00000834 00000017
CmdLessN                         cseg     0000084B 00000011
CmdLessV                         cseg     0000085C 00000017
CmdGreaterN                      cseg     00000873 00000011
CmdGreaterV                      cseg     00000884 00000017
CmdIsSetN                        cseg     0000089B 00000005
CmdIsSetV                        cseg     000008A0 0000000D
CmdHas                           cseg     000008AD 00000019
CmdPosN                          cseg     000008C6 00000054
_GetViewPos                      cseg     0000091A 00000017
CmdController                    cseg     00000931 0000000A
CmdObjInRoom                     cseg     0000093B 00000021
CmdSaid                          cseg     0000095C 00000062
CmdHaveKey                       cseg     000009BE 0000001A
CmdRetFalse                      cseg     000009D8 00000003
CmdCompareStrns                  cseg     000009DB 0000000F
*/
#include "../agi.h"
#include "../flags.h"


#include "../view/obj_blit.h"
#include "../view/obj_base.h"

#include "../logic/logic_base.h"
#include "../logic/logic_execute.h"
#include "../logic/logic_eval.h"

#include "../agi_string.h"

// for cmd_said
#include "../ui/parse.h"

// byte-order support
#include "../sys/endian.h"

// object
#include "../objects.h"

// char poll

#include "../ui/events.h"

#include "../ui/controller.h"

static u8 is_obj_inside(u16 left, u16 right, u16 y);

// logic_data is the logic data
// return the true/false bit

u8 cmd_equal_n()
{
	u8 bl, al;
	bl = *(logic_data++);
	al = *(logic_data++);
	return ( al == state.var[bl] );
}

u8 cmd_equal_v()
{
	u8 bl, al;
	bl = *(logic_data++);
	al = *(logic_data++);
	return ( state.var[al] == state.var[bl] );
}

u8 cmd_less_n()
{
	u8 bl, al;
	bl = *(logic_data++);
	al = *(logic_data++);
	return ( state.var[bl] < al );
}

u8 cmd_less_v()
{
	u8 bl, al;
	bl = *(logic_data++);
	al = *(logic_data++);
	return (state.var[bl] < state.var[al]);
}

u8 cmd_greater_n()
{
	u8 bl, al;
	bl = *(logic_data++);
	al = *(logic_data++);
	return ( state.var[bl] > al );
}

u8 cmd_greater_v()
{
	u8 bl, al;
	bl = *(logic_data++);
	al = *(logic_data++);
	return (state.var[bl] > state.var[al]);
}

// flags
u8 cmd_isset()
{
	return (flag_test( *(logic_data++) )  );
}

u8 cmd_isset_v()
{
	return (flag_test( state.var[*(logic_data++)] )  );
}



u8 cmd_has()
{
	return (inv_obj_table[*(logic_data++)].location == 0xFF);
}



u8 cmd_posn()
{
	VIEW *v;
	v = &objtable[*(logic_data++)];
	return is_obj_inside(v->x, v->x, v->y);
}

u8 cmd_center_posn()
{
	VIEW *v;
	v = &objtable[*(logic_data++)];
	return is_obj_inside(v->x+v->x_size/2,  v->x+v->x_size/2, v->y);
}

u8 cmd_right_posn()
{
	VIEW *v;
	v = &objtable[*(logic_data++)];
	return is_obj_inside(v->x+v->x_size-1, v->x+v->x_size-1, v->y);
}

u8 cmd_obj_in_box()
{
	VIEW *v;
	v = &objtable[*(logic_data++)];
	return is_obj_inside(v->x, v->x+v->x_size-1, v->y);
}
	
static u8 is_obj_inside(u16 left, u16 right, u16 y)
{
	if (left < *(logic_data++))
	{
		logic_data += 3;
		return 0;
	}
	else if (y < *(logic_data++))
	{
		logic_data += 2;
		return 0;
	}
	else if (right > *(logic_data++))
	{
		logic_data += 1;
		return 0;
	}
	else if (y > *(logic_data++))
		return 0;
	
	return 1;
}


/*
CmdController                    cseg     00000931 0000000A
CmdObjInRoom                     cseg     0000093B 00000021
*/

u8 cmd_controller()
{
	return control_state[*(logic_data++)];
}

u8 cmd_obj_in_room()
{
	u8 obj_room;
	obj_room = inv_obj_table[*(logic_data++)].location;
	return ( obj_room == state.var[*(logic_data++)] );
}

static char *word_str_lookup[1000] = {0};

char *sentences_seen[1000] = {0};
int sentences_count = 0;
char *sentences_entered[1000] = {0};
int sentences_entered_count = 0;
int room_for_sentences = -1;

void clear_sentences()
{
	for (int i = 0; i < sentences_count; i++) {
		free(sentences_seen[i]);
		sentences_seen[i] = 0;
	}
	sentences_count = 0;

	for (int i = 0; i < sentences_entered_count; i++) {
		free(sentences_entered[i]);
		sentences_entered[i] = 0;
	}
	sentences_entered_count = 0;
}

void add_sentence(char *sentence)
{
	if (sentence == 0){ return; }
	if (strlen(sentence) <= 0) { return; }
	// does it exist?
	for (int i = 0; i < sentences_count; i++) {
		if (strcmp(sentence, sentences_seen[i]) == 0) {
			return;
		}
	}
	if (sentences_count == 1000) {
		return;
	}
	// add it to end
	sentences_seen[sentences_count] = strdup(sentence);
	sentences_count += 1;
}


void add_entered_sentence(char *sentence)
{
	if (sentence == 0){ return; }
	if (strlen(sentence) <= 0) { return; }
	// does it exist?
	for (int i = 0; i < sentences_entered_count; i++) {
		if (strcmp(sentence, sentences_entered[i]) == 0) {
			return;
		}
	}
	if (sentences_entered_count == 1000) {
		return;
	}
	// add it to end
	sentences_entered[sentences_entered_count] = strdup(sentence);
	sentences_entered_count += 1;
}
int is_sentence_entered(const char *sentence)
{
	for (int i = 0; i < sentences_entered_count; i++) {
		if (strcmp(sentence, sentences_entered[i])== 0) {
			return 1;
		}
	}
	return 0;
}

int comp (const void * elem1, const void * elem2) 
{
	const char **ia = (const char **)elem1;
    	const char **ib = (const char **)elem2;
	return strcmp(*ia, *ib);
}

void sentences_dump()
{
	qsort(sentences_seen, sentences_count, sizeof(char*), comp);

	for (int i = 0; i < sentences_count; i++) {
		printf("%s\n", sentences_seen[i]);
	}
	printf("num sentences: %d\n", sentences_count);
}

void load_words()
{
	char line[500];

	if (word_str_lookup[0] != 0) {
		return;

	}
	FILE *f = fopen("sq1_words.txt", "r");

	int index = 0;

	while(fgets(line, 500, f)) {

		for (;;) {
			int x = strlen(line);
			if ( x == 0) {
				break;
			}
			if (line[x-1] != '\n') {
				break;
			}
			line[x-1] = 0;
		}
		// printf("%s\n", line);

		word_str_lookup[index] = malloc(strlen(line) + 1);
		strcpy(word_str_lookup[index], line);

		index += 1;
	}

	fclose(f);

}

u8 cmd_said()
{
	u16 word_remaining;
	u16 word_bad;
	u16 num;
	u16 cur;
	
	word_remaining = *(logic_data++);	// number of words to check.
	word_bad = word_total;

	load_words();


	int cur_room = state.var[V00_ROOM0];

	if (room_for_sentences != cur_room) {
		clear_sentences();
		room_for_sentences = cur_room;
	}


	char sentencebuf[200] = {0};

	u8 *dbg_logic = logic_data;
	for (int i = 0; i < word_remaining; i++) {
		int dbg_num = load_le_16(dbg_logic);
		dbg_logic += 2;
		if (dbg_num == 9999) {
			strcat(sentencebuf, "*");
			break;
		}
		else {
			strcat(sentencebuf, word_str_lookup[dbg_num]);
			strcat(sentencebuf, " ");
		}
	}
	add_sentence(sentencebuf);
	
	if (word_bad != 0)
		if (flag_test(F04_SAIDACCEPT) == 0)
			if (flag_test(F02_PLAYERCMD) != 0)
			{
				cur = 0;
				while (word_remaining != 0)
				{
					num = load_le_16(logic_data);
					logic_data += 2;
					word_remaining --;
					
					if (num == 9999)	// rol
					{
						logic_data += word_remaining << 1;
						word_remaining = 0;
						word_bad = 0;
						break;
					}
					if (word_bad == 0)
					{
						word_bad++;
						break;
					}
					if ( (num != word_num[cur]) && (num != 1) )	// any word
						break;
					cur ++;
					word_bad--;
				}
			}

	if ((word_remaining | word_bad) == 0)
	{
		add_entered_sentence(sentencebuf);
		flag_set(F04_SAIDACCEPT);	// said command accepted
		return 1;
	}
	else
	{
		logic_data += word_remaining << 1;
		return 0;
	}
}

u8 cmd_have_key()
{
	u16 ax;
	ax = state.var[V19_KEYPRESSED];
	if ( ax == 0)
	{
		do
		{
			ax = char_poll();
		}
		while (ax == 0xFFFF);
	}

	if ( ax != 0 )
	{
		state.var[V19_KEYPRESSED] = ax;
		return 1;
	}
	else
		return 0;
}

/*
CmdRetFalse                      cseg     000009D8 00000003
CmdCompareStrns                  cseg     000009DB 0000000F
*/

u8 cmd_compare_strings()
{
	u16 bx;
	bx = *(logic_data++);
	return agi_string_compare(*(logic_data++), bx);
}

