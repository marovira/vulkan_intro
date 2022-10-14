#include "vulkan_engine.hpp"
#include "vk_initialisers.hpp"

#include <zeus/assert.hpp>

// Convert from a pointer to a vk::raii object to the corresponding vk:: object.
template<typename T>
auto to_vk_type(T const& ptr)
{
    return *(*ptr);
}

VKAPI_ATTR VkBool32 VKAPI_CALL
debug_callback(VkDebugUtilsMessageSeverityFlagBitsEXT severity,
               VkDebugUtilsMessageTypeFlagsEXT type,
               VkDebugUtilsMessengerCallbackDataEXT const* callback_data,
               void*)
{
    auto ms = vkb::to_string_message_severity(severity);
    auto mt = vkb::to_string_message_type(type);
    fmt::print("[{} : {}]\n{}\n", ms, mt, callback_data->pMessage);

    if (severity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
    {
        ASSERT(0);
    }

    return 0;
}

vk::raii::Pipeline PipelineBuilder::build_pipeline(vk::raii::Device const& device,
                                                   vk::RenderPass pass)
{
    vk::PipelineViewportStateCreateInfo viewport_state{.viewportCount = 1,
                                                       .pViewports    = &viewport,
                                                       .scissorCount  = 1,
                                                       .pScissors     = &scissor};

    vk::PipelineColorBlendStateCreateInfo colour_blending{.logicOpEnable = VK_FALSE,
                                                          .logicOp = vk::LogicOp::eCopy,
                                                          .attachmentCount = 1,
                                                          .pAttachments =
                                                              &colour_blend_attachment};

    vk::GraphicsPipelineCreateInfo pipeline_info{
        .stageCount          = static_cast<std::uint32_t>(shader_stages.size()),
        .pStages             = shader_stages.data(),
        .pVertexInputState   = &vertex_input_info,
        .pInputAssemblyState = &input_assembly,
        .pViewportState      = &viewport_state,
        .pRasterizationState = &rasterizer,
        .pMultisampleState   = &multisampling,
        .pColorBlendState    = &colour_blending,
        .layout              = pipeline_layout,
        .renderPass          = pass,
        .subpass             = 0,
        .basePipelineHandle  = VK_NULL_HANDLE};

    vk::PipelineCacheCreateInfo cache_info = {};

    vk::raii::PipelineCache cache{device, cache_info};
    return vk::raii::Pipeline{device, cache, pipeline_info};
}

VulkanEngine::~VulkanEngine()
{
    [[maybe_unused]] auto val =
        m_device->waitForFences({to_vk_type(m_render_fence)}, true, 1000000000);
}

void VulkanEngine::set_surface_callback(SurfaceCallback&& callback)
{
    m_surface_callback = std::move(callback);
}

void VulkanEngine::set_window_extent(vk::Extent2D extent)
{
    m_window_extent = extent;
}

void VulkanEngine::init()
{
    init_vulkan();
    init_swapchain();
    init_commands();
    init_default_render_pass();
    init_framebuffers();
    init_sync_structures();
    init_pipelines();
}

void VulkanEngine::render()
{
    // Global variable to dump the results in. Note that since we have Vulkan configured
    // to throw exceptions, checking the result is not really necessary (we're guaranteed
    // that it will work).
    vk::Result result;

    result = m_device->waitForFences({to_vk_type(m_render_fence)}, true, 1000000000);
    m_device->resetFences({to_vk_type(m_render_fence)});

    std::uint32_t swapchain_image_idx;
    std::tie(result, swapchain_image_idx) =
        m_swapchain.handle->acquireNextImage(1000000000, to_vk_type(m_present_semaphore));

    // Grab the command buffer so we can use it directly.
    auto const& cmd = m_command_pool.command_buffers.front();

    cmd.reset();

    vk::CommandBufferBeginInfo cmd_begin_info{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    cmd.begin(cmd_begin_info);

    float flash = std::abs(std::sin(m_frame_number / 120.0f));
    vk::ClearValue colour_clear{.color = {std::array{0.0f, 0.0f, flash, 1.0f}}};

    vk::RenderPassBeginInfo rp_info{
        .renderPass  = to_vk_type(m_render_pass),
        .framebuffer = *m_framebuffers[swapchain_image_idx],
        .renderArea = vk::Rect2D{.offset = vk::Offset2D{0, 0}, .extent = m_window_extent},
        .clearValueCount = 1,
        .pClearValues    = &colour_clear
    };

    cmd.beginRenderPass(rp_info, vk::SubpassContents::eInline);

    cmd.bindPipeline(vk::PipelineBindPoint::eGraphics, to_vk_type(m_triangle_pipeline));
    cmd.draw(3, 1, 0, 0);

    cmd.endRenderPass();
    cmd.end();

    // We're going to need the address of several vk:: objects, so grab them here.
    auto present_semaphore = to_vk_type(m_present_semaphore);
    auto render_semaphore  = to_vk_type(m_render_semaphore);
    auto swapchain         = to_vk_type(m_swapchain.handle);

    vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submit{.waitSemaphoreCount   = 1,
                          .pWaitSemaphores      = &present_semaphore,
                          .pWaitDstStageMask    = &wait_stage,
                          .commandBufferCount   = 1,
                          .pCommandBuffers      = &(*cmd),
                          .signalSemaphoreCount = 1,
                          .pSignalSemaphores    = &render_semaphore};

    m_graphics_queue.queue.submit({submit}, to_vk_type(m_render_fence));

    vk::PresentInfoKHR present_info{.waitSemaphoreCount = 1,
                                    .pWaitSemaphores    = &render_semaphore,
                                    .swapchainCount     = 1,
                                    .pSwapchains        = &swapchain,
                                    .pImageIndices      = &swapchain_image_idx};

    result = m_graphics_queue.queue.presentKHR(present_info);
    ++m_frame_number;
}

void VulkanEngine::init_vulkan()
{
    m_context = std::make_unique<vk::raii::Context>();

    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("Vulkan App")
                        .request_validation_layers(true)
                        .require_api_version(1, 3, 0)
                        .set_debug_callback(debug_callback)
                        .build();

    vkb::Instance vkb_inst = inst_ret.value();

    m_instance = std::make_unique<vk::raii::Instance>(*m_context, vkb_inst.instance);
    m_debug_messenger =
        std::make_unique<vk::raii::DebugUtilsMessengerEXT>(*m_instance,
                                                           vkb_inst.debug_messenger);

    // Grab the instance itself from the wrapper so we can create the surface.
    auto instance = to_vk_type(m_instance);

    m_surface =
        std::make_unique<vk::raii::SurfaceKHR>(*m_instance, m_surface_callback(instance));

    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::PhysicalDevice physical_device = selector.set_minimum_version(1, 3)
                                              .set_surface(to_vk_type(m_surface))
                                              .select()
                                              .value();

    vkb::DeviceBuilder device_builder{physical_device};
    vkb::Device vkb_device = device_builder.build().value();

    // This is a bit... awkward, but here goes: in order to create a device, we first need
    // the physical device that it will reference (devices are after all logical
    // references to physical devices). So we need to make a RAII physical device in order
    // to create the actual device... BUT! note that this will get destroyed as soon as we
    // exit the function, which is fine because physical devices are not real resources,
    // so this temporary is irrelevant.
    vk::raii::PhysicalDevice phyisical_device{*m_instance, vkb_device.physical_device};

    // Now create the actual device.
    m_device = std::make_unique<vk::raii::Device>(phyisical_device, vkb_device.device);

    // Keep a copy of the physical device in the class itself.
    m_chosen_gpu = vkb_device.physical_device;

    m_graphics_queue.queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    m_graphics_queue.family_index =
        vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::init_swapchain()
{
    vkb::SwapchainBuilder swapchain_builder{m_chosen_gpu,
                                            to_vk_type(m_device),
                                            to_vk_type(m_surface)};

    vkb::Swapchain vkb_swapchain =
        swapchain_builder.use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(m_window_extent.width, m_window_extent.height)
            .build()
            .value();

    // Note that the swapchain is owned by the device, so by creating it like this we
    // guarantee that it will be destroyed before the device.
    m_swapchain.handle =
        std::make_unique<vk::raii::SwapchainKHR>(*m_device, vkb_swapchain.swapchain);

    // Images are owned by the swapchain, so these get destroyed upon its destruction.
    vk_initialisers::to_vk_vector(vkb_swapchain.get_images().value(),
                                  m_swapchain.images,
                                  [](VkImage img) {
                                      return vk::Image{img};
                                  });

    // Views on the other hand are owned by the device, so make sure they're tied to it.
    vk_initialisers::to_vk_vector(
        vkb_swapchain.get_image_views().value(),
        m_swapchain.image_views,
        [this](VkImageView view) {
            return std::make_unique<vk::raii::ImageView>(*m_device, view);
        });

    m_swapchain.format = vk::Format{vkb_swapchain.image_format};
}

void VulkanEngine::init_commands()
{
    using namespace vk_initialisers;

    {
        auto info =
            command_pool_create_info(m_graphics_queue.family_index,
                                     vk::CommandPoolCreateFlagBits::eResetCommandBuffer);
        m_command_pool.pool = std::make_unique<vk::raii::CommandPool>(*m_device, info);
    }

    {
        auto info = command_buffer_allocate_info(*m_command_pool.pool,
                                                 1,
                                                 vk::CommandBufferLevel::ePrimary);
        m_command_pool.command_buffers = vk::raii::CommandBuffers{*m_device, info};
    }
}

void VulkanEngine::init_default_render_pass()
{
    vk::AttachmentDescription colour_attachment{
        .format         = m_swapchain.format,
        .samples        = vk::SampleCountFlagBits::e1,
        .loadOp         = vk::AttachmentLoadOp::eClear,
        .storeOp        = vk::AttachmentStoreOp::eStore,
        .stencilLoadOp  = vk::AttachmentLoadOp::eDontCare,
        .stencilStoreOp = vk::AttachmentStoreOp::eDontCare,
        .initialLayout  = vk::ImageLayout::eUndefined,
        .finalLayout    = vk::ImageLayout::ePresentSrcKHR};

    vk::AttachmentReference colour_attachment_ref{
        .attachment = 0,
        .layout     = vk::ImageLayout::eColorAttachmentOptimal};

    vk::SubpassDescription subpass{.pipelineBindPoint = vk::PipelineBindPoint::eGraphics,
                                   .colorAttachmentCount = 1,
                                   .pColorAttachments    = &colour_attachment_ref};

    vk::RenderPassCreateInfo render_pass_info{.attachmentCount = 1,
                                              .pAttachments    = &colour_attachment,
                                              .subpassCount    = 1,
                                              .pSubpasses      = &subpass};

    m_render_pass = std::make_unique<vk::raii::RenderPass>(*m_device, render_pass_info);
}

void VulkanEngine::init_framebuffers()
{
    auto render_pass     = to_vk_type(m_render_pass);
    auto [width, height] = m_window_extent;
    std::array<vk::ImageView, 1> attachments;
    for (auto const& img_view : m_swapchain.image_views)
    {
        attachments[0] = to_vk_type(img_view);
        vk::FramebufferCreateInfo fb_info{.renderPass      = render_pass,
                                          .attachmentCount = 1,
                                          .pAttachments    = attachments.data(),
                                          .width           = width,
                                          .height          = height,
                                          .layers          = 1};

        m_framebuffers.emplace_back(*m_device, fb_info);
    }
}

void VulkanEngine::init_sync_structures()
{
    vk::FenceCreateInfo fence_create_info{.flags = vk::FenceCreateFlagBits::eSignaled};
    m_render_fence = std::make_unique<vk::raii::Fence>(*m_device, fence_create_info);

    vk::SemaphoreCreateInfo semaphore_info;
    m_present_semaphore =
        std::make_unique<vk::raii::Semaphore>(*m_device, semaphore_info);
    m_render_semaphore = std::make_unique<vk::raii::Semaphore>(*m_device, semaphore_info);
}

void VulkanEngine::init_pipelines()
{
    namespace fs = std::filesystem;
    using namespace vk_initialisers;

    auto shader_root = fs::current_path() / "spv";
    auto vert_module = load_shader_module(shader_root / "triangle.vert.spv");
    auto frag_module = load_shader_module(shader_root / "triangle.frag.spv");

    auto pipeline_layout_info = pipeline_layout_create_info();
    m_triangle_pipeline_layout =
        std::make_unique<vk::raii::PipelineLayout>(*m_device, pipeline_layout_info);

    PipelineBuilder pipeline_builder;

    pipeline_builder.shader_stages.push_back(
        pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eVertex, vert_module));

    pipeline_builder.shader_stages.push_back(
        pipeline_shader_stage_create_info(vk::ShaderStageFlagBits::eFragment,
                                          frag_module));

    pipeline_builder.vertex_input_info = vertex_input_state_create_info();
    pipeline_builder.input_assembly =
        input_assembly_create_info(vk::PrimitiveTopology::eTriangleList);

    pipeline_builder.viewport.x        = 0.0f;
    pipeline_builder.viewport.y        = 0.0f;
    pipeline_builder.viewport.width    = static_cast<float>(m_window_extent.width);
    pipeline_builder.viewport.height   = static_cast<float>(m_window_extent.height);
    pipeline_builder.viewport.minDepth = 0.0f;
    pipeline_builder.viewport.maxDepth = 1.0f;

    pipeline_builder.scissor.offset = vk::Offset2D{0, 0};
    pipeline_builder.scissor.extent = m_window_extent;

    pipeline_builder.rasterizer = rasterization_create_info(vk::PolygonMode::eFill);

    pipeline_builder.multisampling           = multisampling_state_create_info();
    pipeline_builder.colour_blend_attachment = colour_blend_attachment_state();
    pipeline_builder.pipeline_layout         = to_vk_type(m_triangle_pipeline_layout);

    m_triangle_pipeline = std::make_unique<vk::raii::Pipeline>(
        pipeline_builder.build_pipeline(*m_device, to_vk_type(m_render_pass)));
}

vk::raii::ShaderModule VulkanEngine::load_shader_module(std::filesystem::path const& path)
{
    std::ifstream stream{path, std::ios::ate | std::ios::binary};
    if (!stream.is_open())
    {
        auto msg = fmt::format("error: unable to open file {}", path.string());
        throw std::runtime_error{msg.c_str()};
    }

    std::size_t file_size = stream.tellg();
    std::vector<std::uint32_t> buffer(file_size / sizeof(std::uint32_t));
    stream.seekg(0);

    stream.read((char*)buffer.data(), file_size);
    stream.close();

    vk::ShaderModuleCreateInfo create_info{.codeSize =
                                               buffer.size() * sizeof(std::uint32_t),
                                           .pCode = buffer.data()};

    return vk::raii::ShaderModule{*m_device, create_info};
}
