add_executable(nagi)

set_target_properties(nagi PROPERTIES
    C_STANDARD 11
    C_EXTENSIONS NO
)

# All include files references are relative to the source file (not ideal)
# target_include_directories(nagi PRIVATE .)

set(base_sources 
    agi.c
    agi.h
    agi_string.c
    agi_string.h
    base.c
    base.h
    cmd_misc.c
    cmd_misc.h
    config.c
    config.h
    debug.c
    debug.h
    decrypt.c
    decrypt.h
    flags.c
    flags.h
    game_id.c
    game_id.h
    initialise.c
    initialise.h
    list.c
    list.h
    log.c
    log.h
    nagi_main.c
    new_room.c
    new_room.h
    obj_show.c
    obj_show.h
    objects.c
    objects.h
    resource.h
    state_info.c
    state_info.h
    state_io.c
    state_io.h
    # template_code.c
    # template_header.h
    trace.c
    trace.h
)

set(logic_sources 
    logic/arithmetic.c
    logic/arithmetic.h
    logic/cmd_table.c
    logic/cmd_table.h
    logic/logic_base.c
    logic/logic_base.h
    logic/logic_eval.c
    logic/logic_eval.h
    logic/logic_execute.c
    logic/logic_execute.h
)

set(picture_sources 
    picture/pic_add.c
    picture/pic_add.h
    picture/pic_render.c
    picture/pic_render.h
    picture/pic_res.c
    picture/pic_res.h
    picture/sbuf_util.c
    picture/sbuf_util.h
)

set(res_sources 
    res/res.h
    res/res_dir.c
    res/res_lzw.c
    res/res_pic.c
    res/res_vol.c
)

set(sound_sources 
    sound/pcm_out.c
    sound/pcm_out.h
    sound/pcm_out_sdl.c
    sound/pcm_out_sdl.h
    sound/sound.h
    sound/sound_base.c
    sound/sound_base.h
    sound/sound_gen.c
    sound/sound_gen.h
    # sound/sound_sample.c
    # sound/sound_sdl.c
    sound/tone.c
    sound/tone.h
    sound/tone_pcm.c
    sound/tone_pcm.h
)

set(video_sources
    sys/chargen.c
    sys/chargen.h
    # sys/drv_dummy.c
    # sys/drv_dummy.h
    # sys/drv_sdl.c
    # sys/drv_sdl.h
    # sys/drv_video.c
    # sys/drv_video.h
    sys/gfx.c
    sys/gfx.h
    sys/sdl_vid.c
    sys/sdl_vid.h
    sys/vid_render.c
    sys/vid_render.h
)

set(sys_sources 
    sys/agi_file.c
    sys/agi_file.h
    sys/delay.c
    sys/delay.h
    sys/drvpick.c
    sys/drvpick.h
    sys/endian.c
    sys/endian.h
    sys/error.c
    sys/error.h
    sys/glob_sys.c
    sys/glob_sys.h
    sys/ini_config.c
    sys/ini_config.h
    # sys/lib_wrapper.c
    # sys/lib_wrapper.h
    sys/mem_wrap.c
    sys/mem_wrap.h
    sys/memory.c
    sys/memory.h
    sys/rand.c
    sys/rand.h
    sys/script.c
    sys/script.h
    sys/sys_dir.c
    sys/sys_dir.h
    sys/time.c
    sys/time.h
    sys/vstring.c
    sys/vstring.h
)

set(ui_sources 
    ui/agi_text.c
    ui/agi_text.h
    ui/cmd_input.c
    ui/cmd_input.h
    ui/controller.c
    ui/controller.h
    ui/events.c
    ui/events.h
    ui/list_box.c
    ui/list_box.h
    ui/menu.c
    ui/menu.h
    ui/mouse.c
    ui/mouse.h
    ui/msg.c
    ui/msg.h
    ui/parse.c
    ui/parse.h
    ui/printf.c
    ui/printf.h
    ui/status.c
    ui/status.h
    ui/string.c
    ui/string.h
    ui/window.c
    ui/window.h
)

set(version_sources 
    version/agi_crc.c
    version/agi_crc.h
    version/standard.c
    version/standard.h
)

set(view_sources 
    view/obj_base.c
    view/obj_base.h
    view/obj_blit.c
    view/obj_blit.h
    view/obj_block.c
    view/obj_block.h
    view/obj_cycle.c
    view/obj_cycle.h
    view/obj_drawerase.c
    view/obj_drawerase.h
    view/obj_loop.c
    view/obj_loop.h
    view/obj_motion.c
    view/obj_motion.h
    view/obj_motion_cmd.c
    view/obj_motion_cmd.h
    view/obj_picbuff.c
    view/obj_picbuff.h
    view/obj_position.c
    view/obj_position.h
    view/obj_priority.c
    view/obj_priority.h
    view/obj_proximity.c
    view/obj_proximity.h
    view/obj_update.c
    view/obj_update.h
    view/view_base.c
    view/view_base.h
)

target_sources(nagi
    PRIVATE
    ${base_sources} 
    ${logic_sources} 
    ${picture_sources} 
    ${res_sources} 
    ${sound_sources} 
    ${sys_sources} 
    ${ui_sources} 
    ${version_sources} 
    ${video_sources}
    ${view_sources} 
)

target_link_libraries(nagi PRIVATE 
    SDL2::SDL2
    SDL2::SDL2main
)

set_target_properties(nagi PROPERTIES
    RUNTIME_OUTPUT_NAME nagi
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
)

if (WIN32)
    target_sources(ags PRIVATE nagi.rc)
    set_target_properties(ags PROPERTIES
        WIN32_EXECUTABLE TRUE
    )
endif()


# One-off tools

# Hex dump tool?
# add_executable(read_agidata read_agidata.c)
# set_target_properties(read_agidata PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
# )

# Used to convert fonts stored as C code to a binary file.
# add_executable(savefont tools/savefont.c)
# set_target_properties(savefont PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}
# )
