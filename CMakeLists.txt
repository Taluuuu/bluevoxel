cmake_minimum_required(VERSION 3.20)

project(vulkan-test)

set(GAME_NAME "Shhhh")

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/Resources)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)

add_subdirectory(engine)
add_subdirectory(game)

foreach(RESOURCE_PATH ${RESOURCE_PATHS})
    # Find the name of the parent folder of each resource folder.
    # For engine/res, this would be 'engine'.
    cmake_path(GET RESOURCE_PATH PARENT_PATH RESOURCE_PARENT_PATH)
    cmake_path(GET RESOURCE_PARENT_PATH FILENAME RESOURCE_PARENT_NAME)    
    
    # Copy and rename the folder in the Resources directory
    file(COPY ${RESOURCE_PATH} DESTINATION "Resources")
    file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/Resources/res/ ${CMAKE_CURRENT_BINARY_DIR}/Resources/${RESOURCE_PARENT_NAME})
endforeach(RESOURCE_PATH ${RESOURCE_PATHS})

# Compile shaders to SPIR-V.
# TODO: Make sure we want to use Vulkan before doing this
file(GLOB_RECURSE SHADER_SOURCES "${CMAKE_CURRENT_BINARY_DIR}/Resources/**/*.vert" "${CMAKE_CURRENT_BINARY_DIR}/Resources/**/*.frag")
foreach(SHADER_SOURCE ${SHADER_SOURCES})
    execute_process(COMMAND glslangValidator -V ${SHADER_SOURCE} -o ${SHADER_SOURCE}.spv OUTPUT_QUIET)
    file(REMOVE ${SHADER_SOURCE})
endforeach(SHADER_SOURCE ${SHADER_SOURCES})