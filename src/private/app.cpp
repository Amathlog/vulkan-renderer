#include <app.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

VulkanApplication::VulkanApplication(int width, int height)
    : m_width(width)
    , m_height(height)
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

    return 0;
}

int VulkanApplication::InitWindow() { return 0; }

int VulkanApplication::InitVulkan() { return 0; }

int VulkanApplication::Cleanup()
{
    if (!m_initialized)
        return 0;

    return 0;
}