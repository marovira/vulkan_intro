add_subdirectory(${VULKAN_INTRO_SOURCE_ROOT}/shaders)

set(PCH ${VULKAN_INTRO_SOURCE_ROOT}/pch.hpp)

set(SOURCE_LIST
    ${VULKAN_INTRO_SOURCE_ROOT}/main.cpp
    ${VULKAN_INTRO_SOURCE_ROOT}/assert.cpp
    )

set(INCLUDE_LIST
    ${PCH}
    )

source_group("source" FILES ${SOURCE_LIST})
source_group("include" FILES ${INCLUDE_LIST})
source_group("shaders" FILES ${SHADER_LIST})

add_executable(vulkan_intro ${SOURCE_LIST} ${INCLUDE_LIST} ${SHADER_LIST})
target_precompile_headers(vulkan_intro PRIVATE ${PCH})
target_link_libraries(vulkan_intro PRIVATE
    Vulkan::Vulkan
    zeus::zeus
    glm::glm
    glfw
    ${TINYOBJLOADER_LIB}
    vk-bootstrap::vk-bootstrap
    VulkanMemoryAllocator
    )
target_compile_features(vulkan_intro PRIVATE cxx_std_20)
target_compile_definitions(vulkan_intro PRIVATE -DNOMINMAX)

# Set the PCH stuff under a custom filter.
file (GLOB_RECURSE PRECOMPILED_HEADER_FILES
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/cmake_pch.*)
source_group("include\\pch" FILES ${PRECOMPILED_HEADER_FILES})
