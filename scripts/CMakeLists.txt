add_custom_target(build_docs_script
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/build_docs.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running ${CMAKE_CURRENT_SOURCE_DIR}/build_docs.sh"
)

add_custom_target(build_script
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/build.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running ${CMAKE_CURRENT_SOURCE_DIR}/build.sh"
)

add_custom_target(fetch_drivers_script
        COMMAND bash ${CMAKE_CURRENT_SOURCE_DIR}/fetch_drivers.sh
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Running ${CMAKE_CURRENT_SOURCE_DIR}/fetch_drivers.sh"
)

add_custom_target(extract_ppd_and_ask_to_install
        COMMAND echo "Run sudo sh scripts/generate_and_install.sh"
        WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
        COMMENT "Will install preferred lang kyocera drivers generated by fetch_drivers_script"
)

add_dependencies(extract_ppd_and_ask_to_install fetch_drivers_script)