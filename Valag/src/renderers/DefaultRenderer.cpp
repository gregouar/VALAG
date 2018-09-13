#include "Valag/renderers/DefaultRenderer.h"

#include <sstream>

#include "Valag/utils/Profiler.h"
#include "Valag/utils/Logger.h"
#include "Valag/core/Config.h"
#include "Valag/core/VApp.h"

#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"
#include "Valag/gfx/Sprite.h"

#include "Valag/vulkanImpl/VulkanHelpers.h"


namespace vlg
{

const char *DefaultRenderer::DEFAULT_VERTSHADERFILE = "defaultShader.vert.spv";
const char *DefaultRenderer::DEFAULT_FRAGSHADERFILE = "defaultShader.frag.spv";

const float DefaultRenderer::DEPTH_SCALING_FACTOR = 1024*1024;

DefaultRenderer::DefaultRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) : AbstractRenderer(targetWindow, name, order)
{
    this->init();
}

DefaultRenderer::~DefaultRenderer()
{
    this->cleanup();
}

void DefaultRenderer::update(size_t frameIndex)
{
    Sprite::updateRendering(frameIndex);
    AbstractRenderer::update(frameIndex);
}


void DefaultRenderer::draw(Drawable *drawable)
{
    VkCommandBuffer commandBuffer = drawable->getDrawCommandBuffer(this,m_curFrameIndex, m_renderPass, 0);
    if(commandBuffer != VK_NULL_HANDLE)
        m_activeSecondaryCommandBuffers.push_back(commandBuffer);
}

bool DefaultRenderer::recordPrimaryCommandBuffer(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;  //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    if (vkBeginCommandBuffer(m_primaryCMB[m_curFrameIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (false);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_targetWindow->getSwapchainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
    clearValues[1].depthStencil = {0.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_primaryCMB[m_curFrameIndex], &renderPassInfo, /*VK_SUBPASS_CONTENTS_INLINE*/ VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        if(!m_activeSecondaryCommandBuffers.empty())
            vkCmdExecuteCommands(m_primaryCMB[m_curFrameIndex], (uint32_t) m_activeSecondaryCommandBuffers.size(), m_activeSecondaryCommandBuffers.data());

    vkCmdEndRenderPass(m_primaryCMB[m_curFrameIndex]);

    m_activeSecondaryCommandBuffers.clear();

    if (vkEndCommandBuffer(m_primaryCMB[m_curFrameIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}

bool DefaultRenderer::bindTexture(VkCommandBuffer &commandBuffer, AssetTypeID textureID, size_t frameIndex)
{
    TextureAsset *texAsset = TexturesHandler::instance()->getAsset(textureID);

    int pc[] = {0,0};

    if(texAsset != nullptr && texAsset->isLoaded())
    {
        VTexture vtexture = texAsset->getVTexture();
        pc[0] = vtexture.m_textureId;
        pc[1] = vtexture.m_textureLayer;
    }

    vkCmdPushConstants(commandBuffer, m_pipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int)*2, (void*)pc);

    return (true);
}

void DefaultRenderer::bindPipeline(VkCommandBuffer &commandBuffer, size_t frameIndex)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

    VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(frameIndex)/*m_viewDescriptorSets[frameIndex]*/,
                                        VTexturesManager::instance()->getDescriptorSet(frameIndex) };

    vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout,0,2, descriptorSets, 0, nullptr);
}

void DefaultRenderer::bindModelDescriptorSet(size_t frameIndex, VkCommandBuffer &commandBuffer, DynamicUBODescriptor &uboDesc, size_t index)
{
    uint32_t dynamicOffset  = uboDesc.getDynamicOffset(frameIndex, index);
    auto descSet            = uboDesc.getDescriptorSet(frameIndex);
    vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_pipelineLayout,2,1, &descSet, 1, &dynamicOffset);
}

bool DefaultRenderer::init()
{
    Sprite::initRendering(m_targetWindow->getFramesCount());

    m_renderView.setExtent({m_targetWindow->getSwapchainExtent().width,
                            m_targetWindow->getSwapchainExtent().height});
    m_renderView.setDepthFactor(DEPTH_SCALING_FACTOR);

    return AbstractRenderer::init();
}

bool DefaultRenderer::createDescriptorSetLayouts()
{
    return (true);
}

