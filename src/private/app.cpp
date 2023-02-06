#include <app.h>

#include <array>
#include <cstring>
#include <iostream>
#include <map>
#include <tuple>
#include <vector>

#include <config.h>
#include <graphicPipeline.h>
#include <swapChain.h>
#include <utils/queueFamily.h>
#include <utils/utils.h>
#include <utils/verboseDump.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

using VulkanRenderer::Application;

namespace Cst
{
constexpr std::array<const char*, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};
constexpr std::array<const char*, 1> deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

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

    int returnCode = 0;

    while (!glfwWindowShouldClose(m_window) || returnCode != 0)
    {
        glfwPollEvents();
        returnCode = DrawFrame();
    }

    vkDeviceWaitIdle(m_device);

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
        &Application::CreateLogicalDevice,
        &Application::CreateSwapChain,
        &Application::CreateGraphicPipeline,
        &Application::CreateFramebuffers,
        &Application::CreateCommandPool,
        &Application::CreateCommandBuffer,
        &Application::CreateSyncObjects
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

        int validationLayersValidation = VulkanRenderer::Utils::ValidateStrings(
            Cst::validationLayers, validationLayers,
            [](const VkLayerProperties& layerProperty) -> const char* { return layerProperty.layerName; });

        if (validationLayersValidation != static_cast<int>(Cst::validationLayers.size()))
        {
            std::cout << "Validation layer " << Cst::validationLayers[validationLayersValidation] << " not supported..."
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
    QueueFamilyIndices indices = VulkanRenderer::FindQueueFamilies(m_physicalDevice, m_surface);

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

    VkDeviceCreateInfo createDeviceInfo{};
    createDeviceInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createDeviceInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createDeviceInfo.pQueueCreateInfos = queueCreateInfos.data();
    createDeviceInfo.pEnabledFeatures = &deviceFeatures;
    createDeviceInfo.enabledExtensionCount = static_cast<uint32_t>(Cst::deviceExtensions.size());
    createDeviceInfo.ppEnabledExtensionNames = Cst::deviceExtensions.data();

    if (vkCreateDevice(m_physicalDevice, &createDeviceInfo, nullptr, &m_device) != VK_SUCCESS)
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

int Application::CreateSwapChain()
{
    m_swapChain = std::make_unique<VulkanRenderer::SwapChain>(m_device, m_physicalDevice, m_surface, m_window);
    return m_swapChain->IsValid() ? 0 : -1;
}

int Application::CreateGraphicPipeline()
{
    if (!m_swapChain)
    {
        return -1;
    }

    VulkanRenderer::GraphicPipelineConfig config;
    config.device = m_device;
    config.pipelineName = "main";
    config.viewportHeight = m_height;
    config.viewportWidth = m_width;
    config.fragShaderFile = "shaders/simple.frag.spv";
    config.vertShaderFile = "shaders/simple.vert.spv";
    config.swapChainFormat = m_swapChain->GetFormat();

    m_graphicPipeline = std::make_unique<VulkanRenderer::GraphicPipeline>(config);
    return m_graphicPipeline->IsValid() ? 0 : -1;
}

int Application::CreateFramebuffers()
{
    if (!m_swapChain || !m_graphicPipeline)
    {
        return -1;
    }

    std::vector<VkImageView>& imageViews = m_swapChain->GetImageViews();

    m_framebuffers.resize(imageViews.size());

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = m_graphicPipeline->GetRenderPass();
    framebufferInfo.attachmentCount = 1;
    framebufferInfo.width = m_swapChain->GetExtent().width;
    framebufferInfo.height = m_swapChain->GetExtent().height;
    framebufferInfo.layers = 1;

    for (size_t i = 0; i < m_framebuffers.size(); ++i)
    {
        framebufferInfo.pAttachments = &imageViews[i];

        if (vkCreateFramebuffer(m_device, &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
        {
            std::cout << "Failed to create framebuffers" << std::endl;
            return -1;
        }
    }

    return 0;
}

int Application::CreateCommandPool()
{
    QueueFamilyIndices queueFamillyIndices = FindQueueFamilies(m_physicalDevice, m_surface);

    VkCommandPoolCreateInfo commandPoolInfo{};
    commandPoolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    commandPoolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    commandPoolInfo.queueFamilyIndex = queueFamillyIndices.graphicsFamily.value();

    if (vkCreateCommandPool(m_device, &commandPoolInfo, nullptr, &m_commandPool) != VK_SUCCESS)
    {
        std::cout << "Failed to create command pool" << std::endl;
        m_commandPool = VK_NULL_HANDLE;
        return -1;
    }

    return 0;
}

int Application::CreateCommandBuffer()
{
    if (!m_commandPool)
    {
        return -1;
    }

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_commandPool;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

    if (vkAllocateCommandBuffers(m_device, &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        std::cout << "Failed to allocate command buffer." << std::endl;
        return -1;
    }

    return 0;
}

int Application::RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                  // Optional
    beginInfo.pInheritanceInfo = nullptr; // Only relevant for secondary command buffers

    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS)
    {
        std::cout << "Failed to begin command buffer" << std::endl;
        return -1;
    }

    // Then we begin a render pass
    VkRenderPassBeginInfo renderBeginInfo{};
    renderBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderBeginInfo.renderPass = m_graphicPipeline->GetRenderPass();
    renderBeginInfo.framebuffer = m_framebuffers[imageIndex];
    renderBeginInfo.renderArea.offset = {0, 0};
    renderBeginInfo.renderArea.extent = m_swapChain->GetExtent();

    // Turquoise: #40e0d0, with alpha 0.7
    static constexpr VkClearValue clearColor = {{{64.f / 255.f, 224.f / 255.f, 208.f / 255.f, 0.7f}}};

    renderBeginInfo.clearValueCount = 1;
    renderBeginInfo.pClearValues = &clearColor;

    // Start render pass!
    vkCmdBeginRenderPass(commandBuffer, &renderBeginInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicPipeline->GetPipeline());

    // Viewport and scissors were marked dynamic, so set them here
    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_swapChain->GetExtent().width);
    viewport.height = static_cast<float>(m_swapChain->GetExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);

    VkRect2D scissor{};
    scissor.offset = {0, 0};
    scissor.extent = m_swapChain->GetExtent();
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);

    // Let's draw our triangle!
    vkCmdDraw(commandBuffer, 3, 1, 0, 0);

    // And finish the render pass
    vkCmdEndRenderPass(commandBuffer);

    // We can also end the command buffer
    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        std::cout << "Failed to end command buffer" << std::endl;
        return -1;
    }

    return 0;
}

