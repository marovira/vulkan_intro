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
}

void VulkanEngine::render()
{}

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
