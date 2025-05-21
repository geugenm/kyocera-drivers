add_executable(test_rastertokpsl test_raster.cxx)

add_dependencies(test_rastertokpsl rastertokpsl)

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

add_executable(test_filter test_filter.cxx)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/Kyocera_FS-1020MFPGDI.ppd
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/../ppd/English/Kyocera_FS-1020MFPGDI.ppd
        ${CMAKE_CURRENT_BINARY_DIR}/Kyocera_FS-1020MFPGDI.ppd
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/../ppd/English/Kyocera_FS-1020MFPGDI.ppd
    COMMENT "symlink PPD into binary dir"
)

add_custom_target(
    test_filter_assets
    ALL
    DEPENDS
        ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
        ${CMAKE_CURRENT_BINARY_DIR}/Kyocera_FS-1020MFPGDI.ppd
)

add_dependencies(test_filter test_filter_assets)

add_test(
    NAME filter_chain
    COMMAND test_filter
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)
