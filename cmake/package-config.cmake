set(CPACK_PACKAGE_DESCRIPTION_FILE "${CMAKE_SOURCE_DIR}/description.txt")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_SOURCE_DIR}/license")
set(CPACK_RESOURCE_FILE_README "${CMAKE_SOURCE_DIR}/readme.md")
set(CPACK_RESOURCE_FILE_WELCOME "${CMAKE_SOURCE_DIR}/readme.md")
set(CPACK_PACKAGE_CONTACT "glebajk@gmail.com")

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
