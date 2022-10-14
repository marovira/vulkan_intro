#================================
# Project setup.
#================================
cmake_minimum_required(VERSION 3.24)
project(vulkan_intro LANGUAGES CXX)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

if (NOT DEFINED VULKAN_INTRO_PRESET)
    message(ERROR "Builds without a preset are not supported")
endif()
    

#================================
# Directory variables.
#================================
set(VULKAN_INTRO_SOURCE_DIR ${PROJECT_SOURCE_DIR})
set(VULKAN_INTRO_SOURCE_ROOT ${VULKAN_INTRO_SOURCE_DIR}/src)
set(VULKAN_INTRO_EXTERNAL_ROOT ${VULKAN_INTRO_SOURCE_DIR}/external)

#================================
# Find Packages.
#================================
find_package(Vulkan REQUIRED)
find_package(zeus REQUIRED)
find_package(glm REQUIRED)
find_package(glfw3 REQUIRED)
find_package(assimp REQUIRED)

# Vulkna-bootstrap and VMA don't have install logic, so we're going to pull them through
# fetch content.
include(FetchContent)

# Vk-bootstrap v0.6
FetchContent_Declare(
    vk_bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap
    GIT_TAG aeed599098060a1ecf6a5e6266748681f7ccc358
    )

# VMA v3.0.1
FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG a6bfc237255a6bac1513f7c1ebde6d8aed6b5191
    )


FetchContent_MakeAvailable(vk_bootstrap)
FetchContent_MakeAvailable(vma)

set_target_properties(vk-bootstrap PROPERTIES FOLDER "external")
set_target_properties(VulkanMemoryAllocator PROPERTIES FOLDER "external")

#================================
# Add subdirectories.
#================================
add_subdirectory(${VULKAN_INTRO_SOURCE_ROOT})
