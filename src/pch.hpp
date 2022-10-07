#pragma once

#include <cmath>
#include <deque>
#include <fstream>
#include <functional>
#include <memory>
#include <vector>

// Global options for Vulkan (could be moved into the preset file if need be).
// 1. Disable user-defined constructors from structs so we can use aggregate
// initialisation.
// 2. Disable setters so only direct assignment is available.
#define VULKAN_HPP_NO_CONSTRUCTORS
#define VULKAN_HPP_NO_SETTERS
#include <vulkan/vulkan.hpp>

#include <fmt/printf.h>
#include <magic_enum.hpp>
#include <vk_mem_alloc.h>
#include <VkBootstrap.h>
#include <zeus/assert.hpp>
#include <zeus/enum_bitfield.hpp>

#include <GLFW/glfw3.h>

#pragma warning(push)
#pragma warning(disable : 4201)
#include <glm/ext.hpp>
#include <glm/glm.hpp>
#pragma warning(pop)

#include <tiny_obj_loader.h>
