cmake_minimum_required(VERSION 3.25)

find_library(cups cups REQUIRED)
find_library(cupsimage cupsimage REQUIRED)

add_subdirectory(unicode)
add_subdirectory(libjbig)

add_library(rastertokpsl_core STATIC rastertokpsl.cxx halfton.c)
target_link_libraries(rastertokpsl_core PRIVATE cupsimage cups unicode libjbig)
target_include_directories(rastertokpsl_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})

add_executable(rastertokpsl main.cxx)

target_link_libraries(rastertokpsl PRIVATE rastertokpsl_core)

# See https://github.com/tensorflow/tensorflow/issues/1171
target_link_options(rastertokpsl PRIVATE "-lm")