cmake_minimum_required(VERSION 3.25)

find_library(CUPS_LIB cups REQUIRED)
find_library(CUPSIMAGE_LIB cupsimage REQUIRED)

add_subdirectory(unicode)

add_executable(rastertokpsl rastertokpsl.c
        halfton.c
        libjbig/jbig.c
        libjbig/jbig_ar.c
        main.c)

set_target_properties(rastertokpsl PROPERTIES CXX_STANDARD 20
        C_STANDARD 17)
target_include_directories(rastertokpsl SYSTEM AFTER PRIVATE cups-2.1.1)

target_link_libraries(rastertokpsl PRIVATE ${CUPS_LIB} ${CUPSIMAGE_LIB} unicode)

target_compile_options(rastertokpsl PRIVATE
        # TODO: add -Werror
        $<$<CXX_COMPILER_ID:GNU>:-Wall -Wextra -Wpedantic>
        $<$<OR:$<CXX_COMPILER_ID:Clang>,$<CXX_COMPILER_ID:AppleClang>>:-Wall -Wextra -Wpedantic>
)

# See https://github.com/tensorflow/tensorflow/issues/1171
target_link_options(rastertokpsl PRIVATE "-lm")