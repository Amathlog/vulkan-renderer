#pragma once

#include <vulkan/vulkan.h>

#include <memory>

namespace VulkanRenderer
{
class Shader;

struct GraphicPipelineConfig
{
    VkDevice device;
    uint32_t viewportWidth;
    uint32_t viewportHeight;
    const char* vertShaderFile;
    const char* fragShaderFile;
    const char* pipelineName;
};

class GraphicPipeline
{
public:
    GraphicPipeline(GraphicPipelineConfig& config);
    ~GraphicPipeline();

    VkViewport& GetViewport() { return m_viewport; }
    VkRect2D& GetScissors() { return m_scissors; }

    bool IsValid() const;

private:
    VkDevice m_deviceCache;
    std::unique_ptr<Shader> m_vertShader;
    std::unique_ptr<Shader> m_fragShader;

    VkViewport m_viewport;
    VkRect2D m_scissors;

    VkPipelineLayout m_pipelineLayout;
    VkPipeline m_pipeline;
};
} // namespace VulkanRenderer