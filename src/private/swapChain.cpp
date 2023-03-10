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
                     SwapChain* oldSwapChain)
    : m_deviceCache(device)
{
    SwapChainSupportDetails swapChainSupport;
    FillSwapChainSupportDetails(physicalDevice, surface, swapChainSupport);

    // Add some logic to select some settings on the swap chain
    SelectSwapSurfaceFormat(swapChainSupport);
    SelectSwapPresentMode(swapChainSupport);
    SelectSwapChainExtent(swapChainSupport, window);

    // To avoid waiting for the device, we allow the swap chain to process 1 more image than the minimum.
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;

    // Also making sure we don't go over the max number of images supported.
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

    // In case of an old swap chain, we pass it there.
    createInfo.oldSwapchain = oldSwapChain ? oldSwapChain->m_swapChain : VK_NULL_HANDLE;

    if (vkCreateSwapchainKHR(device, &createInfo, nullptr, &m_swapChain) != VK_SUCCESS)
    {
        std::cout << "Failed to create swap chain!" << std::endl;
        m_swapChain = VK_NULL_HANDLE;
        return;
    }

    InitImages(imageCount);
}

SwapChain::~SwapChain()
{
    for (VkImageView imageView : m_imageViews)
    {
        vkDestroyImageView(m_deviceCache, imageView, nullptr);
    }

    vkDestroySwapchainKHR(m_deviceCache, m_swapChain, nullptr);
}

void SwapChain::InitImages(uint32_t imageCount)
{
    // First resize the image vector, and gather it from the device
    vkGetSwapchainImagesKHR(m_deviceCache, m_swapChain, &imageCount, nullptr);
    m_images.resize(imageCount);
    vkGetSwapchainImagesKHR(m_deviceCache, m_swapChain, &imageCount, m_images.data());

    // Also resize the image view vector
    m_imageViews.resize(imageCount);

    // And for each image, create its associated view
    for (uint32_t i = 0; i < imageCount; ++i)
    {
        VkImageViewCreateInfo imageViewCreateInfo{};
        imageViewCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        imageViewCreateInfo.image = m_images[i];
        imageViewCreateInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        imageViewCreateInfo.format = m_surfaceFormat.format;
        imageViewCreateInfo.components = {VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY,
                                          VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY};

        imageViewCreateInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        imageViewCreateInfo.subresourceRange.baseMipLevel = 0;
        imageViewCreateInfo.subresourceRange.levelCount = 1;
        imageViewCreateInfo.subresourceRange.baseArrayLayer = 0;
        imageViewCreateInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_deviceCache, &imageViewCreateInfo, nullptr, &m_imageViews[i]) != VK_SUCCESS)
        {
            std::cout << "Failed to create image view number " << i << std::endl;
            m_imageViews.resize(0);
            return;
        }
    }
}

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
