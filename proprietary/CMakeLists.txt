# The wrapper is portable (bash) and always installed as `rastertokpsl`.
# It resolves the real filter binary relative to its own location at
# runtime, so no prefix is baked in and `cmake --install --prefix ...`
# overrides work correctly.
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

# Kyocera ships the proprietary filter binaries for x86 only:
#  - rastertokpsl_amd64 is installed on x86_64 hosts and runs natively;
#  - rastertokpsl_x86 is installed everywhere — natively on 32-bit x86,
#    and on ARM the wrapper executes it through qemu-i386 (see the
#    readme's ARM section).
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
endif()

install(
    PROGRAMS "${CMAKE_CURRENT_LIST_DIR}/rastertokpsl_x86"
    DESTINATION "${KYOCERA_CUPS_FILTER_DIR}"
    COMPONENT proprietary_runtime
    PERMISSIONS
        OWNER_READ OWNER_WRITE OWNER_EXECUTE
        GROUP_READ GROUP_EXECUTE
        WORLD_READ WORLD_EXECUTE
)
