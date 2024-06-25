cmake_minimum_required(VERSION 3.25)

find_library(cups cups REQUIRED)
find_library(cupsimage cupsimage REQUIRED)

add_subdirectory(unicode)
add_subdirectory(libjbig)

add_executable(rastertokpsl main.c rastertokpsl.c halfton.c)

target_include_directories(rastertokpsl SYSTEM AFTER PUBLIC cups-2.1.1)
target_include_directories(rastertokpsl PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_link_libraries(rastertokpsl PRIVATE cupsimage cups unicode libjbig)

# See https://github.com/tensorflow/tensorflow/issues/1171
target_link_options(rastertokpsl PRIVATE "-lm")