#include <graphicPipeline.h>
#include <shader.h>

#include <vulkan/vulkan.h>

#include <array>
#include <iostream>

using VulkanRenderer::GraphicPipeline;
using VulkanRenderer::Shader;
using VulkanRenderer::ShaderType;

GraphicPipeline::GraphicPipeline(VkDevice device, const char* vertShaderFile, const char* fragShaderFile,
                                 const char* pipelineName)
    : m_deviceCache(device)
{

    // First create shaders
    m_vertShader = Shader::CreateFromFile(device, ShaderType::Vertex, vertShaderFile);
    if (!m_vertShader)
    {
        std::cout << "Failed to create vertex shader" << std::endl;
        return;
    }

    m_fragShader = Shader::CreateFromFile(device, ShaderType::Fragment, fragShaderFile);
    if (!m_fragShader)
    {
        std::cout << "Failed to create fragment shader" << std::endl;
        return;
    }

    // Create shader stage info
    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageInfos;
    for (auto i = 0; i < 2; ++i)
    {
        shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos[i].pName = pipelineName;
    }

    // Vertex (first index)
    shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfos[0].module = m_vertShader->GetModule();
    // Fragment (second index)
    shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageInfos[1].module = m_fragShader->GetModule();
}

GraphicPipeline::~GraphicPipeline() {}

bool GraphicPipeline::IsValid() const { return m_vertShader && m_fragShader; }