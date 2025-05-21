add_executable(test_rastertokpsl test_raster.cxx)

# Ensure rastertokps is built before the test
add_dependencies(test_rastertokpsl rastertokpsl)

# Symlink rastertokps into the test binary dir
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/rastertokpsl
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:rastertokpsl>
        ${CMAKE_CURRENT_BINARY_DIR}/rastertokpsl
    DEPENDS rastertokpsl
    COMMENT "symlink rastertokps into binary dir"
)

add_custom_target(
    test_rastertokpsl_symlink
    ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/rastertokpsl
)

add_dependencies(test_rastertokpsl test_rastertokpsl_symlink)

# Symlink test.pdf as before
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
        ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
    COMMENT "symlink test.pdf into binary dir"
)

add_custom_target(
    test_rastertokpsl_assets
    ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
)

add_dependencies(test_rastertokpsl test_rastertokpsl_assets)

add_test(
    NAME rastertokpsl
    COMMAND test_rastertokpsl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