int Application::CreateSyncObjects()
{
    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    // Create the fence signaled to avoid blocking on the first frame!
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (int i = 0; i < maxFramesInFlight; ++i)
    {
        if (vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) != VK_SUCCESS ||
            vkCreateSemaphore(m_device, &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) != VK_SUCCESS ||
            vkCreateFence(m_device, &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS)
        {
            std::cout << "Failed to initialize sync objects" << std::endl;
            return -1;
        }
    }

    return 0;
}

int Application::Cleanup()
{
    for (int i = 0; i < maxFramesInFlight; ++i)
    {
        if (m_imageAvailableSemaphores[i])
            vkDestroySemaphore(m_device, m_imageAvailableSemaphores[i], nullptr);

        if (m_renderFinishedSemaphores[i])
            vkDestroySemaphore(m_device, m_renderFinishedSemaphores[i], nullptr);

        if (m_inFlightFences[i])
            vkDestroyFence(m_device, m_inFlightFences[i], nullptr);
    }

    if (m_commandPool)
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);

    for (VkFramebuffer& framebuffer : m_framebuffers)
    {
        if (framebuffer != VK_NULL_HANDLE)
            vkDestroyFramebuffer(m_device, framebuffer, nullptr);
    }

    m_framebuffers.clear();

    // We need to delete the swap chain and graphic pipeline before deleting the device.
    m_graphicPipeline.reset();
    m_swapChain.reset();

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

bool Application::IsSuitableDevice(VkPhysicalDevice device) const
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
    QueueFamilyIndices indices = VulkanRenderer::FindQueueFamilies(device, m_surface);

    // If we didn't found all our requested queue family, early out
    if (!indices.IsComplete())
        return false;

    // If the current device doesn't supported wanted extensions, early out
    if (!CheckDeviceExtensionSupport(device))
        return false;

    // If the current device doesn't support all our swap chain requirements, early out
    VulkanRenderer::SwapChainSupportDetails details{};
    VulkanRenderer::SwapChain::FillSwapChainSupportDetails(device, m_surface, details);

    if (details.formats.empty() || details.presentModes.empty())
        return false;

    return true;
}

bool Application::CheckDeviceExtensionSupport(VkPhysicalDevice device) const
{
    uint32_t extensionsCount = 0;
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, nullptr);

    std::vector<VkExtensionProperties> availableExtensions(extensionsCount);
    vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionsCount, availableExtensions.data());

    int nbFoundExtensions = VulkanRenderer::Utils::ValidateStrings(
        Cst::deviceExtensions, availableExtensions,
        [](const VkExtensionProperties& properties) -> const char* { return properties.extensionName; });

    return nbFoundExtensions == static_cast<int>(Cst::deviceExtensions.size());
}

int Application::DrawFrame()
{
    // High level of a frame:
    // * Wait for the previous frame to finish
    // * Acquire an image from the swap chain
    // * Record a command buffer which draws the scene onto that image
    // * Submit the recorded command buffer
    // * Present the swap chain image

    // Since all command as asynchronous with the GPU, we'll have to add synchronisation
    // primitives, such as Semaphores or Fences

    // At the start of our frame, we wait until the previous frame has rendered
    vkWaitForFences(m_device, 1, &m_inFlightFences[m_currentFrame], VK_TRUE, UINT64_MAX);

    vkResetFences(m_device, 1, &m_inFlightFences[m_currentFrame]);

    // Acquire an image from the swap chain, will signal the semaphore when it's done.
    // We use no fences here.
    uint32_t imageIndex = 0;
    vkAcquireNextImageKHR(m_device, m_swapChain->GetSwapChain(), UINT64_MAX, m_imageAvailableSemaphores[m_currentFrame],
                          VK_NULL_HANDLE, &imageIndex);

    vkResetCommandBuffer(m_commandBuffers[m_currentFrame], 0);

    RecordCommandBuffer(m_commandBuffers[m_currentFrame], imageIndex);

    // When the command is recorded, submit it to the queue
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    submitInfo.pWaitDstStageMask = waitStages;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &m_commandBuffers[m_currentFrame];

    // Do it!
    if (vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, m_inFlightFences[m_currentFrame]) != VK_SUCCESS)
    {
        std::cout << "Failed to submit to the graphics queue..." << std::endl;
        return -1;
    }

    // When all is submitted, we need to present the image to the screen
    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain->GetSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &imageIndex;

    if (vkQueuePresentKHR(m_graphicsQueue, &presentInfo) != VK_SUCCESS)
    {
        std::cout << "Failed to present..." << std::endl;
        return -1;
    }

    if (++m_currentFrame >= maxFramesInFlight)
        m_currentFrame = 0;

    return 0;
}