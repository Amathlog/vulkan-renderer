#pragma once

#include <string>

struct VkPhysicalDeviceProperties;
struct VkPhysicalDeviceFeatures;

namespace VulkanRenderer
{
namespace Utils
{
std::string PhysicalDevicePropertiesDump(const VkPhysicalDeviceProperties& properties);
std::string PhysicalDeviceFeaturesDump(const VkPhysicalDeviceFeatures& features);
} // namespace Utils
} // namespace VulkanRenderer