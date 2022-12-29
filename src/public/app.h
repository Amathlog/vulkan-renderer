#pragma once

struct GLFWwindow;
struct VkInstance_T;
struct VkPhysicalDevice_T;

namespace VulkanRenderer
{
class Application
{
public:
    Application(int width, int height, const char* windowName);
    ~Application();

    int Init();
    int Run();

private:
    int InitWindow();
    int InitVulkan();
    int CreateInstance();
    int PickPhysicalDevice();
    int Cleanup();

    bool m_initialized = false;
    int m_width;
    int m_height;
    const char* m_windowName = "";
    GLFWwindow* m_window = nullptr;
    VkInstance_T* m_instance = nullptr;
    VkPhysicalDevice_T* m_physicalDevice = nullptr;
};
} // namespace VulkanRenderer