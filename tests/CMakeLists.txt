add_executable(test_rastertokpsl test_raster.cxx)

add_custom_target(
    test_rastertokpsl_assets
    ALL
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
        ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    BYPRODUCTS ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    COMMENT "symlink test.pdf into binary dir"
)

add_dependencies(test_rastertokpsl test_rastertokpsl_assets)

add_test(
    NAME rastertokpsl
    COMMAND test_rastertokpsl
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
