#pragma once

class VulkanApplication
{
public:
    VulkanApplication(int width, int height);
    ~VulkanApplication();

    int Init();
    int Run();

private:
    int InitWindow();
    int InitVulkan();
    int Cleanup();

    bool m_initialized = false;
    int m_width;
    int m_height;
};