
extern u16 str_to_u16(const char *string);
extern char *u16_to_str(char *out, size_t outlen, u16 num);
extern char *string_zero_pad(char *out, size_t outlen, const char *str, u16 pad_size);
extern char *u16_to_hex(char *out, size_t outlen, u16 num);
extern char *string_reverse(char *str);
extern char *string_lower(char *str);

#ifdef __MINGW32__
extern char *strtok_r(char *newstring, const char *delimiters, char **save_ptr);
#endif
