#include <app.h>

#include <iostream>

int main()
{
    VulkanRenderer::Application app(800, 600, "VulkanApplication");

    if (app.Init() != 0)
    {
        std::cout << "Error while initializing Vulkan" << std::endl;
        return -1;
    }

    app.Run();

    return 0;
}