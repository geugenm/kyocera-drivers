cmake_minimum_required(VERSION 3.25)

add_executable(test_unicode
               test_unicode.cxx
)

target_link_libraries(test_unicode PRIVATE unicode)
add_test(NAME test_unicode COMMAND $<TARGET_FILE:test_unicode>)


add_executable(test_halfton
               test_halfton.cxx
)

target_link_libraries(test_halfton PRIVATE rastertokpsl_core)
add_test(NAME test_halfton COMMAND $<TARGET_FILE:test_halfton>)