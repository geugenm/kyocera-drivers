cmake_minimum_required(VERSION 3.25)

project(rastertokpsl
        VERSION 1.0.0
        DESCRIPTION "A revered engineered Kyocera tool to convert raster images to KPSL format"
        LANGUAGES C CXX)

list(APPEND CMAKE_PREFIX_PATH cmake/cfg)

find_package(clang_format CONFIG REQUIRED)
find_package(clang_tidy CONFIG REQUIRED)
find_package(cpplint CONFIG REQUIRED)

option(BUILD_TESTS "Build tests" ON)
option(USE_CPM "Use Cmake Package Manager" ON)

if (USE_CPM)
    include(cmake/CPM.cmake)
endif ()

add_subdirectory(src)

if (BUILD_TESTS)
    enable_testing()
    add_subdirectory(test)
endif ()