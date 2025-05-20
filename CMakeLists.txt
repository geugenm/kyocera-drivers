cmake_minimum_required(VERSION 3.25)

project(
    raster_to_kpsl
    VERSION 1.0.0
    DESCRIPTION
        "A revered engineered Kyocera tool to convert raster images to KPSL format"
    LANGUAGES C CXX
)

list(APPEND CMAKE_PREFIX_PATH cmake/cfg)

find_package(clang_format CONFIG REQUIRED)
find_package(clang_tidy CONFIG REQUIRED)
find_package(cpplint CONFIG REQUIRED)

add_subdirectory(src)
add_subdirectory(scripts)

option(BUILD_TESTS "Build tests" ON)
if(BUILD_TESTS)
    include(CTest)
    add_subdirectory(tests)
endif()