add_custom_target(
    test_rastertokpsl
    COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/test.sh
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    COMMENT "Fetch .ppd files from kyocera official servers"
)