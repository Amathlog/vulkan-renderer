#pragma once

#include <cstdint>
#include <memory>
#include <optional>

struct GLFWwindow;
struct SwapChainSupportDetails;
struct VkDevice_T;
struct VkInstance_T;
struct VkPhysicalDevice_T;
struct VkQueue_T;
struct VkSurfaceKHR_T;

namespace VulkanRenderer
{
class GraphicPipeline;
class SwapChain;

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
    int CreateSwapChain();
    int CreateGraphicPipeline();

    // Vulkan queue family specific
    bool IsSuitableDevice(VkPhysicalDevice_T* device) const;
    bool CheckDeviceExtensionSupport(VkPhysicalDevice_T* device) const;

    // Window specific
    bool m_initialized = false;
    int m_width;
    int m_height;
    const char* m_windowName = "";
    GLFWwindow* m_window = nullptr;

    std::unique_ptr<SwapChain> m_swapChain;
    std::unique_ptr<GraphicPipeline> m_graphicPipeline;

    // Vulkan handles
    VkInstance_T* m_instance = nullptr;
    VkPhysicalDevice_T* m_physicalDevice = nullptr;
    VkDevice_T* m_device = nullptr;
    VkQueue_T* m_graphicsQueue = nullptr;
    VkQueue_T* m_presentQueue = nullptr;
    VkSurfaceKHR_T* m_surface = nullptr;
};
} // namespace VulkanRenderer