#include "Valag/gfx/DefaultRenderer.h"

#include <sstream>

#include "Valag/utils/Profiler.h"
#include "Valag/utils/Logger.h"
#include "Valag/core/Config.h"
#include "Valag/core/VApp.h"

#include "Valag/vulkanImpl/VulkanHelpers.h"


namespace vlg
{

const char *DefaultRenderer::DEFAULT_VERTSHADERFILE = "defaultShader.vert.spv";
const char *DefaultRenderer::DEFAULT_FRAGSHADERFILE = "defaultShader.frag.spv";

const size_t DefaultRenderer::MODEL_DYNAMICBUFFER_CHUNKSIZE = 1024;

DefaultRenderer::DefaultRenderer(RenderWindow *targetWindow) :
    m_targetWindow(targetWindow),
    m_defaultTextureSampler(VK_NULL_HANDLE),
    m_defaultRenderPass(VK_NULL_HANDLE),
    m_defaultPipelineLayout(VK_NULL_HANDLE),
    m_defaultPipeline(VK_NULL_HANDLE),
    m_currentFrame(0)
{
    m_needToExpandModelBuffers = false;
    m_oldModelBuffers = std::vector<VkBuffer> (VApp::MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    m_oldModelBuffersMemory.resize(VApp::MAX_FRAMES_IN_FLIGHT, VK_NULL_HANDLE);
    //m_currentFrameViewUBO = VApp::MAX_FRAMES_IN_FLIGHT - 1;
    this->init();
}

DefaultRenderer::~DefaultRenderer()
{
    this->cleanup();
}


void DefaultRenderer::draw(Sprite *sprite)
{
    VkCommandBuffer commandBuffer = sprite->getDrawCommandBuffer(this,m_currentFrame, m_defaultRenderPass, 0);
    if(commandBuffer != VK_NULL_HANDLE)
        m_activeSecondaryCommandBuffers.push_back(commandBuffer);
}


VkCommandBuffer DefaultRenderer::getCommandBuffer()
{
 //   return m_commandBuffers[m_currentFrame];
    return m_commandBuffers[(m_currentFrame - 1) % VApp::MAX_FRAMES_IN_FLIGHT];
}

VkSemaphore DefaultRenderer::getRenderFinishedSemaphore(size_t frameIndex)
{
    return m_renderFinishedSemaphore[frameIndex];
}


void DefaultRenderer::updateBuffers(uint32_t imageIndex)
{
    if(m_needToUpdateViewUBO[m_currentFrame])
        this->updateViewUBO();

    Profiler::pushClock("Record primary buffer");
    this->recordPrimaryCommandBuffer(imageIndex);
    Profiler::popClock();

    m_currentFrame = (m_currentFrame + 1) % VApp::MAX_FRAMES_IN_FLIGHT;
}

void DefaultRenderer::checkBuffersExpansion()
{
    size_t oldFrame = (m_currentFrame - 1) % VApp::MAX_FRAMES_IN_FLIGHT;

    VkDevice device = VInstance::device();

    if(m_oldModelBuffers[oldFrame] != VK_NULL_HANDLE)
    {
        vkDestroyBuffer(device, m_oldModelBuffers[oldFrame], nullptr);
        vkFreeMemory(device, m_oldModelBuffersMemory[oldFrame], nullptr);
        m_oldModelBuffers[oldFrame] = VK_NULL_HANDLE;
    }

    if(m_needToExpandModelBuffers)
    {
        m_oldModelBuffers[oldFrame] = m_modelBuffers[oldFrame]->getBuffer();
        m_oldModelBuffersMemory[oldFrame] = m_modelBuffers[oldFrame]->getBufferMemory();
        m_modelBuffers[oldFrame]->expandBuffers(false);
        this->updateModelDescriptorSets(oldFrame);
        m_needToExpandModelBuffers = false;
    }

    m_texturesArrayManager->checkUpdateDescriptorSets(oldFrame);
}

void DefaultRenderer::updateViewUBO()
{
    VkDevice device = VInstance::device();

    ViewUBO viewUbo = {};
    /** this could need to be updated if I implement resizing **/
    viewUbo.view = glm::translate(glm::mat4(1.0f), glm::vec3(-1,-1,0));
    viewUbo.view = glm::scale(viewUbo.view, glm::vec3(2.0f/m_swapchainExtents[0].width,
                                                      2.0f/m_swapchainExtents[0].height,
                                                      1));

    /** I should write a helper for updateBufferMemory **/
    void* data;
    vkMapMemory(device, m_viewBuffersMemory[m_currentFrame], 0, sizeof(viewUbo), 0, &data);
        memcpy(data, &viewUbo, sizeof(viewUbo));
    vkUnmapMemory(device, m_viewBuffersMemory[m_currentFrame]);

    m_needToUpdateViewUBO[m_currentFrame] = false;
}

bool DefaultRenderer::recordPrimaryCommandBuffer(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;  //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    if (vkBeginCommandBuffer(m_commandBuffers[m_currentFrame], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (false);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_defaultRenderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_targetWindow->getSwapchainExtent();

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffers[m_currentFrame], &renderPassInfo, /*VK_SUBPASS_CONTENTS_INLINE*/ VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        if(!m_activeSecondaryCommandBuffers.empty())
            vkCmdExecuteCommands(m_commandBuffers[m_currentFrame], (uint32_t) m_activeSecondaryCommandBuffers.size(), m_activeSecondaryCommandBuffers.data());

    vkCmdEndRenderPass(m_commandBuffers[m_currentFrame]);

    m_activeSecondaryCommandBuffers.clear();

    if (vkEndCommandBuffer(m_commandBuffers[m_currentFrame]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}


bool DefaultRenderer::allocModelUBO(size_t &index, size_t frameIndex)
{
   // bool needToUpdate = false;

    /*if(m_modelBuffers[frameIndex]->allocObject(index))
        needToUpdate = true;

    if(needToUpdate)
        this->updateModelDescriptorSets(frameIndex);

    return needToUpdate;*/


    if(m_modelBuffers[frameIndex]->isFull())
    {
        m_needToExpandModelBuffers = true;
        return (false);
    }

    m_modelBuffers[frameIndex]->allocObject(index);

    return (true);
}

void DefaultRenderer::updateModelUBO(size_t index, void *data, size_t frameIndex)
{
    m_modelBuffers[frameIndex]->updateObject(index, data);
}

void DefaultRenderer::updateModelDescriptorSets(size_t frameIndex)
{
    VkDevice device = VInstance::device();

    //for (size_t i = 0; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_modelBuffers[frameIndex]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ModelUBO);//m_modelBuffers[i]->getBufferRange();

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_modelDescriptorSets[frameIndex];
        descriptorWrite.dstBinding = 0; //Bind number
        descriptorWrite.dstArrayElement = 0; //Set number
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }
}

size_t DefaultRenderer::getModelUBOBufferVersion(size_t frameIndex)
{
    return m_modelBuffers[frameIndex]->getBufferVersion();
}


bool DefaultRenderer::bindTexture(VkCommandBuffer &commandBuffer, AssetTypeID textureID, size_t frameIndex)
{
    int texArrayID;

    if(!m_texturesArrayManager->bindTexture(textureID, frameIndex, &texArrayID))
        return (false);

    vkCmdPushConstants(commandBuffer, m_defaultPipelineLayout,
            VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(int), (void*)&texArrayID);

    return (true);
}

void DefaultRenderer::bindAllUBOs(VkCommandBuffer &commandBuffer, size_t frameIndex, size_t modelUBOIndex)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipeline);

    VkDescriptorSet descriptorSets[] = {m_viewDescriptorSets[frameIndex],
                                        m_texturesArrayManager->getDescriptorSet(frameIndex),
                                        m_modelDescriptorSets[frameIndex] };

    uint32_t dynamicOffset = m_modelBuffers[frameIndex]->getDynamicOffset(modelUBOIndex);

    vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_defaultPipelineLayout,0,3, descriptorSets, 1, &dynamicOffset);
}

