cmake_minimum_required(VERSION 3.25)

project(
    kyocera_drivers
    VERSION 1.0.0
    DESCRIPTION
        "A revered engineered Kyocera tool to convert raster images to KPSL format"
    LANGUAGES C CXX
)
option(BUILD_TESTS "Build tests" ON)
option(
    INSTALL_ORIGINAL_PROPRIETARY_DRIVERS
    "Install origianl proprietary drivers"
    OFF
)

set(CPACK_COMPONENTS_ALL runtime devel)
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Kyocera CUPS raster filter (runtime)")
set(CPACK_COMPONENT_DEVEL_DESCRIPTION
    "Development files for Kyocera raster filter"
)
set(CPACK_COMPONENT_DEVEL_DEPENDS runtime)
set(CPACK_PACKAGE_CONTACT "glebajk@gmail.com")

include(CPack)

list(APPEND CMAKE_PREFIX_PATH cmake)

find_package(clang_format CONFIG REQUIRED)
find_package(clang_tidy CONFIG REQUIRED)
find_package(cpplint CONFIG REQUIRED)

find_package(kyocera_ppd CONFIG REQUIRED)

if(INSTALL_ORIGINAL_PROPRIETARY_DRIVERS)
    add_subdirectory(original)
else()
    add_subdirectory(src)
    if(BUILD_TESTS)
        include(CTest)
        add_subdirectory(tests)
    endif()
endif()