set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(CUPS_SRC_DIR ${SRC_DIR}/cups-2.1.1)
set(UNICODE_SRC_DIR ${SRC_DIR}/unicode)

find_library(CUPS_LIB cups REQUIRED)
find_library(CUPSIMAGE_LIB cupsimage REQUIRED)

add_executable(rastertokpsl
        rastertokpsl.c
        halfton.c
        libjbig/jbig.c
        libjbig/jbig_ar.c
        unicode/ConvertUTF.c
        main.c)
set_target_properties(rastertokpsl PROPERTIES CXX_STANDARD 20
        C_STANDARD 17)
target_include_directories(rastertokpsl PRIVATE ${CUPS_SRC_DIR})

target_link_libraries(rastertokpsl ${CUPS_LIB} ${CUPSIMAGE_LIB})

# See https://github.com/tensorflow/tensorflow/issues/1171
target_link_options(rastertokpsl PRIVATE "-lm")