bool DefaultRenderer::createGraphicsPipeline()
{
    VkDevice device = VInstance::device();

    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << DEFAULT_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << DEFAULT_FRAGSHADERFILE;

    auto vertShaderCode = VulkanHelpers::readFile(vertShaderPath.str());
    auto fragShaderCode = VulkanHelpers::readFile(fragShaderPath.str());

    VkShaderModule vertShaderModule = VulkanHelpers::createShaderModule(vertShaderCode);
    VkShaderModule fragShaderModule = VulkanHelpers::createShaderModule(fragShaderCode);

    VkPipelineShaderStageCreateInfo vertShaderStageInfo = {};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo fragShaderStageInfo = {};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";

    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    VkPipelineVertexInputStateCreateInfo vertexInputInfo = {};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;

    //auto bindingDescription = Vertex2D::getBindingDescription();
    //auto attributeDescriptions = Vertex2D::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 0;
    /*vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();*/


    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
    inputAssembly.primitiveRestartEnable = VK_TRUE;


    VkViewport viewport = {};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = (float) m_targetWindow->getSwapchainExtent().width;
    viewport.height = (float) m_targetWindow->getSwapchainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_targetWindow->getSwapchainExtent();

    VkPipelineViewportStateCreateInfo viewportState = {};
    viewportState.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportState.viewportCount = 1;
    viewportState.pViewports = &viewport;
    viewportState.scissorCount = 1;
    viewportState.pScissors = &scissor;


    VkPipelineRasterizationStateCreateInfo rasterizer = {};
    rasterizer.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizer.depthClampEnable = VK_FALSE;
    rasterizer.rasterizerDiscardEnable = VK_FALSE;
    rasterizer.polygonMode = VK_POLYGON_MODE_FILL;
    rasterizer.lineWidth = 1.0f;
    rasterizer.cullMode = VK_CULL_MODE_NONE; //VK_CULL_MODE_BACK_BIT
    rasterizer.frontFace = VK_FRONT_FACE_CLOCKWISE;
    rasterizer.depthBiasEnable = VK_FALSE;
    rasterizer.depthBiasConstantFactor = 0.0f; // Optional
    rasterizer.depthBiasClamp = 0.0f; // Optional
    rasterizer.depthBiasSlopeFactor = 0.0f; // Optional

    VkPipelineMultisampleStateCreateInfo multisampling = {};
    multisampling.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampling.sampleShadingEnable = VK_FALSE;
    multisampling.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    multisampling.minSampleShading = 1.0f; // Optional
    multisampling.pSampleMask = nullptr; // Optional
    multisampling.alphaToCoverageEnable = VK_FALSE; // Optional
    multisampling.alphaToOneEnable = VK_FALSE; // Optional

    VkPipelineColorBlendAttachmentState colorBlendAttachment = {};
    colorBlendAttachment.colorWriteMask
        = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment.blendEnable = VK_TRUE;
    colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

    VkPipelineColorBlendStateCreateInfo colorBlending = {};
    colorBlending.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlending.logicOpEnable = VK_FALSE;
    colorBlending.logicOp = VK_LOGIC_OP_COPY; // Optional
    colorBlending.attachmentCount = 1;
    colorBlending.pAttachments = &colorBlendAttachment;
    colorBlending.blendConstants[0] = 0.0f; // Optional
    colorBlending.blendConstants[1] = 0.0f; // Optional
    colorBlending.blendConstants[2] = 0.0f; // Optional
    colorBlending.blendConstants[3] = 0.0f; // Optional

    VkPipelineLayoutCreateInfo pipelineLayoutInfo = {};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    //pipelineLayoutInfo.setLayoutCount = 0;

    VkDescriptorSetLayout descriptorSetLayouts[] = {m_renderView.getDescriptorSetLayout()/*m_viewDescriptorSetLayout*/,
                                                    VTexturesManager::instance()->getDescriptorSetLayout(),
                                                    Sprite::getModelDescriptorSetLayout()
                                                    /*m_modelDescriptorSetLayout*/};
    pipelineLayoutInfo.setLayoutCount = 3;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;


    VkPushConstantRange pushConstantRange = {};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(int)*2;
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    //pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    //pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        Logger::error("Failed to create pipeline layout");
        return (false);
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER; //VK_COMPARE_OP_GREATER;
    depthStencil.depthBoundsTestEnable = VK_FALSE;
    depthStencil.minDepthBounds = 0.0f; // Optional
    depthStencil.maxDepthBounds = 1.0f; // Optional
    depthStencil.stencilTestEnable = VK_FALSE;
    depthStencil.front = {}; // Optional
    depthStencil.back = {}; // Optional

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    //pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = m_pipelineLayout;
    pipelineInfo.renderPass = m_renderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_pipeline) != VK_SUCCESS)
        return (false);

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    return (true);
}

bool DefaultRenderer::createDescriptorPool()
{
    return (true);
}

bool DefaultRenderer::createDescriptorSets()
{
    return (true);
}

bool DefaultRenderer::createUBO()
{
    return (true);
}

void DefaultRenderer::cleanup()
{
    AbstractRenderer::cleanup();
    Sprite::cleanupRendering();
}



}
