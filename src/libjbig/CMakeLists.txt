add_library(libjbig OBJECT)

target_sources(libjbig PRIVATE jbig.c
        jbig85.c
        jbig_ar.c)
target_include_directories(libjbig SYSTEM AFTER PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_options(libjbig PRIVATE -g -O -W -Wall -ansi -pedantic)

add_executable(tstcodec tstcodec.c)
target_link_libraries(tstcodec PRIVATE libjbig)

add_executable(tstcodec85 tstcodec85.c)
target_link_libraries(tstcodec PRIVATE libjbig)

