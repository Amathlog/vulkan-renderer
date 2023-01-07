#pragma once

#include <vulkan/vulkan.h>

#include <cstdint>
#include <optional>
#include <vector>

namespace VulkanRenderer
{
struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool IsComplete() const { return graphicsFamily.has_value() && presentFamily.has_value(); }
};

inline QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device, VkSurfaceKHR surface)
{
    QueueFamilyIndices indices;

    uint32_t queueFamilyCount = 0;
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
    vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

    uint32_t i = 0;

    for (VkQueueFamilyProperties& properties : queueFamilies)
    {
        // Check first if this queue family has graphic support
        if (!!(properties.queueFlags & VK_QUEUE_GRAPHICS_BIT))
            indices.graphicsFamily = i;

        // Then check if this queue family has presentation support
        VkBool32 presentSupport = false;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);

        if (presentSupport)
            indices.presentFamily = i;

        if (indices.IsComplete())
            break;

        ++i;
    }

    // Then check if it supports
    i = 0;

    return indices;
}
} // namespace VulkanRenderer