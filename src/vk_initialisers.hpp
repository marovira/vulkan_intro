#pragma once

namespace vk_initialisers
{
    // Extension to convert a vector of Vk types to vk:: types.
    template<typename T, typename U, typename Fn>
    void to_vk_vector(std::vector<U>&& source, std::vector<T>& target, Fn&& fun)
    {
        target.resize(source.size());
        std::transform(std::make_move_iterator(source.begin()),
                       std::make_move_iterator(source.end()),
                       target.begin(),
                       [&fun](auto elem) {
                           return fun(elem);
                       });
    }

    template<typename T, typename U>
    T* to_vk_ptr(U* type)
    {
        return reinterpret_cast<T*>(type);
    }

    vk::UniqueInstance make_unique_instance(vk::Instance const& instance);
    vk::UniqueDevice make_unique_device(vk::Device const& device);

    vk::CommandPoolCreateInfo
    command_pool_create_info(std::uint32_t queue_family_index,
                             vk::CommandPoolCreateFlags flags = {});
    vk::CommandBufferAllocateInfo command_buffer_allocate_info(
        vk::CommandPool const& pool,
        std::uint32_t count    = 1,
        vk::CommandBufferLevel = vk::CommandBufferLevel::ePrimary);
} // namespace vk_initialisers