bool DefaultRenderer::init()
{
    if(!this->createTextureSampler())
    {
        Logger::error("Cannot create default texture sampler");
        return (false);
    }

    if(!this->createRenderPass())
    {
        Logger::error("Cannot create default render pass");
        return (false);
    }

    if(!this->createDescriptorSetLayouts())
    {
        Logger::error("Cannot create default descriptor set layout");
        return (false);
    }


    if(!this->createTexturesArrayManager())
    {
        Logger::error("Cannot create default textures array manager");
        return (false);
    }


    if(!this->createGraphicsPipeline())
    {
        Logger::error("Cannot create default graphics pipeline");
        return (false);
    }

    if(!this->createFramebuffers())
    {
        Logger::error("Cannot create default framebuffers");
        return (false);
    }

    if(!this->createUBO())
    {
        Logger::error("Cannot create default UBOs");
        return (false);
    }

    if(!this->createDescriptorPool())
    {
        Logger::error("Cannot create default descriptor pool");
        return (false);
    }

    if(!this->createDescriptorSets())
    {
        Logger::error("Cannot create default descriptor sets");
        return (false);
    }

    if(!this->createPrimaryCommandBuffers())
    {
        Logger::error("Cannot create primary command buffers");
        return (false);
    }

    if(!this->createSemaphores())
    {
        Logger::error("Cannot create default renderer semaphores");
        return (false);
    }

    return (true);
}

