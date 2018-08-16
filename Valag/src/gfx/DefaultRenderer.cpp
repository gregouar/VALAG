#include "Valag/gfx/DefaultRenderer.h"

#include <sstream>
#include <glm/gtc/matrix_transform.hpp>

#include "Valag/utils/Logger.h"
#include "Valag/core/Config.h"
#include "Valag/core/VApp.h"

#include "Valag/VulkanHelpers.h"


namespace vlg
{

const char *DefaultRenderer::DEFAULT_VERTSHADERFILE = "defaultShader.vert.spv";
const char *DefaultRenderer::DEFAULT_FRAGSHADERFILE = "defaultShader.frag.spv";

const size_t DefaultRenderer::MODEL_DYNAMICBUFFER_CHUNKSIZE = 8;

DefaultRenderer::DefaultRenderer(VInstance *vulkanInstance) :
    m_vulkanInstance(vulkanInstance),
    m_defaultTextureSampler(VK_NULL_HANDLE),
    m_defaultRenderPass(VK_NULL_HANDLE),
    m_defaultPipelineLayout(VK_NULL_HANDLE),
    m_defaultPipeline(VK_NULL_HANDLE),
    m_currentCommandBuffer(0)
{
    //m_currentFrameViewUBO = VApp::MAX_FRAMES_IN_FLIGHT - 1;
    this->init();
}

DefaultRenderer::~DefaultRenderer()
{
    this->cleanup();
}


void DefaultRenderer::draw(Sprite *sprite)
{
    m_activeSecondaryCommandBuffers.push_back(sprite->getDrawCommandBuffer(this,m_currentCommandBuffer, m_defaultRenderPass, 0));
}


VkCommandBuffer DefaultRenderer::getCommandBuffer()
{
 //   return m_commandBuffers[m_currentCommandBuffer];
    return m_commandBuffers[(m_currentCommandBuffer - 1) % VApp::MAX_FRAMES_IN_FLIGHT];
}

VkSemaphore DefaultRenderer::getRenderFinishedSemaphore(size_t frameIndex)
{
    return m_renderFinishedSemaphore[frameIndex];
}


void DefaultRenderer::updateBuffers(uint32_t imageIndex)
{
    /*if(m_vulkanInstance == nullptr)
        throw std::runtime_error("No vulkan instance in updateBuffers()");

    VkDevice device = m_vulkanInstance->getDevice();*/


    if(m_needToUpdateViewUBO[m_currentCommandBuffer])
        this->updateViewUBO();

    this->recordPrimaryCommandBuffer(imageIndex);

    m_currentCommandBuffer = (m_currentCommandBuffer + 1) % VApp::MAX_FRAMES_IN_FLIGHT;
}

void DefaultRenderer::updateViewUBO()
{
    if(m_vulkanInstance == nullptr)
        throw std::runtime_error("No vulkan instance in updateViewUBO()");

    VkDevice device = m_vulkanInstance->getDevice();

    ViewUBO viewUbo = {};
    /** this could need to be updated if I implement resizing **/
    viewUbo.view = glm::translate(glm::mat4(1.0f), glm::vec3(-1,-1,0));
    viewUbo.view = glm::scale(viewUbo.view, glm::vec3(2.0f/m_swapchainExtents[0].width,
                                                      2.0f/m_swapchainExtents[0].height,
                                                      1));

    /** I should write a helper for updateBufferMemory **/
    void* data;
    vkMapMemory(device, m_viewBuffersMemory[m_currentCommandBuffer], 0, sizeof(viewUbo), 0, &data);
        memcpy(data, &viewUbo, sizeof(viewUbo));
    vkUnmapMemory(device, m_viewBuffersMemory[m_currentCommandBuffer]);

    m_needToUpdateViewUBO[m_currentCommandBuffer] = false;
}

bool DefaultRenderer::recordPrimaryCommandBuffer(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;  //VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT;

    if (vkBeginCommandBuffer(m_commandBuffers[m_currentCommandBuffer], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (false);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_defaultRenderPass;
    renderPassInfo.framebuffer = m_swapchainFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_vulkanInstance->getSwapchainExtent();

    VkClearValue clearColor = {0.0f, 0.0f, 0.0f, 1.0f};
    renderPassInfo.clearValueCount = 1;
    renderPassInfo.pClearValues = &clearColor;

    vkCmdBeginRenderPass(m_commandBuffers[m_currentCommandBuffer], &renderPassInfo, /*VK_SUBPASS_CONTENTS_INLINE*/ VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

        //vkCmdBindPipeline(m_commandBuffers[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipeline);
        //vkCmdDraw(m_commandBuffers[imageIndex], 3, 1, 0, 0);

      //  vkCmdBindDescriptorSets(m_commandBuffers[m_currentCommandBuffer], VK_PIPELINE_BIND_POINT_GRAPHICS,
        //                        m_defaultPipelineLayout, 0, 1, &m_viewDescriptorSets[m_currentCommandBuffer], 0, nullptr);

        if(!m_activeSecondaryCommandBuffers.empty())
            vkCmdExecuteCommands(m_commandBuffers[m_currentCommandBuffer], (uint32_t) m_activeSecondaryCommandBuffers.size(), m_activeSecondaryCommandBuffers.data());

    vkCmdEndRenderPass(m_commandBuffers[m_currentCommandBuffer]);

    m_activeSecondaryCommandBuffers.clear();

    if (vkEndCommandBuffer(m_commandBuffers[m_currentCommandBuffer]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}


bool DefaultRenderer::allocModelUBO(size_t &index)
{
    bool needToUpdate = false;

    for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        if(m_modelBuffers[i]->allocObject(index))
            needToUpdate = true;
    }

    if(needToUpdate)
        this->updateModelDescriptorSets();

    return needToUpdate;
}

void DefaultRenderer::updateModelUBO(size_t index, void *data, size_t frameIndex)
{
    m_modelBuffers[frameIndex]->updateObject(index, data);
}

void DefaultRenderer::updateModelDescriptorSets()
{
    VkDevice device = m_vulkanInstance->getDevice();

    for (size_t i = 0; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
    {
        VkDescriptorBufferInfo bufferInfo = {};
        bufferInfo.buffer = m_modelBuffers[i]->getBuffer();
        bufferInfo.offset = 0;
        bufferInfo.range = sizeof(ModelUBO);//m_modelBuffers[i]->getBufferRange();

        VkWriteDescriptorSet descriptorWrite = {};
        descriptorWrite.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptorWrite.dstSet = m_modelDescriptorSets[i];
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

void DefaultRenderer::bindAllUBOs(VkCommandBuffer &commandBuffer, size_t frameIndex, size_t modelUBOIndex)
{
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_defaultPipeline);

    VkDescriptorSet descriptorSets[] = {m_viewDescriptorSets[frameIndex],
                                        m_modelDescriptorSets[frameIndex] };

    uint32_t dynamicOffset = m_modelBuffers[frameIndex]->getDynamicOffset(modelUBOIndex);

    vkCmdBindDescriptorSets(commandBuffer,VK_PIPELINE_BIND_POINT_GRAPHICS,
                            m_defaultPipelineLayout,0,2, descriptorSets, 1, &dynamicOffset);
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
    if(m_vulkanInstance == nullptr)
        return (false);

    VkDevice device = m_vulkanInstance->getDevice();

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = m_vulkanInstance->getSwapchainImageFormat();
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
    if(m_vulkanInstance == nullptr)
        return (false);

    VkDevice device = m_vulkanInstance->getDevice();

    VkDescriptorSetLayoutBinding uboLayoutBinding = {};
    uboLayoutBinding.binding = 0;
    uboLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    uboLayoutBinding.descriptorCount = 1 /*1*/;
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
    if(m_vulkanInstance == nullptr)
        return (false);

    VkDevice device = m_vulkanInstance->getDevice();

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
    viewport.width = (float) m_vulkanInstance->getSwapchainExtent().width;
    viewport.height = (float) m_vulkanInstance->getSwapchainExtent().height;
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;

    VkRect2D scissor = {};
    scissor.offset = {0, 0};
    scissor.extent = m_vulkanInstance->getSwapchainExtent();

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

    VkDescriptorSetLayout descriptorSetLayouts[] = {m_viewDescriptorSetLayout, m_modelDescriptorSetLayout};
    pipelineLayoutInfo.setLayoutCount = 2;
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts;
    pipelineLayoutInfo.pushConstantRangeCount = 0; // Optional
    pipelineLayoutInfo.pPushConstantRanges = nullptr; // Optional

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
    if(m_vulkanInstance == nullptr)
        return (false);

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

    return (vkCreateSampler(m_vulkanInstance->getDevice(), &samplerInfo, nullptr, &m_defaultTextureSampler) == VK_SUCCESS);
}

bool DefaultRenderer::createFramebuffers()
{
    if(m_vulkanInstance == nullptr)
        return (false);

    auto swapChainImageViews = m_vulkanInstance->getSwapchainImageViews();

    m_swapchainFramebuffers.resize(swapChainImageViews.size());
    m_swapchainExtents.resize(swapChainImageViews.size());

    for (size_t i = 0; i < swapChainImageViews.size(); ++i)
    {
        VkImageView attachments[] = {
            swapChainImageViews[i]
        };

        m_swapchainExtents[i] = m_vulkanInstance->getSwapchainExtent();

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_defaultRenderPass;
        framebufferInfo.attachmentCount = 1;
        framebufferInfo.pAttachments = attachments;
        framebufferInfo.width = m_swapchainExtents[i].width;
        framebufferInfo.height = m_swapchainExtents[i].height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_vulkanInstance->getDevice(), &framebufferInfo, nullptr, &m_swapchainFramebuffers[i]) != VK_SUCCESS)
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

    return (vkCreateDescriptorPool(m_vulkanInstance->getDevice(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool DefaultRenderer::createDescriptorSets()
{
    VkDevice device = m_vulkanInstance->getDevice();

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

        this->updateModelDescriptorSets();
    }

    return (true);
}

bool DefaultRenderer::createPrimaryCommandBuffers()
{
    if(m_vulkanInstance == nullptr)
        return (false);

    //m_commandBuffers.resize(m_swapchainFramebuffers.size());
    m_commandBuffers.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_vulkanInstance->getCommandPool(COMMANDPOOL_DEFAULT);
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = (uint32_t) m_commandBuffers.size();

    if (vkAllocateCommandBuffers(m_vulkanInstance->getDevice(), &allocInfo, m_commandBuffers.data()) != VK_SUCCESS)
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

    if(m_vulkanInstance == nullptr)
        return (false);

    m_renderFinishedSemaphore.resize(VApp::MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for(size_t i = 0 ; i < VApp::MAX_FRAMES_IN_FLIGHT ; ++i)
        if(vkCreateSemaphore(m_vulkanInstance->getDevice(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphore[i]) != VK_SUCCESS)
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

void DefaultRenderer::cleanup()
{
    if(m_vulkanInstance == nullptr)
        return;

    VkDevice device = m_vulkanInstance->getDevice();

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
