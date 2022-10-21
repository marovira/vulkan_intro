#include "vk_initialisers.hpp"

namespace vk_initialisers
{
    vk::CommandPoolCreateInfo command_pool_create_info(std::uint32_t queue_family_index,
                                                       vk::CommandPoolCreateFlags flags)
    {
        return vk::CommandPoolCreateInfo{.flags            = flags,
                                         .queueFamilyIndex = queue_family_index};
    }

    vk::CommandBufferAllocateInfo
    command_buffer_allocate_info(vk::raii::CommandPool const& pool,
                                 std::uint32_t count,
                                 vk::CommandBufferLevel level)
    {
        return vk::CommandBufferAllocateInfo{.commandPool        = *pool,
                                             .level              = level,
                                             .commandBufferCount = count};
    }

    vk::PipelineShaderStageCreateInfo
    pipeline_shader_stage_create_info(vk::ShaderStageFlagBits stage,
                                      vk::raii::ShaderModule const& shader_module)
    {
        return vk::PipelineShaderStageCreateInfo{.stage  = stage,
                                                 .module = *shader_module,
                                                 .pName  = "main"};
    }

    vk::PipelineVertexInputStateCreateInfo vertex_input_state_create_info()
    {
        return vk::PipelineVertexInputStateCreateInfo{.vertexBindingDescriptionCount = 0,
                                                      .vertexAttributeDescriptionCount =
                                                          0};
    }

    vk::PipelineInputAssemblyStateCreateInfo
    input_assembly_create_info(vk::PrimitiveTopology topology)
    {
        return vk::PipelineInputAssemblyStateCreateInfo{.topology = topology,
                                                        .primitiveRestartEnable = false};
    }

    vk::PipelineRasterizationStateCreateInfo
    rasterization_create_info(vk::PolygonMode polygon_mode)
    {
        return vk::PipelineRasterizationStateCreateInfo{
            .depthClampEnable        = false,
            .rasterizerDiscardEnable = false,
            .polygonMode             = polygon_mode,
            .cullMode                = vk::CullModeFlagBits::eNone,
            .frontFace               = vk::FrontFace::eClockwise,
            .depthBiasEnable         = false,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp          = 0.0f,
            .depthBiasSlopeFactor    = 0.0f,
            .lineWidth               = 1.0f};
    }

    vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info()
    {
        return vk::PipelineMultisampleStateCreateInfo{.rasterizationSamples =
                                                          vk::SampleCountFlagBits::e1,
                                                      .sampleShadingEnable   = false,
                                                      .minSampleShading      = 1.0f,
                                                      .pSampleMask           = nullptr,
                                                      .alphaToCoverageEnable = false,
                                                      .alphaToOneEnable      = false};
    }

    vk::PipelineColorBlendAttachmentState colour_blend_attachment_state()
    {
        return vk::PipelineColorBlendAttachmentState{
            .blendEnable = false,
            .colorWriteMask =
                vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG
                | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};
    }

    vk::PipelineLayoutCreateInfo pipeline_layout_create_info()
    {
        return vk::PipelineLayoutCreateInfo{.flags                  = {},
                                            .setLayoutCount         = 0,
                                            .pSetLayouts            = nullptr,
                                            .pushConstantRangeCount = 0,
                                            .pPushConstantRanges    = nullptr};
    }

    vk::ImageCreateInfo image_create_info(vk::Format format,
                                          vk::ImageUsageFlags usage_flags,
                                          vk::Extent3D extent)
    {
        return vk::ImageCreateInfo{.imageType   = vk::ImageType::e2D,
                                   .format      = format,
                                   .extent      = extent,
                                   .mipLevels   = 1,
                                   .arrayLayers = 1,
                                   .samples     = vk::SampleCountFlagBits::e1,
                                   .tiling      = vk::ImageTiling::eOptimal,
                                   .usage       = usage_flags};
    }

    vk::ImageViewCreateInfo image_view_create_info(vk::Format format,
                                                   vk::Image image,
                                                   vk::ImageAspectFlags aspect_flags)
    {
        return vk::ImageViewCreateInfo{
            .image            = image,
            .viewType         = vk::ImageViewType::e2D,
            .format           = format,
            .subresourceRange = vk::ImageSubresourceRange{.aspectMask     = aspect_flags,
                                                          .baseMipLevel   = 0,
                                                          .levelCount     = 1,
                                                          .baseArrayLayer = 0,
                                                          .layerCount     = 1}
        };
    }

    vk::PipelineDepthStencilStateCreateInfo
    depth_stencil_create_info(bool depth_test, bool depth_write, vk::CompareOp compare_op)
    {
        return vk::PipelineDepthStencilStateCreateInfo{
            .depthTestEnable       = depth_test,
            .depthWriteEnable      = depth_write,
            .depthCompareOp        = depth_test ? compare_op : vk::CompareOp::eAlways,
            .depthBoundsTestEnable = false,
            .stencilTestEnable     = false,
            .minDepthBounds        = 0.0f,
            .maxDepthBounds        = 1.0f};
    }
} // namespace vk_initialisers
