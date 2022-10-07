#include "vk_initialisers.hpp"

namespace vk_initialisers
{
    vk::UniqueInstance make_unique_instance(vk::Instance const& instance)
    {
        vk::ObjectDestroy<vk::NoParent, vk::DispatchLoaderStatic> deleter{nullptr};
        return vk::UniqueInstance{instance, deleter};
    }

    vk::UniqueDevice make_unique_device(vk::Device const& device)
    {
        vk::ObjectDestroy<vk::NoParent, vk::DispatchLoaderStatic> deleter{nullptr};
        return vk::UniqueDevice{device, deleter};
    }

    vk::CommandPoolCreateInfo command_pool_create_info(std::uint32_t queue_family_index,
                                                       vk::CommandPoolCreateFlags flags)
    {
        return vk::CommandPoolCreateInfo{.flags            = flags,
                                         .queueFamilyIndex = queue_family_index};
    }

    vk::CommandBufferAllocateInfo
    command_buffer_allocate_info(vk::CommandPool const& pool,
                                 std::uint32_t count,
                                 vk::CommandBufferLevel level)
    {
        return vk::CommandBufferAllocateInfo{.commandPool        = pool,
                                             .level              = level,
                                             .commandBufferCount = count};
    }
} // namespace vk_initialisers
