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

#include "../agi.h"

#include "../ui/parse.h"

#include "../flags.h"

#include "../agi_string.h"

// byte-order support
#include "../sys/endian.h"

void parse(u8 *string);
u8 *cmd_parse(u8 *c);
void parse_read(u8 *s);
u16 word_find(void);
void word_isolate(void);
u8 *word_next(u8 *si);

u8 *char_separators = " ,.?!();:[]{}";
u8 *char_illegal = "'`-\"";	// 0x27 is '

u16 word_num[10];
u8 *word_string[10];

u16 word_total = 0;	// bad word
u8 *words_tok_data = 0;

// work area
u8 parse_string[42];
u8 *str_ptr;


 
void parse(u8 *string)
{
	s16 di;
	u8 *si;
	int i;
	u16 temp2;
	u8 *temp4;
	
	si = string;
	
	for (i=0; i<10; i++)
	{
		word_string[i] = 0;
		word_num[i] = 0;
	}
	//printf("\nparse=%s\n", si);
	parse_read(si);
	//printf("\nparse=%s\n", parse_string);
	di = 0;
	str_ptr = parse_string;
	
loc18e8:
	temp4 = str_ptr;
	if ( *str_ptr == 0) goto loc1940;
	if ( di >= 10) goto loc1940;
	temp2 = word_find();
	goto loc1932;
	
	
loc1901:	// bad
	word_string[di] = str_ptr;
	state.var[V09_BADWORD] = di + 1;	// bad word
	word_total = (u8)(di + 1);	// 16bit stripped down to 8
	goto loc1948;
	
loc1919:	// normal
	//printf("%s - %d ", temp4, temp2);
	word_num[di] = temp2;
	word_string[di] = temp4;
	di++;
	goto loc193b;
	
	
loc1932:
	if ( temp2 == 0xFFFF) goto loc1901;	// bad
	if ( temp2 != 0) goto loc1919;	// good
loc193b:
	// if 0 then skip it
	goto loc18e8;
	
loc1940:
	if ( di <= 0) goto loc1952;
	word_total = di;
loc1948:
	flag_set(F02_PLAYERCMD);
loc1952:
}


u8 *cmd_parse(u8 *c)
{
	u8 al;
	flag_reset(F02_PLAYERCMD);
	flag_reset(F04_SAIDACCEPT);
	al = *(c++);
	if ( al < 12)
		parse(state.string[al]);
	return c;
}


// cleans the word.. separates good words by ' '
// puts in it parse_string[]
void parse_read(u8 *s)
{
	u8 temp1;
	u8 *temp3;
	u8 *si;
	
	//printf("user input = %s\n", s);
	si = s;
	temp3 = parse_string;
loc19ad:
	temp1 = *si;
	if ( temp1 != 0) goto loc19b9;
	goto loc1a4c;
loc19b9:
	if ( temp1 == 0) goto loc19f1;
	if ( strchr(char_separators, temp1) != 0) goto loc19e7;
	if ( strchr(char_illegal, temp1) == 0) goto loc19f1;
loc19e7:
	si++;
	temp1 = *si;
	goto loc19b9;
	
loc19f1:
	if ( temp1 == 0) goto loc1a4c;
loc19f7:
	if ( temp1 == 0) goto loc1a3a;
	if ( strchr(char_separators, temp1) != 0) goto loc1a3a;
	if ( strchr(char_illegal, temp1) == 0)
		*(temp3++) = temp1;
	
	si++;
	temp1 = *si;
	goto loc19f7;
	
loc1a3a:
	if ( temp1 == 0) goto loc1a4c;
	*(temp3++) = 0x20;	// space
	goto loc19ad;
	
loc1a4c:
	if ( (temp3 > parse_string) && (*(temp3-1) == 0x20) )
		temp3--;
	*temp3 = 0;
	
	//printf("pre-parsed version = %s\n", parse_string);
}

// accesses the words.tok file
u16 word_find()
{
	u16 tempc;	// offset to data for that letter
	u16 tempa;
	u16 temp8;	// character from user input?
	u16 temp6;	// ptr to the letter array at top of words.tok
	u16 w_num;
	u8 *temp2;	// str_ptr related
	
	u8 *word_data;
	u8 *user_data;
	//u8 *bx;
	u16 ax, cx;
	
	w_num = 0xFFFF;
	temp2 = 0;
	temp8 = tolower(str_ptr[0]);
	if ( ( temp8 < 'a')  || ( temp8 > 'z') )
		word_isolate();
	else
	{
		if (  (str_ptr[1] == ' ') || ( str_ptr[1] == 0)  )
			if (  ( temp8 == 'a') || ( temp8 == 'i')  )	// automatically skip 'a' and 'i'
			{
				w_num = 0;
				temp2 = (u8 *)str_ptr + 1;
				if ( str_ptr[1] == ' ')
					temp2 ++;
			}
		temp6 = (temp8 - 'a') << 1;

		tempc = load_be_16(words_tok_data + temp6);
		if ( tempc == 0)
			word_isolate();
		else
		{
			word_data = words_tok_data + tempc;
			user_data = str_ptr;
			tempa = 0;
			while (  (*word_data >= tempa) && (word_data != 0)  )
			{
				if ( *(word_data++) == tempa)
				{
					goto loc1b4b;
				loc1b4a:
					word_data++;
				loc1b4b:
					cx = tolower(*user_data) ^ 0x7F;
					ax = *word_data & 0x7F;
					if ( ax != cx) goto loc1b9a;
					user_data++;
					tempa++;
					if ( (*word_data & 0x80) == 0) goto loc1b4a;
						
					if (  (*user_data == 0) || ( *user_data == ' ')  )
					{
						w_num = load_be_16(word_data+1);
						temp2 = user_data;
						if (*user_data != 0)
							temp2++;
					}
				loc1b9a:
				}
				if ( *user_data == 0) 
					break;
				word_data = word_next(word_data);	// next word??
			}
		
			
			if ( temp2 == 0) 
				word_isolate();
			else
			{
				str_ptr = temp2;
				if ( *str_ptr != 0)
					*(str_ptr-1) = 0;
			}
		}
	}
	return w_num;
}

void word_isolate()
{
	u8 *s;
	
	s = (u8 *)str_ptr;
	while ( (*s!=' ') && (*s!=0) )
		s++;
	*s = 0;
}

u8 *word_next(u8 *si)
{
	while ((*si & 0x80) == 0)	// 1000 0000
		si++;
	if (*si == 0)
		return 0;
	else
		return si + 3;
}

