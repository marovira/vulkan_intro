#pragma once

using SurfaceCallback = std::function<VkSurfaceKHR(vk::Instance const&)>;

class VulkanEngine
{
public:
    VulkanEngine() = default;
    ~VulkanEngine();

    void set_surface_callback(SurfaceCallback&& callback);
    void set_window_extent(vk::Extent2D extent);

    void init();

    void render();

private:
    struct Swapchain
    {
        vk::UniqueSwapchainKHR handle;
        vk::Format format;
        std::vector<vk::Image> images;
        std::vector<vk::UniqueImageView> image_views;
    };

    struct GraphicsQueue
    {
        vk::Queue queue;
        std::uint32_t family_index{0};
    };

    struct CommandPool
    {
        vk::UniqueCommandPool pool;
        vk::CommandBuffer command_buffer;
    };

    void init_vulkan();
    void init_swapchain();
    void init_commands();

    int m_frame_number{0};
    SurfaceCallback m_surface_callback;
    vk::Extent2D m_window_extent;

    vk::UniqueInstance m_instance;
    vk::DebugUtilsMessengerEXT m_debug_messenger;
    vk::UniqueSurfaceKHR m_surface;

    vk::UniqueDevice m_device;
    vk::PhysicalDevice m_chosen_gpu;

    Swapchain m_swapchain;
    GraphicsQueue m_graphics_queue;
    CommandPool m_command_pool;
};
