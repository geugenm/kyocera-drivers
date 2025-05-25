cmake_minimum_required(VERSION 3.25)

project(
    kyocera_drivers
    VERSION 1.1.0
    DESCRIPTION "High-performance Kyocera raster-to-KPSL conversion toolkit"
    HOMEPAGE_URL "https://github.com/geugenm/kyocera-drivers"
    LANGUAGES C CXX
)

option(BUILD_TESTS "Enable building of test suite" ON)
option(
    INSTALL_ORIGINAL_PROPRIETARY_DRIVERS
    "Install official proprietary Kyocera drivers"
    OFF
)
option(DOWNLOAD_PPDS "Download and install Kyocera PPD files" OFF)

list(APPEND CMAKE_PREFIX_PATH cmake)

find_package(package CONFIG REQUIRED)

find_package(clang_format CONFIG REQUIRED)
find_package(clang_tidy CONFIG REQUIRED)
find_package(clang_doc CONFIG REQUIRED)
find_package(cpplint CONFIG REQUIRED)

if(DOWNLOAD_PPDS)
    find_package(kyocera_ppd CONFIG REQUIRED)
endif()

if(INSTALL_ORIGINAL_PROPRIETARY_DRIVERS)
    add_subdirectory(proprietary SYSTEM)
else()
    add_subdirectory(src)

    if(BUILD_TESTS)
        include(CTest)
        add_subdirectory(tests)
    endif()
endif()

