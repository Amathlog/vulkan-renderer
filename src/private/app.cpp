#include <app.h>

#include <array>
#include <cstring>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>

#include <config.h>
#include <utils/utils.h>
#include <utils/verboseDump.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using VulkanRenderer::Application;
using VulkanRenderer::QueueFamilyIndices;

namespace Cst
{
constexpr std::array<const char*, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif

static const float singleQueuePriority = 1.0f;
} // namespace Cst

Application::Application(int width, int height, const char* windowName)
    : m_width(width)
    , m_height(height)
    , m_windowName(windowName)
{
}

Application::~Application() { Cleanup(); }

int Application::Init()
{
    if (m_initialized)
        return 0;

    int res = InitWindow();
    if (res != 0)
        return res;

    res = InitVulkan();
    if (res != 0)
        return res;

    m_initialized = true;
    return 0;
}

int Application::Run()
{
    if (!m_initialized)
        return -1;

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }

    return 0;
}

int Application::InitWindow()
{
    glfwInit();

    // First hint glfw to not initialize an OpenGL context
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

    // Don't allow resize for now
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

    m_window = glfwCreateWindow(m_width, m_height, m_windowName, nullptr, nullptr);

    if (m_window == nullptr)
    {
        std::cout << "Failed to create window" << std::endl;
        return -1;
    }

    return 0;
}

int Application::InitVulkan()
{
    if (m_initialized)
        return 0;

    // clang-format off
    auto AllInitFunctions = {
        &Application::CreateInstance,
        &Application::CreateSurface,
        &Application::PickPhysicalDevice,
        &Application::CreateLogicalDevice
    };
    // clang-format on

    for (auto Func : AllInitFunctions)
    {
        int res = (this->*Func)();
        if (res != 0)
            return res;
    }

    return 0;
}

int Application::CreateInstance()
{
    if (m_instance != nullptr)
        return 0;

    // Optional information about the application
    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = m_windowName;
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    // Information to create a VkInstance
    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    // It needs a list of extensions. We use glfw information to know all the extensions we need
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

    // Also gather all supported extensions by our graphic device
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);

    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    // Making sure all extensions required by glfw are supported
    uint32_t extensionValidation = VulkanRenderer::Utils::ValidateStrings(
        glfwExtensions, glfwExtensionCount, extensions.data(), extensionCount,
        [](const VkExtensionProperties& extensionProperty) -> const char* { return extensionProperty.extensionName; });

    if (extensionValidation != glfwExtensionCount)
    {
        std::cout << "Extension " << glfwExtensions[extensionValidation] << " required by glfw not supported..."
                  << std::endl;
        return -1;
    }

    // When we verified that all extensions are supported, continue filling the create info struct
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // Do the same with validation layers
    if constexpr (Cst::enableValidationLayers)
    {
        uint32_t validationLayersCount = 0;
        vkEnumerateInstanceLayerProperties(&validationLayersCount, nullptr);

        std::vector<VkLayerProperties> validationLayers(validationLayersCount);
        vkEnumerateInstanceLayerProperties(&validationLayersCount, validationLayers.data());

        uint32_t validationLayersValidation = VulkanRenderer::Utils::ValidateStrings(
            Cst::validationLayers.data(), (uint32_t)Cst::validationLayers.size(), validationLayers.data(),
            validationLayersCount,
            [](const VkLayerProperties& layerProperty) -> const char* { return layerProperty.layerName; });

        if (extensionValidation != glfwExtensionCount)
        {
            std::cout << "Extension " << glfwExtensions[extensionValidation] << " required by glfw not supported..."
                      << std::endl;
            return -1;
        }

        createInfo.enabledLayerCount = static_cast<uint32_t>(Cst::validationLayers.size());
        createInfo.ppEnabledLayerNames = Cst::validationLayers.data();
    }
    else
    {
        createInfo.enabledLayerCount = 0;
    }

    // Finally try to create the instance
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        std::cout << "Failed to create Vulkan instance" << std::endl;
        return -1;
    }

    return 0;
}

int Application::CreateSurface()
{
    if (glfwCreateWindowSurface(m_instance, m_window, nullptr, &m_surface) != VK_SUCCESS)
        return -1;

    return 0;
}

