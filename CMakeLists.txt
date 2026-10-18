cmake_minimum_required(VERSION 3.31 FATAL_ERROR)

project(
    kyocera_drivers
    VERSION 1.1.4
    DESCRIPTION "Legacy Kyocera GDI printer drivers for modern Linux CUPS"
    HOMEPAGE_URL "https://github.com/e-gleba/kyocera-drivers"
    LANGUAGES NONE
)

# Nothing is compiled and no language is enabled, so GNUInstallDirs cannot
# derive libdir from a target architecture. Pin it to the plain FHS default
# instead: configuring this packaging-only project must not require a C
# compiler.
set(CMAKE_INSTALL_LIBDIR
    "lib"
    CACHE PATH "Install path for CMake package files (architecture-independent)"
)
include(GNUInstallDirs)

# ------------------------------------------------------------------
# Platform gate
# ------------------------------------------------------------------
if(NOT CMAKE_SYSTEM_NAME STREQUAL "Linux")
    message(
        FATAL_ERROR
        "This project supports Linux only. Detected: ${CMAKE_SYSTEM_NAME}"
    )
endif()

# ------------------------------------------------------------------
# Host architecture
# ------------------------------------------------------------------
# Kyocera ships the proprietary rastertokpsl filter for x86 only. The
# x86 binary installs everywhere: natively on 32-bit x86 and through
# qemu-i386 on ARM (see the readme's ARM section); the x86_64 binary
# installs on x86_64 hosts only.
string(TOLOWER "${CMAKE_SYSTEM_PROCESSOR}" KYOCERA_ARCH)

if(KYOCERA_ARCH MATCHES "^(x86_64|amd64|x64)$")
    set(KYOCERA_X86_64 ON)
elseif(KYOCERA_ARCH MATCHES "^(i[3-6]86|x86)$")
    set(KYOCERA_X86_32 ON)
endif()

# ------------------------------------------------------------------
# Options
# ------------------------------------------------------------------
# NOTE: Kyocera has transitioned to a universal driver model and no longer
# provides per-model manual PPD files through the legacy download endpoints.
# As a result, automated PPD fetching is currently unsupported and disabled
# by default. Use bundled PPDs in the ppd/ directory or obtain drivers
# directly from the manufacturer.
option(DOWNLOAD_PPDS "Download and install Kyocera PPD files" OFF)

# ------------------------------------------------------------------
# CUPS paths — kept independent of GNUInstallDirs libdir because CUPS
# hardcodes lib/cups/filter on nearly every distribution.
# ------------------------------------------------------------------
set(KYOCERA_CUPS_FILTER_DIR
    "lib/cups/filter"
    CACHE PATH "Install directory for CUPS filters (relative to prefix)"
)

set(KYOCERA_CUPS_MODEL_DIR
    "share/cups/model"
    CACHE PATH "Install directory for CUPS PPD models (relative to prefix)"
)

