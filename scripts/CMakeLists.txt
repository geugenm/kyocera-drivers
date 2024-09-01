add_custom_target(fetch_drivers_script
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/fetch_drivers.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Fetch .ppd files from kyocera official servers"
)