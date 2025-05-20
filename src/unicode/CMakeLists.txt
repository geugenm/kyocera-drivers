cmake_minimum_required(VERSION 3.20)

add_library(unicode OBJECT)

target_sources(unicode PRIVATE ConvertUTF.c)
target_include_directories(
    unicode
    SYSTEM
    AFTER
    PUBLIC ${CMAKE_CURRENT_LIST_DIR}
)
