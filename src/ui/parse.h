#ifndef NAGI_UI_PARSE_H
#define NAGI_UI_PARSE_H

extern void parse(const char *string);
extern u8 *cmd_parse(u8 *c);


extern u16 word_num[10];
extern const char *word_string[10];
extern u16 word_total;	// bad word
extern u8 *words_tok_data;

#endif /* NAGI_UI_PARSE_H */
