/*
_ParseString                     cseg     000018AC 000000AC
CmdParse                         cseg     00001958 00000045
_ParseClean                      cseg     0000199D 000000CE
_WordFind                        cseg     00001A6B 0000015C
_WordIsolate                     cseg     00001BC7 0000001D
_WordNext                        cseg     00001BE4 00000020
*/

// for tolower()
#include <ctype.h>
// for strchr()
#include <string.h>
#include <assert.h>

#include "../agi.h"

#include "../ui/parse.h"

#include "../flags.h"

#include "../agi_string.h"

// byte-order support
#include "../sys/endian.h"


#define WORD_IGNORE 0
#define WORD_ROL 9999
#define WORD_ANY 1

#define WORD_BUF_SIZE 10

void parse(u8 *string);
u8 *cmd_parse(u8 *c);
static void parse_read(u8 *s);
static u16 word_find(void);
static void playerWordIsolate(void);
static u8 *dictWordNext(u8 *si);

static const u8 *char_separators = " ,.?!();:[]{}";
static const u8 *char_illegal = "'`-\"";	// 0x27 is '

u16 word_num[WORD_BUF_SIZE];
u8 *word_string[WORD_BUF_SIZE];

u16 word_total = 0;	// bad word
u8 *words_tok_data = 0;

// work area
static u8 parse_string[42];
static u8 *strPtr;

void parse(u8 *string)
{
	u16 wordNumber;
	u8 *wordString;	// the string data of the word

	memset(word_string, 0, sizeof(word_string));
	memset(word_num, 0, sizeof(word_num));

	parse_read(string);
	word_total = 0;
	strPtr = parse_string;
	
	while ((*strPtr != 0) && (word_total < WORD_BUF_SIZE))
	{
		wordString = strPtr;
		wordNumber = word_find();
	
		if (wordNumber == 0xFFFF)	// bad
		{
			word_string[word_total] = strPtr;
			state.var[V09_BADWORD] = word_total + 1;	// bad word
			word_total++;
			assert(word_total > 0); // we need flag 2 set
			break;
		}

		if (wordNumber != WORD_IGNORE)	// good
		{
			word_num[word_total] = wordNumber;
			word_string[word_total] = wordString;
			word_total++;
		}
		// if WORD_IGNORE then skip it
	}
	
	if (word_total > 0)
		flag_set(F02_PLAYERCMD);
}


u8 *cmd_parse(u8 *c)
{
	flag_reset(F02_PLAYERCMD);
	flag_reset(F04_SAIDACCEPT);
	if (*c < 12)
		parse(state.string[*c]);
	c++;
	return c;
}


// cleans the word.. separates good words by ' '
// puts in it parse_string[]
static void parse_read(u8 *str)
{
	u8 *buf;

	buf = parse_string;	

	while (*str)
	{
		// skip excess separators at start and inbetween words
		if ( (strchr(char_separators, *str) != 0) ||
			(strchr(char_illegal, *str) != 0) )
		{
			str++;
		}
		else
		{
			assert(*str);
			do
			{
				if (strchr(char_separators, *str) != 0)
				{
					*(buf++) = 0x20;	// space
					break;
				}
				
				// if not an illegal character add to buffer
				if (strchr(char_illegal, *str) == 0)
					*(buf++) = *str;
				str++;
			} while (*str != 0);
		}
	}
	
	if ((buf > parse_string) &&	// if buffer has been modified
		(*(buf-1) == 0x20))		// and the last character is a space
		buf--;							// then remove space
	*buf = 0;
}


// accesses the words.tok file
static u16 word_find()
{
	u16 indexOffset;	// offset to data for that letter
	u16 chCount;		// count of characters already matched
	u16 chFirst;		// lowercase version of the first character in the word.
	u16 wordNum;
	u8 *wordNext;		// the next word after the current on
	u8 *wordData;
	u8 *playerInput;
	
	wordNum = 0xFFFF;
	wordNext = 0;
	chFirst = tolower(strPtr[0]);
	
	if ((chFirst<'a') || (chFirst>'z'))	// will not search for words not beginning with letter
		playerWordIsolate();
	else
	{
		if ( (strPtr[1]==' ') || ( strPtr[1]==0) )
			if ( (chFirst=='a') || (chFirst == 'i') )	// automatically skip 'a' and 'i' as words
			{
				wordNum = WORD_IGNORE;	// words are ignored
				wordNext = (u8 *)strPtr + 1;
				if (strPtr[1] == ' ')
					wordNext++;
			}

		// bug?.. shouldn't we jump?
		// or do we check for words like "a bird" in the dictionary first?
			
		// lookup the first letter in the index
		indexOffset = load_be_16(words_tok_data + (chFirst - 'a')*sizeof(u16));
			
		if (indexOffset == 0)	// if no words exist with that first letter..
			playerWordIsolate();
		else
		{
			wordData = words_tok_data + indexOffset;
			playerInput = strPtr;
			chCount = 0;
			while ((wordData != 0) && (*wordData >= chCount))
			{
				if (*(wordData++) == chCount)
				{
					// this shows bits are inverted in the dictionary?
					while ((*wordData&0x7F) == (tolower(*playerInput)^0x7F))
					{
						playerInput++;
						chCount++;
						
						if (*wordData & 0x80)	// if msb set
						{
							// if we're at the end of the player's word anyway
							if ((*playerInput==0)||(*playerInput==' '))
							{
								wordNum = load_be_16(wordData+1);
								wordNext = playerInput;
								if (*playerInput != 0)	// skip past space
									wordNext++;
							}
							break;
						}
						
						wordData++;
					}					
				}
				// find longest match.
				// we don't check for " " since it may appear in the word??
				if (*playerInput == 0) 
					break;
				wordData = dictWordNext(wordData);	// next word??
			}
		
			// if we haven't defined wordNext, that means there's an error with the current one
			if (wordNext == 0) 
				playerWordIsolate();
			else
			{
				strPtr = wordNext;
				if (*strPtr)
					*(strPtr-1) = 0;
			}
		}
	}
	return wordNum;
}

// go through str until we reach a space or zero
// then set it to zero.
static void playerWordIsolate()
{
	u8 *str;
	for (str=(u8*)strPtr; (*str!=' ')&&(*str); str++) {};
	*str = 0;
}

// skip past current word in dictionary
// return pointer to next word
// return 0 if no word exists
static u8 *dictWordNext(u8 *str)
{
	while ((*str & 0x80) == 0)	// msb
		str++;
	// this is how it is implemented in agi.. 
	// but it will never return zero.  how can it?  the first bit is set!
	return (*str)?(str+3):(0);
}