# ------------------------------------------------------------------
# Subdirectories
# ------------------------------------------------------------------
list(APPEND CMAKE_PREFIX_PATH "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

if(DOWNLOAD_PPDS)
    find_package(download_kyocera_ppd CONFIG REQUIRED)
else()
    add_subdirectory(ppd SYSTEM)
endif()

add_subdirectory(proprietary SYSTEM)

# ------------------------------------------------------------------
# CMake package config — enables find_package(kyocera_drivers CONFIG REQUIRED)
# ------------------------------------------------------------------
include(CMakePackageConfigHelpers)

set(KYOCERA_FILTER_INSTALL_DIR ${KYOCERA_CUPS_FILTER_DIR})
set(KYOCERA_PPD_INSTALL_DIR   ${KYOCERA_CUPS_MODEL_DIR})

configure_package_config_file(
    "${CMAKE_CURRENT_SOURCE_DIR}/cmake/kyocera_driversConfig.cmake.in"
    "${CMAKE_CURRENT_BINARY_DIR}/kyocera_driversConfig.cmake"
    INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kyocera_drivers
    PATH_VARS KYOCERA_FILTER_INSTALL_DIR KYOCERA_PPD_INSTALL_DIR
)

write_basic_package_version_file(
    "${CMAKE_CURRENT_BINARY_DIR}/kyocera_driversConfigVersion.cmake"
    VERSION ${PROJECT_VERSION}
    COMPATIBILITY SameMajorVersion
)

install(
    FILES
        "${CMAKE_CURRENT_BINARY_DIR}/kyocera_driversConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/kyocera_driversConfigVersion.cmake"
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/kyocera_drivers
)

# ------------------------------------------------------------------
# Documentation, icons, desktop integration
# ------------------------------------------------------------------
install(
    FILES
        "${CMAKE_CURRENT_SOURCE_DIR}/readme.md"
        "${CMAKE_CURRENT_SOURCE_DIR}/license"
        "${CMAKE_CURRENT_SOURCE_DIR}/package/description.txt"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
)

install(
    PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/scripts/verify.sh"
    DESTINATION ${CMAKE_INSTALL_DOCDIR}
    COMPONENT runtime
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/package/icon.png"
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/pixmaps
)

install(
    FILES "${CMAKE_CURRENT_SOURCE_DIR}/package/kyocera_drivers.desktop"
    DESTINATION ${CMAKE_INSTALL_DATAROOTDIR}/applications
)

# ------------------------------------------------------------------
# CPack — DEB / RPM / TGZ packaging (single package, no components)
# ------------------------------------------------------------------
if(PROJECT_IS_TOP_LEVEL)
    set(CPACK_PACKAGE_NAME "${PROJECT_NAME}")
    set(CPACK_PACKAGE_VENDOR "e-gleba")
    set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_CURRENT_SOURCE_DIR}/package/description.txt")
    set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
        "Legacy Kyocera GDI printer drivers for modern Linux CUPS"
    )
    set(CPACK_PACKAGE_CONTACT "glebajk@gmail.com")
    set(CPACK_PACKAGE_ICON "${CMAKE_CURRENT_SOURCE_DIR}/package/icon.png")
    set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/license")
    set(CPACK_RESOURCE_FILE_README  "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")
    set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_CURRENT_SOURCE_DIR}/readme.md")

    # Stage every generator under /usr. DEB and RPM default to it anyway;
    # without this TGZ alone would stay root-relative (lib/, share/) and
    # extract to the wrong places. `tar -C /` then matches .deb/.rpm.
    set(CPACK_PACKAGING_INSTALL_PREFIX "/usr")

    # ------------------------------------------------------------------
    # Package architecture — derived from the host processor instead of
    # hardcoding x86_64/amd64.
    # ------------------------------------------------------------------
    if(KYOCERA_X86_64)
        set(KYOCERA_RPM_ARCH "x86_64")
        set(KYOCERA_DEB_ARCH "amd64")
    elseif(KYOCERA_X86_32)
        set(KYOCERA_RPM_ARCH "i686")
        set(KYOCERA_DEB_ARCH "i386")
    elseif(KYOCERA_ARCH MATCHES "^(aarch64|arm64)$")
        set(KYOCERA_RPM_ARCH "aarch64")
        set(KYOCERA_DEB_ARCH "arm64")
    elseif(KYOCERA_ARCH MATCHES "^(arm|armhf|armv[5-8](l|hl)?)$")
        set(KYOCERA_RPM_ARCH "armv7hl")
        set(KYOCERA_DEB_ARCH "armhf")
    else()
        message(
            WARNING
            "Unknown architecture '${CMAKE_SYSTEM_PROCESSOR}' — packages will be labeled with it as-is"
        )
        set(KYOCERA_RPM_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
        set(KYOCERA_DEB_ARCH "${CMAKE_SYSTEM_PROCESSOR}")
    endif()

    # ------------------------------------------------------------------
    # RPM
    # ------------------------------------------------------------------
    set(CPACK_RPM_PACKAGE_DESCRIPTION
        "Legacy Kyocera GDI printer drivers for modern Linux CUPS: original per-model PPDs, the proprietary rastertokpsl filter, and a multi-architecture wrapper (native x86, qemu-i386 on ARM)."
    )
    set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
    set(CPACK_RPM_PACKAGE_GROUP "System/Printing")
    set(CPACK_RPM_PACKAGE_ARCHITECTURE "${KYOCERA_RPM_ARCH}")

    # Self-contained package: disable auto-dependency scanning so build-host
    # SONAMEs (e.g. Debian's libcups.so.2) never leak into Requires, and
    # declare the single real runtime dependency manually.
    set(CPACK_RPM_PACKAGE_AUTOREQPROV "no")
    set(CPACK_RPM_PACKAGE_REQUIRES "cups")

    # ALT Linux build hosts run verify-elf on every packaged binary and fail
    # on the bundled 32-bit proprietary filter. Skip ELF verification; the
    # macro is ALT-only, so inject it only when building on ALT.
    # https://www.altlinux.org/Brp
    if(EXISTS "/etc/altlinux-release")
        set(CPACK_RPM_SPEC_MORE_DEFINE "%set_verify_elf_method skip")
    endif()

    # ------------------------------------------------------------------
    # DEB
    # ------------------------------------------------------------------
    set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
        "Legacy Kyocera GDI printer drivers for modern Linux CUPS: original per-model PPDs, the proprietary rastertokpsl filter, and a multi-architecture wrapper (native x86, qemu-i386 on ARM)."
    )
    set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
    set(CPACK_DEBIAN_PACKAGE_MAINTAINER "glebajk@gmail.com")
    set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "${KYOCERA_DEB_ARCH}")

    # rastertokpsl_* link libcupsimage.so.2, which `cups` does not pull in
    # on Debian 13. The t64 name is trixie+/noble+; the plain name covers
    # older releases.
    set(CPACK_DEBIAN_PACKAGE_DEPENDS "cups, libc6, libcupsimage2t64 | libcupsimage2")

    include(CPack)
endif()
