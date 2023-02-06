#include <shader.h>

#include <config.h>

#include <vulkan/vulkan.h>

#include <filesystem>
#include <fstream>
#include <iostream>

using VulkanRenderer::Shader;

Shader::Shader(VkDevice_T* device, ShaderType type, const std::vector<char>& byteCode)
    : m_deviceCache(device)
    , m_module(VK_NULL_HANDLE)
    , m_type(type)
{
    VkShaderModuleCreateInfo shaderCreateInfo{};
    shaderCreateInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    shaderCreateInfo.codeSize = byteCode.size();
    shaderCreateInfo.pCode = reinterpret_cast<const uint32_t*>(byteCode.data());

    if (vkCreateShaderModule(m_deviceCache, &shaderCreateInfo, nullptr, &m_module) != VK_SUCCESS)
    {
        std::cout << "Failed to create the shader module!" << std::endl;
        m_module = VK_NULL_HANDLE;
    }
}

Shader::~Shader() { vkDestroyShaderModule(m_deviceCache, m_module, nullptr); }

bool Shader::IsValid() const { return m_module != VK_NULL_HANDLE; }

std::unique_ptr<Shader> Shader::CreateFromFile(VkDevice_T* device, ShaderType type, const char* filePath)
{
    if (!std::filesystem::exists(filePath))
    {
        // Try to look next to the executable also
        std::filesystem::path execPath =
            std::filesystem::path(VulkanRenderer::VulkanParameters::GetInstance().execPath).parent_path();

        execPath /= filePath;

        if (!std::filesystem::exists(execPath))
        {
            std::cout << "File " << filePath << " was not found." << std::endl;
            return nullptr;
        }
    }

    std::ifstream fileStream;
    fileStream.open(filePath, std::ios::binary | std::ios::ate);

    if (!fileStream.is_open())
    {
        std::cout << "Failed to open file " << filePath << std::endl;
        return nullptr;
    }

    size_t fileSize = (size_t)fileStream.tellg();
    std::vector<char> byteCode(fileSize);
    fileStream.seekg(0);
    fileStream.read(byteCode.data(), fileSize);

    fileStream.close();

    std::unique_ptr<Shader> res = std::make_unique<Shader>(device, type, byteCode);

    if (!res->IsValid())
    {
        std::cout << "Error while create shader module from file " << filePath << std::endl;
        return nullptr;
    }

    return res;
}