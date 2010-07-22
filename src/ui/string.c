/*
_StringLength                    cseg     00004CD2 0000001B
_StringCopy                      cseg     00004CED 00000034
_StringNCopy                     cseg     00004D21 0000003F
_StringCat                       cseg     00004D60 0000003C
_StringCompare                   cseg     00004D9C 0000002A
_String2Int                      cseg     00004DC6 0000003B
_Int2String                      cseg     00004E01 0000003E
_StringZeroPad                   cseg     00004E3F 00000053
_IntToHex                        cseg     00004E92 00000056
_StringReverse                   cseg     00004EE8 0000003B
_CharToLower                     cseg     00004F23 0000001F
CmdRandom                        cseg     00004F42 00000048
CmdInitDisk                      cseg     00004F8A 0000000C
_StringChar                      cseg     00004F96 00000031
_StringLower                     cseg     00004FC7 00000027
*/

// *aToI is pretty similar  .. sierra's implementation only works for positive numbers
// *itoA is new I think
// *strngzeropad is new
// int to hex could be done using gnu libraries
// *strrev not in gnu libraries
// *stringlower is not in library

/*
These are implemented in the gnu library:
	abs(u16) is the same as the one in the library
	twelve times() is useless
	void strlen is the same
	strcpy  is the same
	strncpy is the same
	strcat is the same
	strcmp  is the same
	tolower is same
	stringchar is same
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <assert.h>
#include "../agi.h"

#include "string.h"

// atoi
u16 str_to_u16(const char *string)
{
	const char *s;
	u16 num;
	s = string;
	while (*s == ' ')
		s++;
	num = 0;
	while (  (*s >= '0') && (*s <= '9')  ) 
		num = num*10 + *(s++) - '0';
	return num;
}

char *u16_to_str(char *out, size_t outlen, u16 num)
{
	u16 si;
	char *di;
	
	si = num;
	di = out;
	do
	{
        assert (di < (out+outlen));
		*(di++) = (si % 10) + '0';
		si = si / 10;
	}
	while (si != 0);
	
    assert (di < (out+outlen));
	*di = 0;
	return string_reverse(out);
}

// eg "123" = "000000123" or something
char *string_zero_pad(char *out, size_t outlen, const char *str, u16 pad_size)
{
	u16 size, str_size;

    assert (strlen(str) <= (outlen -1) );
    assert (pad_size <= (outlen -1) );

	memset(out, '0', outlen);	// '0'
    // we ignore null byte cause we are copying
    // the str verbatim

	str_size = strlen(str);	// var8 into temp.. returns var8;

	if (str_size > pad_size)
		size = str_size;
    else
        size = pad_size;
    
	strcpy(out + size - str_size, str);
	return out;
}

char *hex_conv = "0123456789ABCDEF";

char *u16_to_hex(char *out, size_t outlen, u16 num)
{
	char *si = out;
	
	do
	{
        assert(si < (out+outlen));
		*(si++) = hex_conv[num & 0xF];
		num >>= 4;
	} while (num != 0);

    assert(si < (out+outlen));
	*si = 0;
	
	return string_reverse(out);
}

#if 0
u8 *int_to_hex_string_v2(u16 num)
{
	u8 temp;
	u8 *si;
	u16 cx;
	
	si = string_buff;
	do
	{
		temp = num % 0x10;

		if ( temp > 9)
			cx = 0x67-0x10;	//'W'
		else
			cx = 0x30; 	//'0'
		*(si++) =  temp + cx;
		num = num / 0x10;
	}
	while (num > 0);
		
	*si = 0;
	return string_reverse(string_buff);
}
#endif

char *string_reverse(char *str)
{
	char *si, *di;  
	char temp;
	
	si = str;
	di = str + strlen(si) - 1;
	while (di > si)
	{
		temp = *si;
		*si = *di;
		*di = temp;	
		si++;
		di--;
	}
    return str;
}

// this requires writable string constants maybe for some bits 'n pieces
char *string_lower(char *str)
{
    char *di;
	di = str;
	while (*di != 0)
	{
		*di = tolower(*di);
		di++;
	}
	return str;
}

#if 0
// find character
// u8 *strchr
u8 *sub4f96(u8 *given_string, u16 ch)
{
	u8 *s = given_string;
	while (  (*s != 0) && (*s != ch)  )
		s++;
	if ( ch != *s)
		return 0;
	else
		return s;
}
#endif

#ifdef __MINGW32__
/* mingw32 does not define strtok_r */
char *strtok_r(char *newstring, const char *delimiters, char **save_ptr)
{
	u8 *token_cur;
	u8 *token_cur_end;
	u8 *token_next;
	
	assert(save_ptr != 0);
	assert(delimiters != 0);
	
	if (newstring != 0)
		token_cur = newstring;
	else
		token_cur = *save_ptr;
	
	if (token_cur != 0)
	{
		token_cur_end = token_cur + strcspn(token_cur, delimiters);
		token_next = token_cur_end + strspn(token_cur_end, delimiters);
	
		if (*token_next == '\0')
			token_next = 0;
		
		*token_cur_end = '\0';
		*save_ptr = token_next;
	}
	
	return token_cur;
}
#endif
