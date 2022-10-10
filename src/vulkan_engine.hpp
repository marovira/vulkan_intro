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
        using UniqueImageView = std::unique_ptr<vk::raii::ImageView>;

        std::unique_ptr<vk::raii::SwapchainKHR> handle;
        vk::Format format;
        std::vector<vk::Image> images;
        std::vector<UniqueImageView> image_views;
    };

    struct Queue
    {
        // Queues are similar to physical devices in that they're not
        // really resources tied to anything, so we're going to keep it out
        // of the RAII namespace.
        vk::Queue queue;
        std::uint32_t family_index{0};
    };

    struct CommandPool
    {
        std::unique_ptr<vk::raii::CommandPool> pool;
        vk::raii::CommandBuffers command_buffers{nullptr};
    };

    void init_vulkan();
    void init_swapchain();
    void init_commands();
    void init_default_render_pass();
    void init_framebuffers();
    void init_sync_structures();

    int m_frame_number{0};
    SurfaceCallback m_surface_callback;
    vk::Extent2D m_window_extent;

    std::unique_ptr<vk::raii::Context> m_context;
    std::unique_ptr<vk::raii::Instance> m_instance;

    std::unique_ptr<vk::raii::DebugUtilsMessengerEXT> m_debug_messenger;
    std::unique_ptr<vk::raii::SurfaceKHR> m_surface;

    std::unique_ptr<vk::raii::Device> m_device;
    vk::PhysicalDevice m_chosen_gpu;

    Swapchain m_swapchain;
    Queue m_graphics_queue;
    CommandPool m_command_pool;

    std::unique_ptr<vk::raii::RenderPass> m_render_pass;

    std::vector<vk::raii::Framebuffer> m_framebuffers;

    std::unique_ptr<vk::raii::Semaphore> m_present_semaphore;
    std::unique_ptr<vk::raii::Semaphore> m_render_semaphore;
    std::unique_ptr<vk::raii::Fence> m_render_fence;
};
