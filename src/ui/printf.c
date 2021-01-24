/*
_sprintf                         cseg     00002374 0000001C
_printf                          cseg     00002390 00000073
_FormatDecimal                   cseg     00002403 0000001F
_FormatUnsigned                  cseg     00002422 00000011
_FormatHex                       cseg     00002433 00000011
_FormatString                    cseg     00002444 00000009
_FormatStringAX                  cseg     0000244D 00000011
_FormatChar                      cseg     0000245E 00000014
*/

#include <stdarg.h>
#include <stdio.h>
#include <assert.h>
#include <string.h>

#include "../agi.h"

#include "printf.h"
#include "string.h"

// window_put_char
#include "../ui/window.h"
#include "../sys/chargen.h"

static void format_string_ax(u8 *str);
static void format_char(u8 ch);

// void *format_ip = 0;
static u8 *format_strbuff = 0;
static u8 format_to_string = 0;	// boolean value

/*
?? sprintf()
{
	asdfkljasdlfk
}

*/

static u8 *di;	// sprintf string

static int charCount(char *str)
{
	int count;

	assert(str);

	count = 0;
	str = strchr(str, '%');

	while (str != 0)
	{
		count++;
		str++;
		str = strchr(str, '%');
	}

	return count;
}

void agi_printf(u8 *var8, ...)
{
	va_list ap;
	
	u8 *si;
	s16 bx;
	u8 al;
	
	va_start(ap, var8);
	
	si = var8;
	di = format_strbuff;
	//bx = bp + 0xA;

	al = *(si++);

	while (al != 0)
	{
		if ( al != '%')
			format_char(al);
		else
		{

			switch (*(si++))
			{
				case 's':		// string
					format_string_ax(va_arg(ap, u8 *));
					//bx += 2;
					break;
				
				case 'd':		// decimal
					bx = (s16)va_arg(ap, int);
					if (bx < 0)
					{
						format_char('-');
						format_string_ax(  int_to_string(bx * -1)  );
					}
					else
						format_string_ax(  int_to_string(bx)  );
					//bx += 2;
					break;
				
				case 'u':		// unsigned decimal
					format_string_ax(  int_to_string( (u16)va_arg(ap, int))  );
					//bx += 2;
					break;
				
				case 'x':		// hex number
					format_string_ax(  int_to_hex_string( (u16)va_arg (ap, int))  );
					//bx += 2;
					break;
				
				case 'c':		// character
					//al = *bx;
					//bx += 2;	// everything is pushed on as a word
					format_char((u8)va_arg(ap, int));
					break;
				
				default:		// not recognised
					format_char('%');
					si--;
			}
		}
		al = *(si++);
	}
	

	if ( (format_to_string & 0xFF) != 0)
		*(di++) = 0;
	va_end (ap);
	ch_update();
}


static void format_string_ax(u8 *str)
{
	u8 al;
	
	al = *(str++);
	while ( al != 0 )
	{
		format_char(al);
		al = *(str++);
	}
}

static void format_char(u8 ch)
{
	if (format_to_string != 0)
		*(di++) = ch;
	else
		window_put_char(ch);
}

