#include <graphicPipeline.h>
#include <shader.h>

#include <array>
#include <cstdint>
#include <iostream>

using VulkanRenderer::GraphicPipeline;
using VulkanRenderer::GraphicPipelineConfig;
using VulkanRenderer::Shader;
using VulkanRenderer::ShaderType;

GraphicPipeline::GraphicPipeline(GraphicPipelineConfig& config)
    : m_deviceCache(config.device)
{
    // Step 1: Shaders
    m_vertShader = Shader::CreateFromFile(m_deviceCache, ShaderType::Vertex, config.vertShaderFile);
    if (!m_vertShader)
    {
        std::cout << "Failed to create vertex shader" << std::endl;
        return;
    }

    m_fragShader = Shader::CreateFromFile(m_deviceCache, ShaderType::Fragment, config.fragShaderFile);
    if (!m_fragShader)
    {
        std::cout << "Failed to create fragment shader" << std::endl;
        return;
    }

    std::array<VkPipelineShaderStageCreateInfo, 2> shaderStageInfos;
    for (auto i = 0; i < 2; ++i)
    {
        shaderStageInfos[i].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStageInfos[i].pName = config.pipelineName;
    }

    // Vertex (first index)
    shaderStageInfos[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStageInfos[0].module = m_vertShader->GetModule();
    // Fragment (second index)
    shaderStageInfos[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStageInfos[1].module = m_fragShader->GetModule();

    // Step 2: Dynamic state
    std::array<VkDynamicState, 2> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};

    VkPipelineDynamicStateCreateInfo dynamicState{};
    dynamicState.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
    dynamicState.dynamicStateCount = static_cast<uint32_t>(dynamicStates.size());
    dynamicState.pDynamicStates = dynamicStates.data();

    // Step 3: Vertex input
    // Note: As vertex data is hardcoded in the shader (for now), there is not a lot to do.
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 0;
    vertexInputInfo.pVertexBindingDescriptions = nullptr; // Optional
    vertexInputInfo.vertexAttributeDescriptionCount = 0;
    vertexInputInfo.pVertexAttributeDescriptions = nullptr; // Optional

    // Step 4: Input assembly
    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    inputAssembly.primitiveRestartEnable = VK_FALSE;

    // Step 5: Viewport and scissors
    // Note that since we went with a dynamic state, viewport and scissors will be given
    // when we are drawing.
    m_viewport.x = 0.0f;
    m_viewport.y = 0.0f;
    m_viewport.width = (float)config.viewportWidth;
    m_viewport.height = (float)config.viewportHeight;
    m_viewport.minDepth = 0.0f;
    m_viewport.maxDepth = 1.0f;

    m_scissors.offset = {0, 0};
    m_scissors.extent = VkExtent2D{config.viewportWidth, config.viewportHeight};

    VkPipelineViewportStateCreateInfo viewportState{};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.scissorCount = 1;

    // Step 6: Rasterizer
    VkPipelineRasterizationStateCreateInfo rasterizer{};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_BACK_BIT;
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f;          // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f;    // Optional

    // Step 7: Multisampling
    // For now, it is disabled
    VkPipelineMultisampleStateCreateInfo multisampling{};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f;          // Optional
    multisampling.pSampleMask = nullptr;            // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE;      // Optional

    // Step 8: Depth and stencil testing
    // Not used for now
    VkPipelineDepthStencilStateCreateInfo depthStencilState{};

    // Step 9: Color blending
    VkPipelineColorBlendAttachmentState colorBlendAttachment{};
    colorBlendAttachment.colorWriteMask =
        VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_FALSE;

    VkPipelineColorBlendStateCreateInfo colorBlending{};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;

    // Step 10: Viewport
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;            // Optional
    pipelineLayoutInfo.pSetLayouts = nullptr;         // Optional
    pipelineLayoutInfo.pushConstantRangeCount = 0;    // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(m_deviceCache, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        std::cout << "Failed to create pipeline layout!" << std::endl;
        m_pipelineLayout = VK_NULL_HANDLE;
        return;
    }
}

GraphicPipeline::~GraphicPipeline()
{
    vkDestroyPipeline(m_deviceCache, m_pipeline, nullptr);
    vkDestroyPipelineLayout(m_deviceCache, m_pipelineLayout, nullptr);
}

bool GraphicPipeline::IsValid() const
{
    return m_vertShader && m_fragShader && m_pipelineLayout != VK_NULL_HANDLE && m_pipeline != VK_NULL_HANDLE;
}