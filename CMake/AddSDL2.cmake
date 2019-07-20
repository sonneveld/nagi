# SDL2 supplies an sdl2-config.cmake file, which lives in lib/cmake/SDL2/sdl2-config.cmake
# and should be picked up by cmake if configured correctly

find_package(SDL2 REQUIRED)

# Create an interface target to link to

add_library(sdl2_interface INTERFACE)
target_link_directories(sdl2_interface INTERFACE ${SDL2_LIBDIR})
target_include_directories(sdl2_interface INTERFACE ${SDL2_INCLUDE_DIRS})
target_link_libraries(sdl2_interface INTERFACE SDL2)
add_library(SDL2::SDL2 ALIAS sdl2_interface)

add_library(sdl2_main_interface INTERFACE)
target_link_directories(sdl2_main_interface INTERFACE ${SDL2_LIBDIR})
target_include_directories(sdl2_main_interface INTERFACE ${SDL2_INCLUDE_DIRS})
target_link_libraries(sdl2_main_interface INTERFACE SDL2main)
add_library(SDL2::SDL2main ALIAS sdl2_main_interface)
