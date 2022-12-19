#pragma once

struct GLFWwindow;
struct VkInstance_T;

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
    int CreateInstance();
    int Cleanup();

    bool m_initialized = false;
    int m_width;
    int m_height;
    const char* m_windowName = "";
    GLFWwindow* m_window = nullptr;
    VkInstance_T* m_instance = nullptr;
};