bool DefaultRenderer::createRenderPass()
{
    VkDevice device = VInstance::device();

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_targetWindow->getSwapchainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;

    VkSubpassDependency dependency = {};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
    dependency.dstSubpass = 0;
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = 1;
    renderPassInfo.pAttachments = &colorAttachment;
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    return (vkCreateRenderPass(device, &renderPassInfo, nullptr, &m_defaultRenderPass) == VK_SUCCESS);
}

bool DefaultRenderer::createDescriptorSetLayouts()
{
    VkDevice device = VInstance::device();

    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1;
    uboLayoutBinding.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
    uboLayoutBinding.pImmutableSamplers = nullptr; // Optional

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = 1;
    layoutInfo.pBindings = &uboLayoutBinding;

    if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_viewDescriptorSetLayout) != VK_SUCCESS)
        return (false);

    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC;

    if(vkCreateDescriptorSetLayout(device, &layoutInfo, nullptr, &m_modelDescriptorSetLayout) != VK_SUCCESS)
        return (false);

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

    auto bindingDescription = Vertex2D::getBindingDescription();
    auto attributeDescriptions = Vertex2D::getAttributeDescriptions();

    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();


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

    VkDescriptorSetLayout descriptorSetLayouts[] = {m_viewDescriptorSetLayout,
                                                    m_texturesArrayManager->getDescriptorSetLayout(),
                                                    m_modelDescriptorSetLayout};
    pipelineLayoutInfo.setLayoutCount = 3;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;


    VkPushConstantRange pushConstantRange = {};
	pushConstantRange.offset = 0;
	pushConstantRange.size = sizeof(int);
	pushConstantRange.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;

	pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    pipelineLayoutInfo.pushConstantRangeCount = 1;

    //pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    //pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

    if (vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &m_defaultPipelineLayout) != VK_SUCCESS)
    {
        Logger::error("Failed to create pipeline layout");
        return (false);
    }

    VkGraphicsPipelineCreateInfo pipelineInfo = {};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = nullptr; // Optional
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = nullptr; // Optional
    pipelineInfo.layout = m_defaultPipelineLayout;
    pipelineInfo.renderPass = m_defaultRenderPass;
    pipelineInfo.subpass = 0;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE; // Optional
    pipelineInfo.basePipelineIndex = -1; // Optional

    if (vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &m_defaultPipeline) != VK_SUCCESS)
        return (false);

    vkDestroyShaderModule(device, fragShaderModule, nullptr);
    vkDestroyShaderModule(device, vertShaderModule, nullptr);

    return (true);
}


bool DefaultRenderer::createTextureSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter = VK_FILTER_LINEAR;
    samplerInfo.minFilter = VK_FILTER_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    samplerInfo.anisotropyEnable = VK_TRUE;
    samplerInfo.maxAnisotropy = Config::getInt("Graphics","AnisotropicFiltering",VApp::DEFAULT_ANISOTROPIC);
    samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;

    return (vkCreateSampler(VInstance::device(), &samplerInfo, nullptr, &m_defaultTextureSampler) == VK_SUCCESS);
}

bool DefaultRenderer::createFramebuffers()
{
    auto swapChainImageViews = m_targetWindow->getSwapchainImageViews();

    m_swapchainFramebuffers.resize(swapChainImageViews.size());
    m_swapchainExtents.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); ++i)
    {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        m_swapchainExtents[i] = m_targetWindow->getSwapchainExtent();

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_defaultRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainExtents[i].width;
        framebufferInfo.height = m_swapchainExtents[i].height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS)
            return (false);

    }

    return (true);
}

bool DefaultRenderer::createDescriptorPool()
{
    VkDescriptorPoolSize poolSize = {};
    poolSize.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    poolSize.descriptorCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT*2);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = 1;
    poolInfo.pPoolSizes = &poolSize;

    poolInfo.maxSets = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT*2);

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool DefaultRenderer::createDescriptorSets()
{
    VkDevice device = VInstance::device();

    {
        std::vector<VkDescriptorSetLayout> layouts(VApp::MAX_FRAMES_IN_FLIGHT, m_viewDescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        m_viewDescriptorSets.resize(VApp::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo,m_viewDescriptorSets.data()) != VK_SUCCESS)
            return (false);

    }

    for (size_t i = 0; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_viewBuffers[i];
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ViewUBO);

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_viewDescriptorSets[i];
        descriptorWrite.dstBinding = 0; //Bind number
        descriptorWrite.dstArrayElement = 0; //Set number
        descriptorWrite.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        descriptorWrite.descriptorCount = 1;
        descriptorWrite.pBufferInfo = &bufferInfo;
        descriptorWrite.pImageInfo = nullptr; // Optional
        descriptorWrite.pTexelBufferView = nullptr; // Optional

        vkUpdateDescriptorSets(device, 1, &descriptorWrite, 0, nullptr);
    }

    {
        std::vector<VkDescriptorSetLayout> layouts(VApp::MAX_FRAMES_IN_FLIGHT, m_modelDescriptorSetLayout);
        VkDescriptorSetAllocateInfo allocInfo = {};
        allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
        allocInfo.descriptorPool = m_descriptorPool;
        allocInfo.descriptorSetCount = static_cast<uint32_t>(VApp::MAX_FRAMES_IN_FLIGHT);
        allocInfo.pSetLayouts = layouts.data();

        m_modelDescriptorSets.resize(VApp::MAX_FRAMES_IN_FLIGHT);
        if (vkAllocateDescriptorSets(device, &allocInfo,m_modelDescriptorSets.data()) != VK_SUCCESS)
            return (false);
    }

    for (size_t i = 0; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
        this->updateModelDescriptorSets(i);

    return (true);
}

