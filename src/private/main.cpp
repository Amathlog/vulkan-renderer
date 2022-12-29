#include <app.h>
#include <config.h>

#include <iostream>

int main(int argc, char* argv[])
{
    VulkanRenderer::VulkanParameters::Initialize(argc, argv);

    if (VulkanRenderer::Parameters().verbose())
    {
        std::cout << "Verbose mode" << std::endl;
    }

    VulkanRenderer::Application app(800, 600, "VulkanApplication");

    if (app.Init() != 0)
    {
        std::cout << "Error while initializing Vulkan" << std::endl;
        return -1;
    }

    app.Run();

    return 0;
}