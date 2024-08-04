cmake_minimum_required(VERSION 3.25)

add_executable(test_rastertokpsl
               main.cxx
               test_unicode.cxx
               test_halfton.cxx)

target_link_libraries(test_rastertokpsl PRIVATE rastertokpsl_core cups cupsimage unicode)
target_include_directories(test_rastertokpsl PRIVATE ${CMAKE_CURRENT_LIST_DIR})

target_link_options(test_rastertokpsl PRIVATE "-lm")