#include <utils/verboseDump.h>

#include <vulkan/vulkan.h>

#include <sstream>

namespace
{
const char* DeviceTypeToString(VkPhysicalDeviceType deviceType)
{
    switch (deviceType)
    {
    case VK_PHYSICAL_DEVICE_TYPE_INTEGRATED_GPU:
        return "Integrated GPU";
    case VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU:
        return "Discrete GPU";
    case VK_PHYSICAL_DEVICE_TYPE_VIRTUAL_GPU:
        return "Virtual GPU";
    case VK_PHYSICAL_DEVICE_TYPE_CPU:
        return "CPU";
    default:
        return "Other";
    }
}

// Convert vendor specific driver version string
std::string GetDriverVerson(const VkPhysicalDeviceProperties& properties)
{
    if (properties.driverVersion == 0)
    {
        return "";
    }

    std::stringstream stream;
    // NVIDIA
    if (properties.vendorID == 4318)
    {
        stream << ((properties.driverVersion >> 22) & 0x3ff) << "." << ((properties.driverVersion >> 14) & 0xff) << "."
               << ((properties.driverVersion >> 6) & 0x0ff) << "." << (properties.driverVersion & 0x3);
    }
    else
    {
        stream << VK_API_VERSION_MAJOR(properties.driverVersion) << "."
               << VK_API_VERSION_MINOR(properties.driverVersion) << "."
               << VK_API_VERSION_PATCH(properties.driverVersion);
    }
    return stream.str();
}
} // namespace

std::string VulkanRenderer::Utils::PhysicalDevicePropertiesDump(const VkPhysicalDeviceProperties& properties)
{
    std::stringstream stream;

    const char* tab = "  ";

    stream << "Device properties:" << std::endl;
    stream << tab << "- Api Version: " << VK_API_VERSION_MAJOR(properties.apiVersion) << "."
           << VK_API_VERSION_MINOR(properties.apiVersion) << "." << VK_API_VERSION_PATCH(properties.apiVersion)
           << std::endl;
    stream << tab << "- Driver Version: " << GetDriverVerson(properties) << std::endl;
    stream << tab << "- Vendor Id: " << properties.vendorID << std::endl;
    stream << tab << "- Device Id: " << properties.deviceID << std::endl;
    stream << tab << "- Device Type: " << DeviceTypeToString(properties.deviceType) << std::endl;
    stream << tab << "- Device Name: " << properties.deviceName << std::endl;

    return stream.str();
}

std::string VulkanRenderer::Utils::PhysicalDeviceFeaturesDump(const VkPhysicalDeviceFeatures& features)
{
    std::stringstream stream;
    return stream.str();
}