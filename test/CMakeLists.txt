set(SRC_DIR ${PROJECT_SOURCE_DIR}/src)
set(CUPS_SRC_DIR ${SRC_DIR}/cups-2.1.1)
set(UNICODE_SRC_DIR ${SRC_DIR}/unicode)

find_library(CUPS_LIB cups REQUIRED)
find_library(CUPSIMAGE_LIB cupsimage REQUIRED)

include_directories(${SRC_DIR} ${CUPS_SRC_DIR} ${UNICODE_SRC_DIR})

add_executable(test-rastertokpsl
        example.c
        test_unicode.c
        ${UNICODE_SRC_DIR}/ConvertUTF.c
        test_halfton.c
        # ${SRC_DIR}/halfton.c
        main.c)
# add_definitions ("-Wundefined-internal")
# target_link_libraries(rastertokpsl-re ${CUPS_LIB} ${CUPSIMAGE_LIB})