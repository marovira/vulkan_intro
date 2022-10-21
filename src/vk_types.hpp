#pragma once

namespace vk_types
{
    struct AllocatedBuffer
    {
        vk::Buffer buffer;
        VmaAllocation allocation;
    };
} // namespace vk_types
