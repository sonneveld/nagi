# add base files
SRC := base.c debug.c decrypt.c flags.c nagi_main.c\
initialise.c agi_string.c new_room.c objects.c state_io.c agi.c\
state_info.c obj_show.c  game_id.c cmd_misc.c\
trace.c

# version
SRC += version/ver_init.c version/agi_crc.c version/gamelist.c

# resource files
SRC += res/res_dir.c res/res_vol.c res/res_lzw.c res/res_pic.c

# sys files
SRC += sys/script.c sys/error.c sys/glob_sys.c\
sys/delay.c  sys/memory.c sys/rand.c sys/endian.c sys/time.c\
sys/mem_wrap.c sys/ini_config.c sys/sys_dir.c sys/vstring.c

# video files
SRC +=  sys/video.c sys/video_misc.c\
sys/vid_sdl.c sys/vid_dummy.c sys/vid_render.c\
sys/font_8x8.c sys/font_4x8.c sys/font_16x16.c

# pic files
SRC += picture/pic_render.c picture/pic_res.c\
picture/sbuf_util.c picture/pic_add.c

# new logic
SRC += logic/arithmetic.c logic/cmd_table.c logic/logic_base.c\
logic/logic_eval.c logic/logic_execute.c

# view files
SRC += view/view_base.c view/obj_update.c view/obj_blit.c\
view/obj_base.c  view/obj_proximity.c \
view/obj_position.c view/obj_loop.c view/obj_motion.c view/obj_drawerase.c \
view/obj_cycle.c view/obj_motion_cmd.c view/obj_block.c\
view/obj_priority.c view/obj_picbuff.c

# sound files
SRC += sound/sound_base.c sound/sound_sample.c sound/sound_sdl.c

# user input
SRC += ui/parse.c ui/window.c ui/printf.c ui/agi_text.c ui/string.c
SRC += ui/msg.c ui/events.c  ui/cmd_input.c ui/controller.c ui/status.c\
ui/menu.c ui/mouse.c

OBJ := $(SRC:.c=.o)

CC = gcc

CFLAGS := -O2 -fwritable-strings
#CFLAGS += -finline-functions
CFLAGS += -W -Winline -Wshadow -Wstrict-prototypes -Wpointer-arith -Wcast-qual 
#CFLAGS += -Werror
CFLAGS += -IC:/mingw/mingw32/include/SDL

LIB = -lmingw32 -lSDLmain -lSDL -mwindows -mconsole

n.exe: $(OBJ)
	$(CC) $(CFLAGS) -o $@ $(OBJ) $(LIB) 

#%.o: %.c
#	$(CC) $(CFLAGS) -c $< -o $@

include $(OBJ:.o=.d)

%.d: %.c
	sh depend.sh $(CFLAGS) $< > $@

clean:
	$(RM) $(OBJ) n.exe core


