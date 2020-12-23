
# force (enable) static linking:
link_libraries(${Boost_LIBRARIES})

add_executable (test1 "test1.cpp" "test1.h" "common.h" "common.cpp")
add_test(test1 test1)

add_custom_command(
        TARGET test1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/zpool
                ${CMAKE_CURRENT_BINARY_DIR}/zpool)
add_custom_command(
        TARGET test1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy
                ${CMAKE_CURRENT_SOURCE_DIR}/zpool.cmd
                ${CMAKE_CURRENT_BINARY_DIR}/zpool.cmd)
add_custom_command(
        TARGET test1 POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
                ${CMAKE_CURRENT_SOURCE_DIR}/example_outputs
                ${CMAKE_CURRENT_BINARY_DIR}/)
