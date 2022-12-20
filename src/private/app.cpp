#include <app.h>

#include <array>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <vector>

#include <utils/utils.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using VulkanRenderer::Application;

namespace Cst
{
constexpr std::array<const char*, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif
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

    int res = CreateInstance();
    if (res != 0)
        return res;

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

int Application::Cleanup()
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