extern int ini_open(u8 *ini_given_name);
extern int ini_close(void);
extern u8 *ini_read(u8 *sect_name, u8 *key_name);

extern int ini_boolean(u8 *sect, u8 *sect_parent, u8 *key, int val_default);
extern int ini_int(u8 *sect, u8 *sect_parent, u8 *key, int val_default);
extern u8 *ini_string(u8 *sect, u8 *sect_parent, u8 *key);