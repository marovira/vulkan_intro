set(SHADER_ROOT ${VULKAN_INTRO_SOURCE_ROOT}/shaders)

set(SHADER_LIST
    ${SHADER_ROOT}/triangle.vert
    ${SHADER_ROOT}/triangle.frag
    PARENT_SCOPE)

set(SHADER_INCLUDE
    ${SHADER_ROOT}/bindings.h
    PARENT_SCOPE)
