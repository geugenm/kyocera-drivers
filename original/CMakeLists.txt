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

# Install wrapper.sh as rastertokpsl
install(
    PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/wrapper.sh"
    DESTINATION /usr/lib/cups/filter
    RENAME rastertokpsl
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
)

# Install rastertokpsl-x32 binary
install(
    PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/rastertokpsl-x32"
    DESTINATION /usr/lib/cups/filter
    RENAME rastertokpsl-x32
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
)

# Install rastertokpsl-x64 binary
install(
    PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/rastertokpsl-x64"
    DESTINATION /usr/lib/cups/filter
    RENAME rastertokpsl-x64
    PERMISSIONS
        OWNER_READ
        OWNER_WRITE
        OWNER_EXECUTE
        GROUP_READ
        GROUP_EXECUTE
        WORLD_READ
        WORLD_EXECUTE
)
