#ifndef agi_crc_h_file
#define agi_crc_h_file

/* STRUCTURES	---	---	---	---	---	---	--- */
/* VARIABLES	---	---	---	---	---	---	--- */
/* FUNCTIONS	---	---	---	---	---	---	--- */
extern u32 crc_generate(u8 *data, int size);
extern int file_crc_compare(u8 *file_name, u32 crc_correct);
#endif