struct sound_struct
{
	struct sound_struct *next;	// 0-1
	u16 num;		// 2-3
	u8 *data;		// 4-5
	u8 *channel[4];	// 6-7, 8-9, A-B, C-D
};
typedef struct sound_struct SOUND;

struct channel_struct
{
	u8 *data;
	u16 duration;
	u16 toggle;	// turned on (1)  but when the channel's data runs out, it's set to (0)
	u16 word1796;
	u8 attenuation;
	u8 attenuation_copy;
	u8 *dissolve;
	
	// for the sample mixer
	u16 freq_count;
	u16 freq_prev;
	u8 volume;	
	u16 sample_num;
	u8 *sample_cur;
	s32 count;
	s32 scale;
};
typedef struct channel_struct CHANNEL;

// must be mono 16bit 44100 for now
struct sample_struct
{
	u8 *data;
	int size;
	
	u16 freq_base;
	int *scale_table;
};
typedef struct sample_struct SAMPLE;

// --------- BASE



extern u16 sound_playing;
extern u16 sound_flag;

extern void sound_list_init(void);
extern void sound_list_new_room(void);
extern u8 *cmd_load_sound(u8 *c);
extern SOUND *sound_load(u16 snd_num);
extern u8 *cmd_sound(u8 *c);
extern u8 *cmd_stop_sound(u8 *c);
extern void sound_stop(void);

// ------ SAMPLE

extern SAMPLE *sample[2];
	
extern void sample_init(void);
extern void sample_denit(void);
extern void mix_new(u8 *stream, CHANNEL *c, int len);
extern void mix_add(u8 *stream, CHANNEL *c, int len);

// ------------- SDL




extern void sound_driver_init(void);
extern void sound_driver_denit(void);


void sound_new(SOUND *snd);
void sound_fill_buff(void *udata, u8 *stream, int len);
void sound_vector(void);
void sound_stop_sdl(void);
void sound_send(u16 freq);
void sound_volume(void);
SAMPLE *open_sample(u8 *file_name, int freq);
void sample_fill_chan(u8 *stream, int c,  int len);
void fill_sample_mix_audio(void *udata, u8 *stream, int len);

int snd_off;
u8 *sample_data;


//extern u8 nagi_sound_disable;