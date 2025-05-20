add_executable(test_rastertokpsl test_raster.cxx)

add_test(
    NAME rastertokpsl
    COMMAND test_rastertokpsl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

# Ensure test.pdf is available in the binary dir
add_custom_command(
    TARGET test_rastertokpsl
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
        ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    COMMENT "Symlink test.pdf into binary directory"
)