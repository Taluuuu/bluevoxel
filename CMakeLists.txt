cmake_minimum_required(VERSION 3.20)

project(vulkan-test)

set(GAME_NAME "BlueVoxel")

file(REMOVE_RECURSE ${CMAKE_CURRENT_BINARY_DIR}/Resources)
file(MAKE_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}/Resources)

add_subdirectory(engine)
add_subdirectory(bluevoxel_client)
add_subdirectory(bluevoxel_server)

foreach(RESOURCE_PATH ${RESOURCE_PATHS})
    # Find the name of the parent folder of each resource folder.
    # For engine/res, this would be 'engine'.
    cmake_path(GET RESOURCE_PATH PARENT_PATH RESOURCE_PARENT_PATH)
    cmake_path(GET RESOURCE_PARENT_PATH FILENAME RESOURCE_PARENT_NAME)    
    
    # Copy and rename the folder in the Resources directory
    file(COPY ${RESOURCE_PATH} DESTINATION "Resources")
    set(NEW_RESOURCE_PATH ${CMAKE_CURRENT_BINARY_DIR}/Resources/${RESOURCE_PARENT_NAME})
    file(RENAME ${CMAKE_CURRENT_BINARY_DIR}/Resources/res/ ${NEW_RESOURCE_PATH})

    # Compile vulkan shaders to SPIR-V
    if(H2O_USE_VULKAN)
        file(GLOB_RECURSE VULKAN_SHADER_SOURCES "${NEW_RESOURCE_PATH}/shaders/vulkan/**.vert" "${NEW_RESOURCE_PATH}/shaders/vulkan/**.frag")
        foreach(VULKAN_SHADER_SOURCE ${VULKAN_SHADER_SOURCE})
            execute_process(COMMAND glslangValidator -V ${VULKAN_SHADER_SOURCE} -o ${VULKAN_SHADER_SOURCE}.spv OUTPUT_QUIET)
            file(REMOVE ${VULKAN_SHADER_SOURCE})
        endforeach()
    else()
        # Delete vulkan shader files
        file(REMOVE_RECURSE "${NEW_RESOURCE_PATH}/shaders/vulkan/")
    endif()
endforeach()