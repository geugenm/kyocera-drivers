cmake_minimum_required(VERSION 3.25)

find_library(cups cups REQUIRED)
find_library(cupsimage cupsimage REQUIRED)

add_subdirectory(unicode SYSTEM)
add_subdirectory(libjbig SYSTEM)

add_library(halftone STATIC halfton.cxx)
target_include_directories(halftone PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(
    halftone
    PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)

add_library(rastertokpsl_core STATIC rastertokpsl.c)
target_link_libraries(
    rastertokpsl_core
    PRIVATE cupsimage cups unicode libjbig halftone
)
target_include_directories(rastertokpsl_core PUBLIC ${CMAKE_CURRENT_LIST_DIR})
set_target_properties(rastertokpsl_core PROPERTIES C_STANDARD 23)

add_executable(rastertokpsl main.cxx)
target_link_libraries(rastertokpsl PRIVATE rastertokpsl_core)
set_target_properties(
    rastertokpsl
    PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)

cpack_add_component(
    runtime
    DISPLAY_NAME "Runtime"
    DESCRIPTION "Kyocera CUPS raster filter (runtime component)"
    REQUIRED
)

cpack_add_component(
    devel
    DISPLAY_NAME "Development"
    DESCRIPTION "Development headers and libraries for Kyocera raster filter"
)

install(
    TARGETS rastertokpsl
    RUNTIME DESTINATION lib/cups/filter
    ARCHIVE DESTINATION lib/cups/filter
    LIBRARY
        DESTINATION lib/cups/filter
        COMPONENT runtime
        PERMISSIONS
            OWNER_READ
            OWNER_WRITE
            OWNER_EXECUTE
            GROUP_READ
            GROUP_EXECUTE
            WORLD_READ
            WORLD_EXECUTE
)

# Development headers (only with devel/dev component)
install(
    FILES
        ${CMAKE_CURRENT_LIST_DIR}/halfton.h
        ${CMAKE_CURRENT_LIST_DIR}/rastertokpsl.h
    DESTINATION include/rastertokpsl
    COMPONENT devel
)

# Optionally install static libs in devel
install(
    TARGETS rastertokpsl_core halftone
    ARCHIVE DESTINATION lib/rastertokpsl
    LIBRARY DESTINATION lib/rastertokpsl COMPONENT devel
)
