#pragma once

#include <array>
#include <cstdint>
#include <memory>
#include <optional>
#include <vector>

struct GLFWwindow;
struct SwapChainSupportDetails;
struct VkCommandBuffer_T;
struct VkCommandPool_T;
struct VkDevice_T;
struct VkFence_T;
struct VkFramebuffer_T;
struct VkInstance_T;
struct VkPhysicalDevice_T;
struct VkQueue_T;
struct VkSemaphore_T;
struct VkSurfaceKHR_T;

namespace VulkanRenderer
{
class GraphicPipeline;
class SwapChain;

constexpr int maxFramesInFlight = 2; // Allow the CPU to prepare next frame while GPU is rendering the other.

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
    int DrawFrame();

    static void FramebufferResizeCallback(GLFWwindow* window, int width, int height);

    // Init vulkan subfuctions
    int CreateInstance();
    int CreateLogicalDevice();
    int CreateSurface();
    int PickPhysicalDevice();
    int CreateSwapChain();
    int CreateGraphicPipeline();
    int CreateFramebuffers();
    int CreateCommandPool();
    int CreateCommandBuffer();
    int CreateSyncObjects();

    // Swap chain specific
    int RecreateSwapChain();
    int CleanupSwapChain();

    // Command buffer
    int RecordCommandBuffer(VkCommandBuffer_T* commandBuffer, uint32_t imageIndex);

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
    std::vector<VkFramebuffer_T*> m_framebuffers{};
    VkCommandPool_T* m_commandPool = nullptr;
    std::array<VkCommandBuffer_T*, maxFramesInFlight> m_commandBuffers{};

    // Sync objects
    std::array<VkSemaphore_T*, maxFramesInFlight> m_imageAvailableSemaphores{};
    std::array<VkSemaphore_T*, maxFramesInFlight> m_renderFinishedSemaphores{};
    std::array<VkFence_T*, maxFramesInFlight> m_inFlightFences{};

    // Utility
    bool m_framebufferResized = false;
    int m_currentFrame = 0;
};
} // namespace VulkanRenderer