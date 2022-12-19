#include <app.h>

#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

VulkanApplication::VulkanApplication(int width, int height, const char* windowName)
    : m_width(width)
    , m_height(height)
    , m_windowName(windowName)
{
}

VulkanApplication::~VulkanApplication() { Cleanup(); }

int VulkanApplication::Init()
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

int VulkanApplication::Run()
{
    if (!m_initialized)
        return -1;

    while (!glfwWindowShouldClose(m_window))
    {
        glfwPollEvents();
    }

    return 0;
}

int VulkanApplication::InitWindow()
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

int VulkanApplication::InitVulkan()
{
    if (m_initialized)
        return 0;

    CreateInstance();
    return 0;
}

int VulkanApplication::CreateInstance()
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
    for (uint32_t i = 0; i < glfwExtensionCount; ++i)
    {
        bool found = false;
        for (VkExtensionProperties& extensionProperties : extensions)
        {
            if (std::strcmp(extensionProperties.extensionName, glfwExtensions[i]) == 0)
            {
                found = true;
                break;
            }
        }

        if (!found)
        {
            std::cout << "Extension " << glfwExtensions[i] << " required by glfw not supported..." << std::endl;
            return -1;
        }
    }

    // When we verified that all extensions are supported, finish filling the create info struct
    createInfo.enabledExtensionCount = glfwExtensionCount;
    createInfo.ppEnabledExtensionNames = glfwExtensions;

    // No validation layer for now
    createInfo.enabledLayerCount = 0;

    // Finally try to create the instance
    if (vkCreateInstance(&createInfo, nullptr, &m_instance) != VK_SUCCESS)
    {
        std::cout << "Failed to create Vulkan instance" << std::endl;
        return -1;
    }

    return 0;
}

int VulkanApplication::Cleanup()
{
    if (m_instance)
    {
        vkDestroyInstance(m_instance, nullptr);
    }

    if (m_window)
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    m_window = nullptr;
    m_initialized = false;

    return 0;
}