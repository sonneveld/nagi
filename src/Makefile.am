bin_PROGRAMS = $(top_builddir)/bin/nagi

# add base files
__top_builddir__bin_nagi_SOURCES = base.c debug.c decrypt.c flags.c nagi_main.c\
initialise.c agi_string.c new_room.c objects.c state_io.c agi.c\
state_info.c obj_show.c  game_id.c cmd_misc.c\
trace.c config.c list.c log.c

# version
__top_builddir__bin_nagi_SOURCES +=  version/agi_crc.c version/standard.c

#version/ver_init.c version/gamelist.c
# resource files
__top_builddir__bin_nagi_SOURCES += res/res_dir.c res/res_vol.c res/res_lzw.c res/res_pic.c

# sys files
__top_builddir__bin_nagi_SOURCES += sys/script.c sys/error.c sys/glob_sys.c\
sys/delay.c  sys/memory.c sys/rand.c sys/endian.c sys/time.c\
sys/mem_wrap.c  sys/sys_dir.c sys/vstring.c\
sys/ini_config.c

# video files
__top_builddir__bin_nagi_SOURCES += sys/gfx.c sys/vid_render.c\
 sys/sdl_vid.c \
sys/chargen.c sys/agi_file.c

# pic files
__top_builddir__bin_nagi_SOURCES += picture/pic_render.c picture/pic_res.c\
picture/sbuf_util.c picture/pic_add.c

# new logic
__top_builddir__bin_nagi_SOURCES += logic/arithmetic.c logic/cmd_table.c logic/logic_base.c\
logic/logic_eval.c logic/logic_execute.c

# view files
__top_builddir__bin_nagi_SOURCES += view/view_base.c view/obj_update.c view/obj_blit.c\
view/obj_base.c  view/obj_proximity.c\
view/obj_position.c view/obj_loop.c view/obj_motion.c view/obj_drawerase.c\
view/obj_cycle.c view/obj_motion_cmd.c view/obj_block.c\
view/obj_priority.c view/obj_picbuff.c

# sound files
__top_builddir__bin_nagi_SOURCES += sound/sound_base.c sound/pcm_out.c sound/pcm_out_sdl.c\
sound/sound_gen.c sound/tone.c sound/tone_pcm.c sys/drvpick.c

# user input
__top_builddir__bin_nagi_SOURCES += ui/parse.c ui/window.c ui/printf.c ui/agi_text.c ui/string.c
__top_builddir__bin_nagi_SOURCES += ui/msg.c ui/events.c  ui/cmd_input.c ui/controller.c ui/status.c\
ui/menu.c ui/mouse.c ui/list_box.c

