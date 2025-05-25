cmake_minimum_required(VERSION 3.16)

# Ensure all required files exist before install
foreach(_file wrapper.sh rastertokpsl-x32 rastertokpsl-x64)
    if(NOT EXISTS "${CMAKE_CURRENT_LIST_DIR}/${_file}")
        message(
            FATAL_ERROR
            "Required file '${_file}' not found in ${CMAKE_CURRENT_LIST_DIR}"
        )
    endif()
endforeach()

cpack_add_component(
    proprietary_runtime
    DISPLAY_NAME "Proprietary Runtime"
    DESCRIPTION "Kyocera rastertokpsl wrapper and binaries"
    REQUIRED
)

# Install wrapper.sh as rastertokpsl (runtime component)
install(
    PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/wrapper.sh"
    DESTINATION lib/cups/filter
    RENAME rastertokpsl
    COMPONENT proprietary_runtime
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
)

# Install rastertokpsl-x32 binary (runtime component)
install(
    PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/rastertokpsl-x32"
    DESTINATION lib/cups/filter
    RENAME rastertokpsl-x32
    COMPONENT proprietary_runtime
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
)

# Install rastertokpsl-x64 binary (runtime component)
install(
    PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/rastertokpsl-x64"
    DESTINATION lib/cups/filter
    RENAME rastertokpsl-x64
    COMPONENT proprietary_runtime
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
)