bool DefaultRenderer::createPrimaryCommandBuffers()
{
    //m_commandBuffers.resize(m_swapchainFramebuffers.size());
    m_commandBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VInstance::commandPool(COMMANDPOOL_DEFAULT);
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) m_commandBuffers.size();

    if (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
    {
        Logger::error("Failed to allocate command buffers");
        return (false);
    }

    /*for (size_t i = 0; i < m_commandBuffers.size(); ++i)
    {
        VkCommandBufferBeginInfo beginInfo = {};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;  //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

        if (vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo) != VK_SUCCESS)
        {
            Logger::error("Failed to begin recording command buffer");
            return (false);
        }

        VkRenderPassBeginInfo renderPassInfo = {};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_defaultRenderPass;
        renderPassInfo.framebuffer = m_swapchainFramebuffers[i];
        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = m_vulkanInstance->getSwapchainExtent();

        VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
        renderPassInfo.clearValueCount = 1;
        renderPassInfo.pClearValues = &clearColor;

        vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

            vkCmdBindPipeline(m_commandBuffers[i], VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipeline);

            //vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);

        vkCmdEndRenderPass(m_commandBuffers[i]);

        if (vkEndCommandBuffer(m_commandBuffers[i]) != VK_SUCCESS)
        {
            Logger::error("Failed to record command buffer");
            return (false);
        }
    }*/

    return (true);
}

bool DefaultRenderer::createSemaphores()
{
    /*if(m_vulkanInstance == nullptr)
        return (false);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    return (vkCreateSemaphore(m_vulkanInstance->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore) == VK_SUCCESS);*/

    m_renderFinishedSemaphore.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
        if(vkCreateSemaphore(VInstance::device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore[i]) != VK_SUCCESS)
            return (false);

    return (true);
}

bool DefaultRenderer::createUBO()
{
    VkDeviceSize bufferSize = sizeof(ViewUBO);

    m_viewBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_viewBuffersMemory.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_needToUpdateViewUBO.resize(VApp::MAX_FRAMES_IN_FLIGHT);
    m_modelBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    for (size_t i = 0; i < VApp::MAX_FRAMES_IN_FLIGHT; ++i)
    {
        if(!VulkanHelpers::createBuffer(bufferSize, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
                                    m_viewBuffers[i], m_viewBuffersMemory[i]))
            return (false);

        //this->updateViewUBO();
        m_needToUpdateViewUBO[i] = true;

        m_modelBuffers[i] = new DynamicUBO(sizeof(ModelUBO),MODEL_DYNAMICBUFFER_CHUNKSIZE);
    }
    return (true);
}

bool DefaultRenderer::createTexturesArrayManager()
{
    m_texturesArrayManager = new TexturesArrayManager();
    return (true);
}

void DefaultRenderer::cleanup()
{
    VkDevice device = VInstance::device();

    delete m_texturesArrayManager;

    vkDestroyDescriptorPool(device,m_descriptorPool,nullptr);

    for(auto modelBuffer : m_modelBuffers)
        delete modelBuffer;

    for(auto ubo : m_viewBuffers)
        vkDestroyBuffer(device, ubo, nullptr);

    for(auto uboMemory : m_viewBuffersMemory)
        vkFreeMemory(device, uboMemory, nullptr);

    for (auto semaphore : m_renderFinishedSemaphore)
        vkDestroySemaphore(device, semaphore, nullptr);

    for (auto framebuffer : m_swapchainFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);

    if(m_defaultPipeline != VK_NULL_HANDLE)
        vkDestroyPipeline(device, m_defaultPipeline, nullptr);

    if(m_defaultPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(device, m_defaultPipelineLayout, nullptr);

    vkDestroyDescriptorSetLayout(device, m_viewDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_modelDescriptorSetLayout, nullptr);

    if(m_defaultRenderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(device, m_defaultRenderPass, nullptr);

    if(m_defaultTextureSampler != VK_NULL_HANDLE)
        vkDestroySampler(device, m_defaultTextureSampler, nullptr);


}



}
