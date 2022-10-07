#pragma once

class VulkanEngine;

class VulkanApp
{
public:
    VulkanApp();
    ~VulkanApp();

    void run();

private:
    void on_mouse_press(int button, int action, int mods, double x, double y);
    void on_mouse_move(double x, double y);
    void on_key_press(int key, int scancode, int action, int mods);
    void on_window_size(int width, int height);
    void on_framebuffer_size(int width, int height);
    void on_char(unsigned int codepoint);
    void on_close();

    GLFWwindow* m_window{nullptr};
    std::unique_ptr<VulkanEngine> m_engine;
};
