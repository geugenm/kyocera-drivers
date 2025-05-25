add_executable(test_raster test_raster.cxx)
set_target_properties(
    test_raster
    PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)

add_dependencies(test_raster rastertokpsl)

add_custom_command(
    TARGET test_raster
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink $<TARGET_FILE:rastertokpsl>
        $<TARGET_FILE_DIR:test_raster>/rastertokpsl
    COMMENT
        "symlink $<TARGET_FILE:rastertokpsl> -> $<TARGET_FILE_DIR:test_raster>/rastertokpsl"
)

add_custom_command(
    TARGET test_raster
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
        $<TARGET_FILE_DIR:test_raster>/test.pdf
    COMMENT
        "symlink ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf -> $<TARGET_FILE_DIR:test_raster>/test.pdf"
)

add_test(
    NAME raster_chain
    COMMAND $<TARGET_FILE:test_raster>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:test_raster>
)

add_executable(test_filter test_filter.cxx)
set_target_properties(
    test_filter
    PROPERTIES CXX_STANDARD 23 CXX_STANDARD_REQUIRED YES CXX_EXTENSIONS NO
)

add_dependencies(test_filter rastertokpsl)

add_custom_command(
    TARGET test_filter
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink
        ${CMAKE_CURRENT_SOURCE_DIR}/../ppd/English/Kyocera_FS-1020MFPGDI.ppd
        $<TARGET_FILE_DIR:test_filter>/Kyocera_FS-1020MFPGDI.ppd
    COMMENT
        "symlink ${CMAKE_CURRENT_SOURCE_DIR}/../ppd/English/Kyocera_FS-1020MFPGDI.ppd -> $<TARGET_FILE_DIR:test_filter>/Kyocera_FS-1020MFPGDI.ppd"
)

add_custom_command(
    TARGET test_filter
    POST_BUILD
    COMMAND
        ${CMAKE_COMMAND} -E create_symlink ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf
        $<TARGET_FILE_DIR:test_filter>/test.pdf
    COMMENT
        "symlink ${CMAKE_CURRENT_SOURCE_DIR}/test.pdf -> $<TARGET_FILE_DIR:test_filter>/test.pdf"
)

add_test(
    NAME filter_chain
    COMMAND $<TARGET_FILE:test_filter>
    WORKING_DIRECTORY $<TARGET_FILE_DIR:test_filter>
)
