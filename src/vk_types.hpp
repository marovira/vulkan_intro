#pragma once

namespace vk_types
{
    struct AllocatedBuffer
    {
        vk::Buffer buffer;
        VmaAllocation allocation;
    };

    struct AllocatedImage
    {
        vk::Image image;
        VmaAllocation allocation;
    };
} // namespace vk_types
