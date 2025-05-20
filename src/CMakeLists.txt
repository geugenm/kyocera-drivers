cmake_minimum_required(VERSION 3.25)

find_library(cups cups REQUIRED)
find_library(cupsimage cupsimage REQUIRED)

add_subdirectory(unicode)
add_subdirectory(libjbig)

add_library(rastertokpsl_core STATIC rastertokpsl.c halfton.c)
target_link_libraries(rastertokpsl_core PRIVATE cupsimage cups unicode libjbig)
target_include_directories(rastertokpsl_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

add_executable(rastertokpsl main.c)
target_link_libraries(rastertokpsl PRIVATE rastertokpsl_core)

# /usr/lib64/libm.so.6: error adding symbols: DSO missing from command line
target_link_libraries(rastertokpsl PRIVATE m)

install(
    TARGETS rastertokpsl
    ARCHIVE DESTINATION /usr/lib/cups/filter
    RUNTIME DESTINATION /usr/lib/cups/filter
)
