cmake_minimum_required(VERSION 3.25)

find_library(cups cups REQUIRED)
find_library(cupsimage cupsimage REQUIRED)

add_subdirectory(unicode)
add_subdirectory(libjbig)

add_library(rastertokpsl_core STATIC rastertokpsl.cxx halfton.cxx)

target_link_libraries(rastertokpsl_core PRIVATE cupsimage cups unicode libjbig)
target_include_directories(rastertokpsl_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(rastertokpsl_core PROPERTIES CXX_STANDARD 20)

add_executable(rastertokpsl main.cxx)
target_link_libraries(rastertokpsl PRIVATE rastertokpsl_core)