cmake_minimum_required(VERSION 2.6)

add_test(ui_string check_ui_string args)

link_libraries(check)  # this shouldn't link to nagi exe

ADD_EXECUTABLE(check_ui_string  check_ui_string.c ../ui/string.c )
#target_link_libraries(check_ui_string check xml2)
