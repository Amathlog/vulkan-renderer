#pragma once

#include <cstdint>
#include <optional>

struct GLFWwindow;
struct VkDevice_T;
struct VkInstance_T;
struct VkPhysicalDevice_T;
struct VkQueue_T;
struct VkSurfaceKHR_T;

namespace VulkanRenderer
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

class Application
{
public:
    Application(int width, int height, const char* windowName);
    ~Application();

    // To be called once. Return 0 if all is good.
    int Init();

    // Simple run, will wait for the window to be closed.
    int Run();

private:
    int Cleanup();
    int InitWindow();
    int InitVulkan();

    // Init vulkan subfuctions
    int CreateInstance();
    int CreateLogicalDevice();
    int CreateSurface();
    int PickPhysicalDevice();

    // Vulkan queue family specific
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice_T* device);
    bool IsSuitableDevice(VkPhysicalDevice_T* device);

    // Window specific
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
    VkQueue_T* m_presentQueue = nullptr;
    VkSurfaceKHR_T* m_surface = nullptr;
};
} // namespace VulkanRenderer