#include "Valag/renderers/InstancingRenderer.h"

#include "Valag/vulkanImpl/vulkanImpl.h"
#include "Valag/utils/Logger.h"
#include "Valag/core/VApp.h"


#include "Valag/core/AssetHandler.h"
#include "Valag/gfx/TextureAsset.h"
#include "Valag/gfx/Sprite.h"
#include "Valag/gfx/SpritesBatch.h"

namespace vlg
{

const char *InstancingRenderer::INSTANCING_VERTSHADERFILE = "instancingShader.vert.spv";
const char *InstancingRenderer::INSTANCING_FRAGSHADERFILE = "instancingShader.frag.spv";

const size_t InstancingRenderer::VBO_CHUNK_SIZE = 1024;

const float InstancingRenderer::DEPTH_SCALING_FACTOR = 1024*1024;


VkVertexInputBindingDescription InstanciedSpriteDatum::getBindingDescription()
{
    VkVertexInputBindingDescription bindingDescription = {};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(InstanciedSpriteDatum);
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;

    return bindingDescription;
}

std::array<VkVertexInputAttributeDescription, 8> InstanciedSpriteDatum::getAttributeDescriptions()
{
    std::array<VkVertexInputAttributeDescription, 8> attributeDescriptions = {};

    size_t i = 0;
    /*attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, pos_ul);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, pos_ur);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, pos_dl);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, layer);
    ++i;*/

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, model_0);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, model_1);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, model_2);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, model_3);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, color);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, texPos);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, texExtent);
    ++i;

    attributeDescriptions[i].binding = 0;
    attributeDescriptions[i].location = i;
    attributeDescriptions[i].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[i].offset = offsetof(InstanciedSpriteDatum, texId);
    ++i;


    return attributeDescriptions;
}

InstancingRenderer::InstancingRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) : AbstractRenderer(targetWindow, name,order)
{
    this->init();
}

InstancingRenderer::~InstancingRenderer()
{
    this->cleanup();
}

void InstancingRenderer::update(size_t frameIndex)
{
    AbstractRenderer::update(frameIndex);
}

void InstancingRenderer::updateVBO(size_t frameIndex)
{
    if(m_spritesRenderQueueSize != 0)
        VBuffersAllocator::writeBuffer(m_vbos[frameIndex],m_spritesRenderQueue[frameIndex].data(),
                                       m_spritesRenderQueueSize*sizeof(InstanciedSpriteDatum),false);
}


void InstancingRenderer::draw(Sprite* sprite)
{
    if(m_spritesRenderQueueSize == m_spritesRenderQueue[m_curFrameIndex].size())
        this->expandVBO(m_curFrameIndex);

    SpriteModelUBO modelUBO = sprite->getModelUBO();

    TextureAsset *texAsset = TexturesHandler::instance()->getAsset(sprite->getTexture());

    if(texAsset != nullptr && texAsset->isLoaded())
    {
        VTexture vtexture = texAsset->getVTexture();
       // pc[0] = vtexture.m_textureId;
       // pc[1] = vtexture.m_textureLayer;

       InstanciedSpriteDatum datum = {};
       datum.model_0 = modelUBO.model[0];
       datum.model_1 = modelUBO.model[1];
       datum.model_2 = modelUBO.model[2];
       datum.model_3 = modelUBO.model[3];
       datum.color   = modelUBO.color;
       datum.texExtent = modelUBO.texExt;
       datum.texPos = modelUBO.texPos;
       datum.texId = {vtexture.m_textureId, vtexture.m_textureLayer};

       m_spritesRenderQueue[m_curFrameIndex][m_spritesRenderQueueSize++] = datum;
    }
}


void InstancingRenderer::draw(SpritesBatch* spritesBatch)
{
    spritesBatch->draw(this);
}

