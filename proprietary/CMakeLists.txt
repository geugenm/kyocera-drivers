# The wrapper is portable (bash) and always installed as `rastertokpsl`.
# It resolves the real filter binary relative to its own location at
# runtime, so no prefix is baked in and `cmake --install --prefix ...`
# overrides work correctly. On architectures without a proprietary
# binary (ARM, …) it dispatches to the rastertokpsl-re filter instead —
# see the readme.
install(
    PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/wrapper.sh"
    DESTINATION "${KYOCERA_CUPS_FILTER_DIR}"
    COMPONENT proprietary_runtime
    RENAME rastertokpsl
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)

# Kyocera ships the proprietary filter binaries for x86 only. Install the
# matching one — plus the 32-bit binary on x86_64 hosts for multilib —
# and skip them entirely everywhere else; the wrapper then requires
# rastertokpsl-re (see the readme's ARM section).
if(KYOCERA_X86_64)
    install(
        PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/rastertokpsl_amd64"
        DESTINATION "${KYOCERA_CUPS_FILTER_DIR}"
        COMPONENT proprietary_runtime
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )

    install(
        PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/rastertokpsl_x86"
        DESTINATION "${KYOCERA_CUPS_FILTER_DIR}"
        COMPONENT proprietary_runtime
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )
elseif(KYOCERA_X86_32)
    install(
        PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/rastertokpsl_x86"
        DESTINATION "${KYOCERA_CUPS_FILTER_DIR}"
        COMPONENT proprietary_runtime
        PERMISSIONS
            OWNER_READ OWNER_WRITE OWNER_EXECUTE
            GROUP_READ GROUP_EXECUTE
            WORLD_READ WORLD_EXECUTE
    )
endif()
