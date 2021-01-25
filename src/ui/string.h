#ifndef NAGI_UI_STRING_H
#define NAGI_UI_STRING_H

// some functions (marked below) will return results from a shared static buffer
// so previous results will be overwritten.

extern u16 string_to_int(const char *string);
extern char *int_to_string(u16 num);				// shared buffer
extern char *string_zero_pad(const char *str, u16 pad_size);	// shared buffer
extern char *int_to_hex_string(u16 num);			// shared buffer
extern char *string_lower(char *str);

#endif /* NAGI_UI_STRING_H */
