
// res_dir.c
extern u8 dir_id[10];
extern void dir_load(void);
extern u8 *dir_logic(u16 num);
extern u8 *dir_view(u16 num);
extern u8 *dir_picture(u16 num);
extern u8 *dir_sound(u16 num);

// res_vol.c

extern u16 free_mem_check;
extern u16 res_size;
extern u16 not_compressed;

extern u8 *vol_res_load(u8 *dir_entry, u8 *buff);
extern u8 *v2_res_load(u8 *dir_entry, u8 *buff);
extern u8 *v3_res_load(u8 *dir_entry, u8 *buff);
extern void err_insert_disk(u16 num);
extern void err_msg(u8 *msg, u16 num);
extern u16 err_wrong_disk(u16 num);
extern void volumes_open(void);
extern void volumes_close(void);
extern u8 *file_load(u8 *name, u8 *buff);

// res_lzw.c
struct dict_struct
{
	u16 prev;
	u8 ascii;
};
typedef struct dict_struct DICT;
	
extern DICT *lzw_dict;

u16 lzw_decompress(FILE *cfile, u8 *cbuff, u16 fsize, u8 *ubuff, u16 usize);

// res_pic.c
u16 pic_decompress(FILE *cfile, u8 *cbuff, u16 cfile_size, u8 *buff, u16 cbuff_size);
