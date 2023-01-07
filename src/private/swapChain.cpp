#include <swapChain.h>

#include <utils/queueFamily.h>

#include <GLFW/glfw3.h>

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <limits>

using VulkanRenderer::SwapChain;
using VulkanRenderer::SwapChainSupportDetails;

SwapChain::SwapChain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, GLFWwindow* window,
                     VkSwapchainKHR oldSwapChain)
    : m_deviceCache(device)
{
    SwapChainSupportDetails swapChainSupport;
    FillSwapChainSupportDetails(physicalDevice, surface, swapChainSupport);

    SelectSwapSurfaceFormat(swapChainSupport);
    SelectSwapPresentMode(swapChainSupport);
    SelectSwapChainExtent(swapChainSupport, window);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount)
        imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = m_surfaceFormat.format;
    createInfo.imageColorSpace = m_surfaceFormat.colorSpace;
    createInfo.imageExtent = m_extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    VulkanRenderer::QueueFamilyIndices indices = VulkanRenderer::FindQueueFamilies(physicalDevice, surface);
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

    if (indices.graphicsFamily != indices.presentFamily)
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    }
    else
    {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;     // Optional
        createInfo.pQueueFamilyIndices = nullptr; // Optional
    }

    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = m_presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = oldSwapChain;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
    {
        std::cout << "Failed to create swap chain!" << std::endl;
        m_swapChain = VK_NULL_HANDLE;
        return;
    }

    vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(device, m_swapChain, &imageCount, m_images.data());
}

SwapChain::~SwapChain() { vkDestroySwapchainKHR(m_deviceCache, m_swapChain, nullptr); }

void SwapChain::FillSwapChainSupportDetails(VkPhysicalDevice device, VkSurfaceKHR surface,
                                            SwapChainSupportDetails& outDetails)
{
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface, &outDetails.capabilities);

    uint32_t formatCount = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, nullptr);

    if (formatCount != 0)
    {
        outDetails.formats.resize(formatCount);
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface, &formatCount, outDetails.formats.data());
    }

    uint32_t presentModeCount = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, nullptr);

    if (presentModeCount != 0)
    {
        outDetails.presentModes.resize(presentModeCount);
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface, &presentModeCount, outDetails.presentModes.data());
    }
}

void SwapChain::SelectSwapSurfaceFormat(const SwapChainSupportDetails& details)
{
    for (const auto& availableFormat : details.formats)
    {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
        {
            m_surfaceFormat = availableFormat;
            return;
        }
    }

    m_surfaceFormat = details.formats.empty() ? VkSurfaceFormatKHR{} : details.formats[0];
}

void SwapChain::SelectSwapPresentMode(const SwapChainSupportDetails& details)
{
    for (const auto& availablePresentMode : details.presentModes)
    {
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
        {
            m_presentMode = availablePresentMode;
            return;
        }
    }

    m_presentMode = VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChain::SelectSwapChainExtent(const SwapChainSupportDetails& details, GLFWwindow* window)
{
    if (details.capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
    {
        m_extent = details.capabilities.currentExtent;
    }
    else
    {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        VkExtent2D actualExtent = {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};

        actualExtent.width = std::clamp(actualExtent.width, details.capabilities.minImageExtent.width,
                                        details.capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, details.capabilities.minImageExtent.height,
                                         details.capabilities.maxImageExtent.height);

        m_extent = actualExtent;
    }
}
