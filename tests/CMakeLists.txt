add_executable(test_raster test_raster.cxx)
set_target_properties(
    test_raster
    PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)

add_dependencies(test_raster rastertokpsl)

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

add_dependencies(test_raster test_rastertokpsl_symlink)

add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
        ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
    DEPENDS ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
    COMMENT "symlink test.pdf into binary dir"
)

add_custom_target(
    test_raster_assets
    ALL
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/test.pdf
)

add_dependencies(test_raster test_raster_assets)

add_test(
    NAME raster_chain
    COMMAND test_raster
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_executable(test_filter test_filter.cxx)
set_target_properties(
    test_filter
    PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)

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
