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

    vk::CommandPoolCreateInfo
    command_pool_create_info(std::uint32_t queue_family_index,
                             vk::CommandPoolCreateFlags flags = {});
    vk::CommandBufferAllocateInfo command_buffer_allocate_info(
        vk::raii::CommandPool const& pool,
        std::uint32_t count    = 1,
        vk::CommandBufferLevel = vk::CommandBufferLevel::ePrimary);

    vk::PipelineShaderStageCreateInfo
    pipeline_shader_stage_create_info(vk::ShaderStageFlagBits stage,
                                      vk::raii::ShaderModule const& shader_module);
    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info();

    vk::PipelineInputAssemblyStateCreateInfo
    input_assembly_create_info(vk::PrimitiveTopology topology);

    vk::PipelineRasterizationStateCreateInfo
    rasterization_create_info(vk::PolygonMode polygon_mode);

    vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info();

    vk::PipelineColorBlendAttachmentState colour_blend_attachment_state();

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info();

    vk::ImageCreateInfo image_create_info(vk::Format format,
                                          vk::ImageUsageFlags usage_flags,
                                          vk::Extent3D extent);
    vk::ImageViewCreateInfo image_view_create_info(vk::Format format,
                                                   vk::Image image,
                                                   vk::ImageAspectFlags aspect_flags);

    vk::PipelineDepthStencilStateCreateInfo
    depth_stencil_create_info(bool depth_test,
                              bool depth_write,
                              vk::CompareOp compare_op);

} // namespace vk_initialisers