int Application::PickPhysicalDevice()
{
    uint32_t nbDevicesFound = 0;
    vkEnumeratePhysicalDevices(m_instance, &nbDevicesFound, nullptr);

    if (nbDevicesFound == 0)
    {
        std::cout << "Found no capable physical devices." << std::endl;
        return -1;
    }

    if (VulkanRenderer::Parameters().verbose())
        std::cout << "Found " << nbDevicesFound << " physical devices" << std::endl;

    std::vector<VkPhysicalDevice> devices(nbDevicesFound);
    vkEnumeratePhysicalDevices(m_instance, &nbDevicesFound, devices.data());

    int i = 0;
    int selectedDevice = 0;

    for (int i = 0; i < (int)devices.size(); ++i)
    {
        if (VulkanRenderer::Parameters().verbose())
            std::cout << "Device " << i << ":" << std::endl;

        if (!IsSuitableDevice(devices[i]))
            continue;

        if (VulkanRenderer::Parameters().forceSelectedDevice.count() > 0 &&
            VulkanRenderer::Parameters().forceSelectedDevice() != i)
            continue;

        if (m_physicalDevice == nullptr)
        {
            selectedDevice = i;
            m_physicalDevice = devices[i];
        }
    }

    if (VulkanRenderer::Parameters().verbose())
    {
        std::cout << "Selected device: " << selectedDevice << std::endl;
    }

    if (m_physicalDevice == nullptr)
    {
        std::cout << "Found no suitable physical devices." << std::endl;
        return -1;
    }

    return 0;
}

int Application::CreateLogicalDevice()
{
    QueueFamilyIndices indices = FindQueueFamilies(m_physicalDevice);

    // Store information on all queues we want to gather and their family queue index.
    // clang-format off
    using AllQueuesInfos = std::vector<std::tuple<uint32_t, VkQueue*, const char*>>;
    AllQueuesInfos allQueues = {
        {indices.graphicsFamily.value(), &m_graphicsQueue, "graphic"},
        {indices.presentFamily.value(), &m_presentQueue, "presentation"}
    };
    // clang-format on

    // But each queue needs to have a unique family queue index.
    // It is possible for example that graphicQueue and presentQueue have the same
    // family queue index.
    // To be sure we only create one queue for this, we keep the unique indexes in a map.
    std::map<uint32_t, AllQueuesInfos> mapQueueIndexes;

    // Gather all create structs in this vector
    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;

    for (auto& queue : allQueues)
    {
        uint32_t queueFamilyIndex = std::get<uint32_t>(queue);
        auto it = mapQueueIndexes.find(queueFamilyIndex);
        if (it == mapQueueIndexes.end())
        {
            it = mapQueueIndexes.emplace(queueFamilyIndex, AllQueuesInfos()).first;

            VkDeviceQueueCreateInfo& queueCreateInfo = queueCreateInfos.emplace_back();
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamilyIndex;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &Cst::singleQueuePriority;
        }

        it->second.emplace_back(queue);
    }

    // We need to ask for specific features if we want to use them.
    // For now, not used
    VkPhysicalDeviceFeatures deviceFeatures{};

    VkDeviceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.pEnabledFeatures = &deviceFeatures;

    if (vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device) != VK_SUCCESS)
    {
        std::cout << "Failed to create a logical device" << std::endl;
        return -1;
    }

    // When logical device is created, we need to get a handle on all the queues we requested
    uint32_t i = 0;
    for (auto it : mapQueueIndexes)
    {
        for (auto queue : it.second)
        {
            VkQueue* vkQueue = std::get<VkQueue*>(queue);
            vkGetDeviceQueue(m_device, std::get<uint32_t>(queue), static_cast<uint32_t>(i), vkQueue);

            if (vkQueue == nullptr)
            {
                std::cout << "Failed to gather the " << std::get<const char*>(queue) << " queue" << std::endl;
                return -1;
            }
        }

        i++;
    }

    return 0;
}

int Application::Cleanup()
{
    if (m_device)
        vkDestroyDevice(m_device, nullptr);

    if (m_surface)
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);

    if (m_instance)
        vkDestroyInstance(m_instance, nullptr);

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    m_window = nullptr;
    m_initialized = false;

    return 0;
}

QueueFamilyIndices Application::FindQueueFamilies(VkPhysicalDevice device)
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
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);

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

bool Application::IsSuitableDevice(VkPhysicalDevice device)
{
    // Gather properties of the device
    VkPhysicalDeviceProperties deviceProperties;
    vkGetPhysicalDeviceProperties(device, &deviceProperties);

    // Also gather features of the device
    VkPhysicalDeviceFeatures deviceFeatures;
    vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

    if (VulkanRenderer::Parameters().verbose())
    {
        std::cout << VulkanRenderer::Utils::PhysicalDevicePropertiesDump(deviceProperties) << std::endl;
        std::cout << VulkanRenderer::Utils::PhysicalDeviceFeaturesDump(deviceFeatures) << std::endl;
    }

    // TODO: Add more capabilities as we need it.
    QueueFamilyIndices indices = FindQueueFamilies(device);
    return indices.IsComplete();
}