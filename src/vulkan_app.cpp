#include "vulkan_app.hpp"
#include "vulkan_engine.hpp"

#include <zeus/assert.hpp>

static constexpr auto window_width{1700};
static constexpr auto window_height{900};

struct WindowCallbacks
{
    std::function<void(int, int, int, double, double)> mouse_press_callback{};
    std::function<void(double, double)> mouse_move_callback{};
    std::function<void(double, double)> mouse_scroll_callback{};
    std::function<void(int, int, int, int)> key_press_callback{};
    std::function<void(int, int)> window_size_callback{};
    std::function<void(int, int)> framebuffer_size_callback{};
    std::function<void(unsigned int)> char_callback{};
    std::function<void()> window_close_callback{};
};

static void glfw_error_callback(int code, char const* message)
{
    fmt::print("error ({}): {}\n", code, message);
}

static void mouse_press_callback(GLFWwindow* window, int button, int action, int mods)
{
    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(window));
    if (callbacks == nullptr)
    {
        return;
    }

    if (callbacks->mouse_press_callback)
    {
        double x;
        double y;
        glfwGetCursorPos(window, &x, &y);
        callbacks->mouse_press_callback(button, action, mods, x, y);
    }
}

static void mouse_move_callback(GLFWwindow* window, double xPos, double yPos)
{
    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(window));
    if (callbacks == nullptr)
    {
        return;
    }

    if (callbacks->mouse_move_callback)
    {
        callbacks->mouse_move_callback(xPos, yPos);
    }
}

static void mouse_scroll_callback(GLFWwindow* window, double xOffset, double yOffset)
{
    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(window));
    if (callbacks == nullptr)
    {
        return;
    }

    if (callbacks->mouse_scroll_callback)
    {
        callbacks->mouse_scroll_callback(xOffset, yOffset);
    }
}

static void
key_press_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, true);
        return;
    }

    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(window));
    if (callbacks == nullptr)
    {
        return;
    }

    if (callbacks->key_press_callback)
    {
        callbacks->key_press_callback(key, scancode, action, mods);
    }
}

static void window_size_callback(GLFWwindow* window, int width, int height)
{
    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(window));
    if (callbacks == nullptr)
    {
        return;
    }

    if (callbacks->window_size_callback)
    {
        callbacks->window_size_callback(width, height);
    }
}

static void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(window));
    if (callbacks == nullptr)
    {
        return;
    }

    if (callbacks->framebuffer_size_callback)
    {
        callbacks->framebuffer_size_callback(width, height);
    }
}

static void char_callback(GLFWwindow* window, unsigned int codepoint)
{
    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(window));
    if (callbacks == nullptr)
    {
        return;
    }

    if (callbacks->char_callback)
    {
        callbacks->char_callback(codepoint);
    }
}

static void window_close_callback(GLFWwindow* window)
{
    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(window));
    if (callbacks == nullptr)
    {
        return;
    }

    if (callbacks->window_close_callback)
    {
        callbacks->window_close_callback();
    }
}

VulkanApp::VulkanApp()
{
    glfwSetErrorCallback(glfw_error_callback);
    if (!glfwInit())
    {
        fmt::print("error: could not initialise GLFW\n");
        ASSERT(0);
        std::abort();
    }

    if (!glfwVulkanSupported())
    {
        fmt::print("error: Vulkan is not supported\n");
        ASSERT(0);
        std::abort();
    }

    // Initialise the window to not be full-size or resizeable (mainly so we don't need to
    // worry about details like rebuilding the swap chain).
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_MAXIMIZED, 0);
    glfwWindowHint(GLFW_RESIZABLE, 0);

    m_window =
        glfwCreateWindow(window_width, window_height, "Vulkan App", nullptr, nullptr);
    if (!m_window)
    {
        ASSERT(0);
        std::abort();
    }

    auto calls = new WindowCallbacks;
    calls->mouse_press_callback =
        [this](int button, int action, int mods, double x, double y) {
            on_mouse_press(button, action, mods, x, y);
        };
    calls->mouse_move_callback = [this](double x, double y) {
        on_mouse_move(x, y);
    };
    calls->key_press_callback = [this](int key, int scancode, int action, int mods) {
        on_key_press(key, scancode, action, mods);
    };
    calls->window_size_callback = [this](int width, int height) {
        on_window_size(width, height);
    };
    calls->framebuffer_size_callback = [this](int width, int height) {
        on_framebuffer_size(width, height);
    };
    calls->char_callback = [this](unsigned int codepoint) {
        on_char(codepoint);
    };
    calls->window_close_callback = [this]() {
        on_close();
    };

    glfwSetWindowUserPointer(m_window, calls);

    glfwSetKeyCallback(m_window, key_press_callback);
    glfwSetMouseButtonCallback(m_window, mouse_press_callback);
    glfwSetScrollCallback(m_window, mouse_scroll_callback);
    glfwSetCursorPosCallback(m_window, mouse_move_callback);

    glfwSetWindowSizeCallback(m_window, window_size_callback);
    glfwSetFramebufferSizeCallback(m_window, framebuffer_size_callback);
    glfwSetWindowCloseCallback(m_window, window_close_callback);
    glfwSetCharCallback(m_window, char_callback);

    m_engine = std::make_unique<VulkanEngine>();
    m_engine->set_surface_callback([this](vk::Instance const& instance) {
        VkSurfaceKHR surface;
        glfwCreateWindowSurface(instance, m_window, nullptr, &surface);
        return surface;
    });
    m_engine->set_window_extent({window_width, window_height});
    m_engine->init();
}

VulkanApp::~VulkanApp()
{
    // Make sure the engine is deleted BEFORE we destroy the GLFW state.
    m_engine = nullptr;

    auto callbacks = static_cast<WindowCallbacks*>(glfwGetWindowUserPointer(m_window));
    delete callbacks;
    glfwSetWindowUserPointer(m_window, nullptr);
    glfwDestroyWindow(m_window);
    glfwTerminate();
}

void VulkanApp::run()
{
    while (!glfwWindowShouldClose(m_window))
    {
        m_engine->render();
        glfwPollEvents();
    }
}

void VulkanApp::on_mouse_press([[maybe_unused]] int button,
                               [[maybe_unused]] int action,
                               [[maybe_unused]] int mods,
                               [[maybe_unused]] double x,
                               [[maybe_unused]] double y)
{}

void VulkanApp::on_mouse_move([[maybe_unused]] double x, [[maybe_unused]] double y)
{}

void VulkanApp::on_key_press(int key,
                             [[maybe_unused]] int scancode,
                             int action,
                             [[maybe_unused]] int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(m_window, GLFW_TRUE);
    }
}

void VulkanApp::on_window_size([[maybe_unused]] int width, [[maybe_unused]] int height)
{}

void VulkanApp::on_framebuffer_size([[maybe_unused]] int width,
                                    [[maybe_unused]] int height)
{}

void VulkanApp::on_char([[maybe_unused]] unsigned int codepoint)
{}

void VulkanApp::on_close()
{}
