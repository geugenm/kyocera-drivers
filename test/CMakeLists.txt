cmake_minimum_required(VERSION 3.25)

add_executable(test-rastertokpsl
        main.c
        example.c
        test_unicode.c
        test_halfton.c)

target_link_libraries(test-rastertokpsl PRIVATE rastertokpsl_base cups cupsimage unicode)