bool InstancingRenderer::recordPrimaryCommandBuffer(uint32_t imageIndex)
{
    this->updateVBO(m_curFrameIndex);

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;

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

    if(m_order == Renderer_First || m_order == Renderer_Unique)
    {
        std::array<VkClearValue, 2> clearValues = {};
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {0.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
    }

    vkCmdBeginRenderPass(m_primaryCMB[m_curFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE /*VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS*/);

    if(m_spritesRenderQueueSize != 0)
    {
        vkCmdBindPipeline(m_primaryCMB[m_curFrameIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_pipeline);

        VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                            VTexturesManager::instance()->getDescriptorSet(m_curFrameIndex) };

        vkCmdBindDescriptorSets(m_primaryCMB[m_curFrameIndex],VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_pipelineLayout,0,2, descriptorSets, 0, nullptr);

        vkCmdBindVertexBuffers(m_primaryCMB[m_curFrameIndex], 0, 1, &m_vbos[m_curFrameIndex].buffer, &m_vbos[m_curFrameIndex].offset);

        vkCmdDraw(m_primaryCMB[m_curFrameIndex], 4, m_spritesRenderQueueSize, 0, 0);
        m_spritesRenderQueueSize = 0;
    }

    vkCmdEndRenderPass(m_primaryCMB[m_curFrameIndex]);

    if (vkEndCommandBuffer(m_primaryCMB[m_curFrameIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}

bool InstancingRenderer::init()
{
    m_renderView.setExtent({m_targetWindow->getSwapchainExtent().width,
                            m_targetWindow->getSwapchainExtent().height});
    m_renderView.setDepthFactor(DEPTH_SCALING_FACTOR);

    m_spritesRenderQueueSize = 0;
    //m_maxSpriteRenderQueueSize = VBO_CHUNK_SIZE;
    m_spritesRenderQueue.resize(m_targetWindow->getFramesCount());
    m_vbos.resize(m_targetWindow->getFramesCount());

    for(size_t i = 0 ; i < m_vbos.size() ; ++i)
        if(!this->expandVBO(i))
            return (false);

    return AbstractRenderer::init();
}

bool InstancingRenderer::createDescriptorSetLayouts()
{
    return (true);
}

bool InstancingRenderer::createGraphicsPipeline()
{
    VkDevice device = VInstance::device();

    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << INSTANCING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << INSTANCING_FRAGSHADERFILE;

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
    auto bindingDescription = InstanciedSpriteDatum::getBindingDescription();
    auto attributeDescriptions = InstanciedSpriteDatum::getAttributeDescriptions();
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


    VkPipelineInputAssemblyStateCreateInfo inputAssembly = {};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;//VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
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

    VkDescriptorSetLayout descriptorSetLayouts[] = {m_renderView.getDescriptorSetLayout(),
                                                    VTexturesManager::instance()->getDescriptorSetLayout()};
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;

    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_pipelineLayout) != VK_SUCCESS)
    {
        Logger::error("Failed to create pipeline layout");
        return (false);
    }

    VkPipelineDepthStencilStateCreateInfo depthStencil = {};
    depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencil.depthTestEnable = VK_TRUE;
    depthStencil.depthWriteEnable = VK_TRUE;
    depthStencil.depthCompareOp = VK_COMPARE_OP_GREATER;//VK_COMPARE_OP_ALWAYS; ///VK_COMPARE_OP_GREATER;
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

bool InstancingRenderer::createDescriptorPool()
{
    return (true);
}

bool InstancingRenderer::createDescriptorSets()
{
    return (true);
}

bool InstancingRenderer::createUBO()
{
    return (true);
}

/*bool InstancingRenderer::createVBO(size_t frameIndex)
{
    VkDeviceSize vboSize = m_maxSpriteRenderQueueSize * sizeof(InstanciedSpriteDatum);
    m_spritesRenderQueue.resize(m_maxSpriteRenderQueueSize);

    return VBuffersAllocator::allocBuffer(vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                                          , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,m_vbos[frameIndex]);
}*/

bool InstancingRenderer::expandVBO(size_t frameIndex)
{
    VBuffer oldBuffer = m_vbos[frameIndex];
    //m_maxSpriteRenderQueueSize += VBO_CHUNK_SIZE;
    m_spritesRenderQueue[frameIndex].resize(m_spritesRenderQueue[frameIndex].size() + VBO_CHUNK_SIZE);


    VkDeviceSize vboSize = m_spritesRenderQueue[frameIndex].size() * sizeof(InstanciedSpriteDatum);
    if(!VBuffersAllocator::allocBuffer(vboSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT
                                          , VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,m_vbos[frameIndex]))
        return (false);

    //if(!this->createVBO(frameIndex))
      //  return (false);

    if(m_spritesRenderQueueSize != 0)
    {
        VBuffersAllocator::copyBuffer(oldBuffer,m_vbos[frameIndex],m_spritesRenderQueueSize*sizeof(InstanciedSpriteDatum));
        VBuffersAllocator::freeBuffer(oldBuffer);
    }

    return (true);
}

void InstancingRenderer::cleanup()
{
    for(auto vbo : m_vbos)
        VBuffersAllocator::freeBuffer(vbo);
    m_vbos.clear();

    AbstractRenderer::cleanup();
}

}
