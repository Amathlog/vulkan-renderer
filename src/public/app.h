#pragma once

struct GLFWwindow;

class VulkanApplication
{
public:
    VulkanApplication(int width, int height, const char* windowName);
    ~VulkanApplication();

    int Init();
    int Run();

private:
    int InitWindow();
    int InitVulkan();
    int Cleanup();

    bool m_initialized = false;
    int m_width;
    int m_height;
    const char* m_windowName = "";
    GLFWwindow* m_window = nullptr;
};