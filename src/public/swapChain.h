#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct GLFWwindow;

namespace VulkanRenderer
{
struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class SwapChain
{
public:
    SwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window,
              SwapChain* oldSwapChain = nullptr);
    ~SwapChain();

    bool IsValid() const { return m_swapChain != VK_NULL_HANDLE && !m_imageViews.empty(); }

    VkSwapchainKHR GetSwapChain() const { return m_swapChain; }
    std::vector<VkImage>& GetImages() { return m_images; }
    std::vector<VkImageView>& GetImageViews() { return m_imageViews; }
    VkFormat GetFormat() const { return m_surfaceFormat.format; }
    VkExtent2D GetExtent() const { return m_extent; }

    static void FillSwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface,
                                            SwapChainSupportDetails& outDetails);

private:
    void SelectSwapSurfaceFormat(const SwapChainSupportDetails& details);
    void SelectSwapPresentMode(const SwapChainSupportDetails& details);
    void SelectSwapChainExtent(const SwapChainSupportDetails& details, GLFWwindow* window);

    void InitImages(uint32_t imageCount);

    VkDevice m_deviceCache = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_images;
    std::vector<VkImageView> m_imageViews;
    VkSurfaceFormatKHR m_surfaceFormat;
    VkPresentModeKHR m_presentMode;
    VkExtent2D m_extent;
};
} // namespace VulkanRenderer