#include <app.h>

#include <iostream>

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

int VulkanApplication::InitVulkan() { return 0; }

int VulkanApplication::Cleanup()
{
    if (m_window)
    {
        glfwDestroyWindow(m_window);
        glfwTerminate();
    }

    m_window = nullptr;
    m_initialized = false;

    return 0;
}