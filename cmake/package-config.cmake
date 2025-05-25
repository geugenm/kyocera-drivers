set(CPACK_PACKAGE_NAME "kyocera_drivers")
set(CPACK_PACKAGE_VENDOR "Kyocera Reverse Engineering Team")
set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/description.txt")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY
    "High-performance Kyocera raster-to-KPSL conversion toolkit"
)
set(CPACK_PACKAGE_CONTACT "glebajk@gmail.com")
set(CPACK_PACKAGE_ICON "${CMAKE_SOURCE_DIR}/icon.png")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/readme.md")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/readme.md")

# RPM/DEB specific
set(CPACK_RPM_PACKAGE_DESCRIPTION
    "High-performance Kyocera raster-to-KPSL conversion toolkit. See README.md for details."
)
set(CPACK_RPM_PACKAGE_LICENSE "GPLv2")
set(CPACK_RPM_PACKAGE_GROUP "System/Printing")
set(CPACK_RPM_PACKAGE_ARCHITECTURE "x86_64")
set(CPACK_DEBIAN_PACKAGE_DESCRIPTION
    "High-performance Kyocera raster-to-KPSL conversion toolkit. See README.md for details."
)
set(CPACK_DEBIAN_PACKAGE_SECTION "utils")
set(CPACK_DEBIAN_PACKAGE_MAINTAINER "glebajk@gmail.com")
set(CPACK_DEBIAN_PACKAGE_ARCHITECTURE "amd64")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "cups, libc6")

include(CPack)

cpack_add_component(
    documentation
    DISPLAY_NAME "Documentation"
    DESCRIPTION "Project README and documentation"
    GROUP "Documentation"
    REQUIRED
)

install(
    FILES
        "${CMAKE_SOURCE_DIR}/readme.md"
        "${CMAKE_SOURCE_DIR}/license"
        "${CMAKE_SOURCE_DIR}/description.txt"
    DESTINATION "share/doc/${PROJECT_NAME}"
    COMPONENT documentation
    PERMISSIONS OWNER_READ GROUP_READ WORLD_READ
)

install(
    FILES "${CMAKE_SOURCE_DIR}/icon.png"
    DESTINATION "share/pixmaps"
    COMPONENT documentation
)

install(
    FILES "${CMAKE_SOURCE_DIR}/kyocera_drivers.desktop"
    DESTINATION "share/applications"
    COMPONENT documentation
)
