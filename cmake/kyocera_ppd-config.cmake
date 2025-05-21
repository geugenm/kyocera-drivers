cmake_minimum_required(VERSION 3.28)
project(kyocera_driver_downloader LANGUAGES NONE)

include(FetchContent)

set(driver_url
    "https://www.kyoceradocumentsolutions.eu/content/download-center/eu/drivers/all/LinuxDrv_1_1203_FS_1x2xMFP_zip.download.zip"
)
set(driver_archive "${CMAKE_CURRENT_BINARY_DIR}/driver.zip")
set(extracted_dir "${CMAKE_CURRENT_BINARY_DIR}/fetched")

file(
    DOWNLOAD "${driver_url}" "${driver_archive}"
    SHOW_PROGRESS
    STATUS download_status
    LOG download_log
    TLS_VERIFY ON
)

list(GET download_status 0 download_code)
if(NOT download_code EQUAL 0)
    message(FATAL_ERROR "download failed: ${download_log}")
endif()

file(MAKE_DIRECTORY "${extracted_dir}")
file(ARCHIVE_EXTRACT INPUT "${driver_archive}" DESTINATION "${extracted_dir}")

file(GLOB lang_zips "${extracted_dir}/Linux/*/Global/*.tar.gz")

foreach(lang_zip IN LISTS lang_zips)
    get_filename_component(lang_dir "${lang_zip}" NAME_WE)
    set(lang_extract_dir "${extracted_dir}/Global/${lang_dir}")
    file(MAKE_DIRECTORY "${lang_extract_dir}")
    file(ARCHIVE_EXTRACT INPUT "${lang_zip}" DESTINATION "${lang_extract_dir}")
endforeach()

# Install PPDs to the canonical CUPS model directory
install(
    DIRECTORY "${extracted_dir}/Global/"
    DESTINATION "share/cups/model/Kyocera"
    FILES_MATCHING
    PATTERN "*.ppd"
)

add_custom_target(
    fetch_kyocera_driver
    ALL
    DEPENDS "${extracted_dir}"
    COMMENT "downloaded and extracted kyocera drivers to ${extracted_dir}"
)
