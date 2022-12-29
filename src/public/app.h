#pragma once

struct GLFWwindow;
struct VkInstance_T;
struct VkDevice_T;
struct VkPhysicalDevice_T;
struct VkQueue_T;

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
    int CreateLogicalDevice();
    int Cleanup();

    bool m_initialized = false;
    int m_width;
    int m_height;
    const char* m_windowName = "";
    GLFWwindow* m_window = nullptr;

    // Vulkan handles
    VkInstance_T* m_instance = nullptr;
    VkPhysicalDevice_T* m_physicalDevice = nullptr;
    VkDevice_T* m_device = nullptr;
    VkQueue_T* m_graphicsQueue = nullptr;
};
} // namespace VulkanRenderer