cmake_minimum_required(VERSION 2.6)

enable_testing()

project(nagi C)

# for debug
set(CMAKE_C_FLAGS_DEBUG   "-O0 -g3 -ggdb3")
set(CMAKE_C_FLAGS_RELEASE "-O2")

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DRAD_LINUX -DRAD_WARN")
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Winline -Wshadow -Wstrict-prototypes -W")

set ( SOURCES 

    #base
    base.c
    debug.c
    decrypt.c
    flags.c
    nagi_main.c
    initialise.c
    agi_string.c
    new_room.c
    objects.c
    state_io.c
    agi.c
    state_info.c
    obj_show.c
    game_id.c
    cmd_misc.c
    trace.c
    config.c
    list.c
    log.c

    #version
    version/agi_crc.c
    version/standard.c

    #resource
    res/res_dir.c
    res/res_vol.c
    res/res_lzw.c
    res/res_pic.c

    #sys
    sys/script.c
    sys/error.c
    sys/glob_sys.c
    sys/delay.c
    sys/memory.c
    sys/rand.c
    sys/endian.c
    sys/time.c
    sys/mem_wrap.c
    sys/sys_dir.c
    sys/vstring.c
    sys/ini_config.c

    #video
    sys/gfx.c
    sys/vid_render.c
    sys/sdl_vid.c
    sys/chargen.c
    sys/agi_file.c

    #pic
    picture/pic_render.c
    picture/pic_res.c
    picture/sbuf_util.c
    picture/pic_add.c

    #logic
    logic/arithmetic.c
    logic/cmd_table.c
    logic/logic_base.c
    logic/logic_eval.c
    logic/logic_execute.c

    #view
    view/view_base.c
    view/obj_update.c
    view/obj_blit.c
    view/obj_base.c
    view/obj_proximity.c
    view/obj_position.c
    view/obj_loop.c
    view/obj_motion.c
    view/obj_drawerase.c
    view/obj_cycle.c
    view/obj_motion_cmd.c
    view/obj_block.c
    view/obj_priority.c
    view/obj_picbuff.c

    #sound
    sound/sound_base.c
    sound/pcm_out.c
    sound/pcm_out_sdl.c
    sound/sound_gen.c
    sound/tone.c
    sound/tone_pcm.c
    sys/drvpick.c

    #ui
    ui/parse.c
    ui/window.c
    ui/printf.c
    ui/agi_text.c
    ui/string.c
    ui/msg.c
    ui/events.c
    ui/cmd_input.c
    ui/controller.c
    ui/status.c
    ui/menu.c
    ui/mouse.c
    ui/list_box.c
)


# from test case here: http://www.cmake.org/Bug/view.php?id=4068
if(MINGW)
    # resource compilation for mingw
    ADD_CUSTOM_COMMAND(
        OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/nagires.o
        COMMAND i586-mingw32msvc-windres -I${CMAKE_CURRENT_SOURCE_DIR}
            -i${CMAKE_CURRENT_SOURCE_DIR}/nagi.rc
            -o ${CMAKE_CURRENT_BINARY_DIR}/nagires.o
        )
    SET(SOURCES ${SOURCES} ${CMAKE_CURRENT_BINARY_DIR}/nagires.o)
endif(MINGW)

find_package( SDL REQUIRED )
#find_package( SDL_image REQUIRED )

if ( NOT SDL_FOUND )
   message ( FATAL_ERROR "SDL not found!" )
endif ( NOT SDL_FOUND )

INCLUDE_DIRECTORIES(${SDL_INCLUDE_DIR}) 

add_subdirectory( test )

ADD_EXECUTABLE(nagi ${SOURCES} )

target_link_libraries(nagi
    ${SDL_LIBRARY}
    # ${SDLIMAGE_LIBRARY} # if using SDL_image, obviously
    # SDLmain # Sadly not included in SDL_LIBRARY variable
)

