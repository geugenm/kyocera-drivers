cmake_minimum_required(VERSION 3.25)

add_executable(test_rastertokpsl
        main.c
        example.c
        test_unicode.c
        test_halfton.c)

target_link_libraries(test_rastertokpsl PRIVATE cups cupsimage unicode)
target_include_directories(test_rastertokpsl PRIVATE ${CMAKE_SOURCE_DIR}/src)

target_link_options(test_rastertokpsl PRIVATE "-lm")