#include "vulkan_engine.hpp"
#include "vk_initialisers.hpp"

#include <zeus/assert.hpp>

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

VulkanEngine::~VulkanEngine()
{
    [[maybe_unused]] auto val =
        m_device->waitForFences({*m_render_fence}, true, 1000000000);

    // Everything is handled through the pointer handles, so the only thing we need to
    // delete manually is the debug messenger.
    vkb::destroy_debug_utils_messenger(*m_instance, m_debug_messenger);
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
}

void VulkanEngine::render()
{
    auto val = m_device->waitForFences({*m_render_fence}, true, 1000000000);
    m_device->resetFences({*m_render_fence});

    auto swapchain_image_idx = m_device
                                   ->acquireNextImageKHR(*m_swapchain.handle,
                                                         1000000000,
                                                         *m_present_semaphore,
                                                         nullptr)
                                   .value;

    m_command_pool.command_buffer.reset();

    auto& cmd = m_command_pool.command_buffer;

    vk::CommandBufferBeginInfo cmd_begin_info{
        .flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
    cmd.begin(cmd_begin_info);

    float flash = std::abs(std::sin(m_frame_number / 120.0f));
    vk::ClearValue colour_clear{.color = {std::array{0.0f, 0.0f, flash, 1.0f}}};

    vk::RenderPassBeginInfo rp_info{
        .renderPass  = *m_render_pass,
        .framebuffer = *m_framebuffers[swapchain_image_idx],
        .renderArea = vk::Rect2D{.offset = vk::Offset2D{0, 0}, .extent = m_window_extent},
        .clearValueCount = 1,
        .pClearValues    = &colour_clear
    };

    cmd.beginRenderPass(rp_info, vk::SubpassContents::eInline);

    cmd.endRenderPass();
    cmd.end();

    vk::PipelineStageFlags wait_stage = vk::PipelineStageFlagBits::eColorAttachmentOutput;
    vk::SubmitInfo submit{.waitSemaphoreCount   = 1,
                          .pWaitSemaphores      = &(*m_present_semaphore),
                          .pWaitDstStageMask    = &wait_stage,
                          .commandBufferCount   = 1,
                          .pCommandBuffers      = &cmd,
                          .signalSemaphoreCount = 1,
                          .pSignalSemaphores    = &(*m_render_semaphore)};

    m_graphics_queue.queue.submit(submit, *m_render_fence);

    vk::PresentInfoKHR present_info{.waitSemaphoreCount = 1,
                                    .pWaitSemaphores    = &(*m_render_semaphore),
                                    .swapchainCount     = 1,
                                    .pSwapchains        = &(*m_swapchain.handle),
                                    .pImageIndices      = &swapchain_image_idx};

    val = m_graphics_queue.queue.presentKHR(present_info);
    ++m_frame_number;
}

void VulkanEngine::init_vulkan()
{
    vkb::InstanceBuilder builder;
    auto inst_ret = builder.set_app_name("Vulkan App")
                        .request_validation_layers(true)
                        .require_api_version(1, 3, 0)
                        .set_debug_callback(debug_callback)
                        .build();

    vkb::Instance vkb_inst = inst_ret.value();

    m_instance        = vk_initialisers::make_unique_instance(vkb_inst.instance);
    m_debug_messenger = vkb_inst.debug_messenger;

    m_surface = vk::UniqueSurfaceKHR{m_surface_callback(*m_instance), *m_instance};

    vkb::PhysicalDeviceSelector selector{vkb_inst};
    vkb::PhysicalDevice physical_device =
        selector.set_minimum_version(1, 3).set_surface(*m_surface).select().value();

    vkb::DeviceBuilder device_builder{physical_device};
    vkb::Device vkb_device = device_builder.build().value();

    // Physical devices aren't a resource, so they don't have to be kept in pointers.
    m_device     = vk_initialisers::make_unique_device(vkb_device.device);
    m_chosen_gpu = vkb_device.physical_device;

    m_graphics_queue.queue = vkb_device.get_queue(vkb::QueueType::graphics).value();
    m_graphics_queue.family_index =
        vkb_device.get_queue_index(vkb::QueueType::graphics).value();
}

void VulkanEngine::init_swapchain()
{
    vkb::SwapchainBuilder swapchain_builder{m_chosen_gpu, *m_device, *m_surface};

    vkb::Swapchain vkb_swapchain =
        swapchain_builder.use_default_format_selection()
            .set_desired_present_mode(VK_PRESENT_MODE_FIFO_KHR)
            .set_desired_extent(m_window_extent.width, m_window_extent.height)
            .build()
            .value();

    // Note that the swapchain is owned by the device, so by creating it like this we
    // guarantee that it will be destroyed before the device.
    m_swapchain.handle = vk::UniqueSwapchainKHR{vkb_swapchain.swapchain, *m_device};

    // Images are owned by the swapchain, so these get destroyed upon its destruction.
    vk_initialisers::to_vk_vector(vkb_swapchain.get_images().value(),
                                  m_swapchain.images,
                                  [](VkImage img) {
                                      return vk::Image{img};
                                  });

    // Views on the other hand are owned by the device, so make sure they're tied to it.
    vk_initialisers::to_vk_vector(vkb_swapchain.get_image_views().value(),
                                  m_swapchain.image_views,
                                  [this](VkImageView view) {
                                      return vk::UniqueImageView{view, *m_device};
                                  });

    m_swapchain.format = vk::Format{vkb_swapchain.image_format};
}

void VulkanEngine::init_commands()
{
    using namespace vk_initialisers;

    m_command_pool.pool = m_device->createCommandPoolUnique(
        command_pool_create_info(m_graphics_queue.family_index,
                                 vk::CommandPoolCreateFlagBits::eResetCommandBuffer));

    vk::CommandBufferAllocateInfo cmd_alloc_info{.commandPool = *m_command_pool.pool,
                                                 .level =
                                                     vk::CommandBufferLevel::ePrimary,

                                                 .commandBufferCount = 1};

    m_command_pool.command_buffer =
        m_device
            ->allocateCommandBuffers(
                command_buffer_allocate_info(*m_command_pool.pool,
                                             1,
                                             vk::CommandBufferLevel::ePrimary))
            .front();
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

    m_render_pass = m_device->createRenderPassUnique(render_pass_info);
}

void VulkanEngine::init_framebuffers()
{
    vk::FramebufferCreateInfo fb_info{.renderPass      = *m_render_pass,
                                      .attachmentCount = 1,
                                      .width           = m_window_extent.width,
                                      .height          = m_window_extent.height,
                                      .layers          = 1};

    m_framebuffers.resize(m_swapchain.images.size());
    for (int i{0}; auto& framebuffer : m_framebuffers)
    {
        fb_info.pAttachments = &(*m_swapchain.image_views[i]);
        framebuffer          = m_device->createFramebufferUnique(fb_info);
        ++i;
    }
}

void VulkanEngine::init_sync_structures()
{
    vk::FenceCreateInfo fence_create_info{.flags = vk::FenceCreateFlagBits::eSignaled};
    m_render_fence = m_device->createFenceUnique(fence_create_info);

    vk::SemaphoreCreateInfo semaphore_info;
    m_present_semaphore = m_device->createSemaphoreUnique(semaphore_info);
    m_render_semaphore  = m_device->createSemaphoreUnique(semaphore_info);
}
