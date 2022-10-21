add_subdirectory(${VULKAN_INTRO_SOURCE_ROOT}/shaders)

set(PCH ${VULKAN_INTRO_SOURCE_ROOT}/pch.hpp)

set(SOURCE_LIST
    ${VULKAN_INTRO_SOURCE_ROOT}/main.cpp
    ${VULKAN_INTRO_SOURCE_ROOT}/assert.cpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vulkan_app.cpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vulkan_engine.cpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vk_initialisers.cpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vma.cpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vk_mesh.cpp
    )

set(INCLUDE_LIST
    ${PCH}
    ${VULKAN_INTRO_SOURCE_ROOT}/vulkan_app.hpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vulkan_engine.hpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vk_initialisers.hpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vk_types.hpp
    ${VULKAN_INTRO_SOURCE_ROOT}/vk_mesh.hpp
    )

source_group("source" FILES ${SOURCE_LIST})
source_group("include" FILES ${INCLUDE_LIST})
source_group("shaders" FILES ${SHADER_LIST} ${SHADER_INCLUDE})

add_executable(vulkan_intro ${SOURCE_LIST} ${INCLUDE_LIST} ${SHADER_LIST})
target_precompile_headers(vulkan_intro PRIVATE ${PCH})
target_link_libraries(vulkan_intro PRIVATE
    Vulkan::Vulkan
    zeus::zeus
    glm::glm
    glfw
    assimp::assimp
    vk-bootstrap::vk-bootstrap
    VulkanMemoryAllocator
    )
target_compile_features(vulkan_intro PRIVATE cxx_std_20)
target_compile_definitions(vulkan_intro PRIVATE -DNOMINMAX)

# Set the PCH stuff under a custom filter.
file (GLOB_RECURSE PRECOMPILED_HEADER_FILES
    ${CMAKE_CURRENT_BINARY_DIR}${CMAKE_FILES_DIRECTORY}/cmake_pch.*)
source_group("include\\pch" FILES ${PRECOMPILED_HEADER_FILES})

# Tell VS to set the working directory to the output of the exe itself.
if (MSVC)
    set_target_properties(vulkan_intro PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY
        $<TARGET_FILE_DIR:vulkan_intro>)
endif()

#================================
# SPIR-V Generation
#================================
set(SPV_BUILD_ROOT "${CMAKE_CURRENT_BINARY_DIR}/spv")
file(MAKE_DIRECTORY ${SPV_BUILD_ROOT})

foreach(GLSL ${SHADER_LIST})
    get_filename_component(FILE_NAME ${GLSL} NAME)
    set(SPIRV "${SPV_BUILD_ROOT}/${FILE_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND Vulkan::glslangValidator -V ${GLSL} -o ${SPIRV}
        DEPENDS ${GLSL}
        )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()
add_custom_target(compile_shaders DEPENDS ${SPIRV_BINARY_FILES})
add_custom_command(TARGET compile_shaders POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${SPV_BUILD_ROOT}
    $<TARGET_FILE_DIR:vulkan_intro>/spv
    )
add_custom_command(TARGET vulkan_intro POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${VULKAN_INTRO_MODELS_ROOT}
    $<TARGET_FILE_DIR:vulkan_intro>/models
    )
add_dependencies(vulkan_intro compile_shaders)
