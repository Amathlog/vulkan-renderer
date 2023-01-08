#pragma once

#include <memory>

struct VkDevice_T;

namespace VulkanRenderer
{
class Shader;

class GraphicPipeline
{
public:
    GraphicPipeline(VkDevice_T* device, const char* vertShaderFile, const char* fragShaderFile,
                    const char* pipelineName);
    ~GraphicPipeline();

    bool IsValid() const;

private:
    VkDevice_T* m_deviceCache;
    std::unique_ptr<Shader> m_vertShader;
    std::unique_ptr<Shader> m_fragShader;
};
} // namespace VulkanRenderer