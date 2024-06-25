cmake_minimum_required(VERSION 3.25)

add_executable(test-rastertokpsl
        example.c
        test_unicode.c
        test_halfton.c
        main.c)

target_link_libraries(test-rastertokpsl PRIVATE ${CUPS_LIB} ${CUPSIMAGE_LIB} unicode)

# TODO: Fix hardcoded paths
target_include_directories(test-rastertokpsl PRIVATE ../src ../src/cups-2.1.1)