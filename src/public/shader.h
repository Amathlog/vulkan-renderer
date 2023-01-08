#pragma once

#include <cstdint>
#include <memory>
#include <vector>

struct VkDevice_T;
struct VkShaderModule_T;

namespace VulkanRenderer
{

enum class ShaderType : uint8_t
{
    Vertex = 0,
    Fragment = 1,

    Count = 255
};

class Shader
{
public:
    Shader(VkDevice_T* device, ShaderType type, const std::vector<char>& byteCode);
    ~Shader();

    bool IsValid() const;
    VkShaderModule_T* GetModule() { return m_module; }
    const VkShaderModule_T* GetConstModule() const { return m_module; }
    ShaderType GetType() const { return m_type; }

    static std::unique_ptr<Shader> CreateFromFile(VkDevice_T* device, ShaderType type, const char* filePath);

private:
    VkDevice_T* m_deviceCache;
    VkShaderModule_T* m_module;
    ShaderType m_type;
};
} // namespace VulkanRenderer