#ifndef NAGI_STATE_INFO_H
#define NAGI_STATE_INFO_H

extern char save_description[0x20];

extern VSTRING *save_dir;
extern VSTRING *save_filename;

extern u16 state_get_info(u8 state_type);

#endif /* NAGI_STATE_INFO_H */
