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
                                                        .primitiveRestartEnable =
                                                            VK_FALSE};
    }

    vk::PipelineRasterizationStateCreateInfo
    rasterization_create_info(vk::PolygonMode polygon_mode)
    {
        return vk::PipelineRasterizationStateCreateInfo{
            .depthClampEnable        = VK_FALSE,
            .rasterizerDiscardEnable = VK_FALSE,
            .polygonMode             = polygon_mode,
            .cullMode                = vk::CullModeFlagBits::eNone,
            .frontFace               = vk::FrontFace::eClockwise,
            .depthBiasEnable         = VK_FALSE,
            .depthBiasConstantFactor = 0.0f,
            .depthBiasClamp          = 0.0f,
            .depthBiasSlopeFactor    = 0.0f,
            .lineWidth               = 1.0f};
    }

    vk::PipelineMultisampleStateCreateInfo multisampling_state_create_info()
    {
        return vk::PipelineMultisampleStateCreateInfo{.rasterizationSamples =
                                                          vk::SampleCountFlagBits::e1,
                                                      .sampleShadingEnable   = VK_FALSE,
                                                      .minSampleShading      = 1.0f,
                                                      .pSampleMask           = nullptr,
                                                      .alphaToCoverageEnable = VK_FALSE,
                                                      .alphaToOneEnable      = VK_FALSE};
    }

    vk::PipelineColorBlendAttachmentState colour_blend_attachment_state()
    {
        return vk::PipelineColorBlendAttachmentState{
            .blendEnable = VK_FALSE,
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
} // namespace vk_initialisers
