#ifndef NAGI_RES_RES_H
#define NAGI_RES_RES_H

// res_dir.c

extern void dir_load(void);
extern void dir_unload(void);
extern u8 *dir_logic(u16 num);
extern u8 *dir_view(u16 num);
extern u8 *dir_picture(u16 num);
extern u8 *dir_sound(u16 num);

// res_vol.c


extern u16 free_mem_check;
extern int res_size;
extern u16 not_compressed;

extern u8 *vol_res_load(u8 *dir_entry, u8 *buff);
extern void err_msg(u8 *msg, u16 num);
extern void volumes_close(void);
extern u8 *file_load(u8 *name, u8 *buff);

// res_lzw.c

extern void lzw_init(void);
extern void lzw_shutdown(void);
extern u16 lzw_decompress(FILE *cfile, u8 *cbuff, u16 fsize, u8 *ubuff, u16 usize);

// res_pic.c
u16 pic_decompress(FILE *cfile, u8 *cbuff, u16 cfile_size, u8 *buff, u16 cbuff_size);

#endif /* NAGI_RES_RES_H */
