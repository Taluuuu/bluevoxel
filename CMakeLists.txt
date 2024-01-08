cmake_minimum_required(VERSION 3.20)

project(bluevoxel)

add_subdirectory(engine)
add_subdirectory(bluevoxel_client)
add_subdirectory(bluevoxel_server)
add_subdirectory(bluevoxel_editor)

# Copy res folder to output directory and rename to "Resources"
file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/Resources)
file(COPY res DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/res ${CMAKE_CURRENT_BINARY_DIR}/Resources)