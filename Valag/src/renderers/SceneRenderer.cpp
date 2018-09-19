#include "Valag/renderers/SceneRenderer.h"

#include "Valag/core/VApp.h"
#include "Valag/scene/IsoSpriteEntity.h"
#include "Valag/utils/Logger.h"

namespace vlg
{

const char *SceneRenderer::ISOSPRITE_VERTSHADERFILE = "isoSpriteShader.vert.spv";
const char *SceneRenderer::ISOSPRITE_FRAGSHADERFILE = "isoSpriteShader.frag.spv";
const char *SceneRenderer::AMBIENTLIGHTING_VERTSHADERFILE = "ambientLighting.vert.spv";
const char *SceneRenderer::AMBIENTLIGHTING_FRAGSHADERFILE = "ambientLighting.frag.spv";
const char *SceneRenderer::TONEMAPPING_VERTSHADERFILE = "toneMapping.vert.spv";
const char *SceneRenderer::TONEMAPPING_FRAGSHADERFILE = "toneMapping.frag.spv";


SceneRenderer::SceneRenderer(RenderWindow *targetWindow, RendererName name, RenderereOrder order) : AbstractRenderer(targetWindow, name, order)
{
    this->init();
}

SceneRenderer::~SceneRenderer()
{
    this->cleanup();
}

void SceneRenderer::update(size_t frameIndex)
{
    //IsoSpriteEntity::updateRendering(frameIndex);
    AbstractRenderer::update(frameIndex);
}

/*void SceneRenderer::draw(Scene* scene)
{
   // scene->getRootNode()->searchInsideForEntities();
}

void SceneRenderer::draw(IsoSpriteEntity* sprite)
{
    InstanciedIsoSpriteDatum spriteDatum = sprite->getIsoSpriteDatum();
    m_spritesVbos[m_curFrameIndex].push_back(spriteDatum);
}*/

void SceneRenderer::addToSpritesVbo(const InstanciedIsoSpriteDatum &datum)
{
    m_spritesVbos[m_curFrameIndex].push_back(datum);
}

void SceneRenderer::updateCmb(uint32_t imageIndex)
{
    this->recordDefferedCmb(imageIndex);

    this->submitToGraphicsQueue(imageIndex);

    m_curFrameIndex = (m_curFrameIndex + 1) % m_targetWindow->getFramesCount();
}


VkCommandBuffer SceneRenderer::getCommandBuffer(size_t frameIndex , size_t imageIndex)
{
    return m_primaryCmb[imageIndex];
}

VkSemaphore SceneRenderer::getFinalPassWaitSemaphore(size_t frameIndex)
{
    return m_ambientLightingToToneMappingSemaphore[frameIndex];
}

void SceneRenderer::submitToGraphicsQueue(size_t imageIndex)
{
    std::vector<VkSubmitInfo> submitInfos;

    submitInfos.resize(2);

    for(size_t i = 0 ; i < submitInfos.size() ; ++i)
    {
        submitInfos[i] = {};
        submitInfos[i].sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    }

    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};

    submitInfos[0].pWaitDstStageMask = waitStages;
    submitInfos[0].signalSemaphoreCount = 1;
    submitInfos[0].pSignalSemaphores = &m_deferredToAmbientLightingSemaphore[m_curFrameIndex];
    submitInfos[0].commandBufferCount = 1;
    submitInfos[0].pCommandBuffers = &m_deferredCmb[m_curFrameIndex];

    VkSemaphore waitSemaphores[] = {m_deferredToAmbientLightingSemaphore[m_curFrameIndex]};
    submitInfos[1].pWaitDstStageMask = waitStages;
    submitInfos[1].waitSemaphoreCount = 1;
    submitInfos[1].pWaitSemaphores = waitSemaphores;
    submitInfos[1].signalSemaphoreCount = 1;
    submitInfos[1].pSignalSemaphores = &m_ambientLightingToToneMappingSemaphore[m_curFrameIndex];
    submitInfos[1].commandBufferCount = 1;
    submitInfos[1].pCommandBuffers = &m_ambientLightingCmb[imageIndex];


    VInstance::submitToGraphicsQueue(submitInfos, 0);
}

bool SceneRenderer::recordPrimaryCmb(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT ;

    if (vkBeginCommandBuffer(m_primaryCmb[imageIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (false);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_renderPass;
    renderPassInfo.framebuffer = m_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_targetWindow->getSwapchainExtent();

    std::array<VkClearValue, 2> clearValues = {};
    if(m_order == Renderer_First || m_order == Renderer_Unique)
    {
        clearValues[0].color = {0.0f, 0.0f, 0.0f, 1.0f};
        clearValues[1].depthStencil = {0.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();
    }

    vkCmdBeginRenderPass(m_primaryCmb[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_toneMappingPipeline.bind(m_primaryCmb[imageIndex]);
        vkCmdBindDescriptorSets(m_primaryCmb[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_toneMappingPipeline.getLayout(),
                                0, 1, &m_hdrDescriptorSets[imageIndex], 0, NULL);
        vkCmdDraw(m_primaryCmb[imageIndex], 3, 1, 0, 0);

    vkCmdEndRenderPass(m_primaryCmb[imageIndex]);

    if (vkEndCommandBuffer(m_primaryCmb[imageIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}


bool SceneRenderer::recordDefferedCmb(uint32_t imageIndex)
{
    size_t spritesVboSize = m_spritesVbos[m_curFrameIndex].uploadVBO();

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT ;

    if (vkBeginCommandBuffer(m_deferredCmb[m_curFrameIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (false);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_deferredRenderPass;
    renderPassInfo.framebuffer = m_deferredFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_targetWindow->getSwapchainExtent();

    std::array<VkClearValue, 4> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
    clearValues[1].depthStencil = {0.0f, 0};
    clearValues[2].color = {0.0f, 0.0f, 0.0f, 0.0f};
    clearValues[3].color = {0.0f, 0.0f, 0.0f, 0.0f};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();


    vkCmdBeginRenderPass(m_deferredCmb[m_curFrameIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_deferredPipeline.bind(m_deferredCmb[m_curFrameIndex]);

        VkDescriptorSet descriptorSets[] = {m_renderView.getDescriptorSet(m_curFrameIndex),
                                            VTexturesManager::instance()->getDescriptorSet(m_curFrameIndex) };

        vkCmdBindDescriptorSets(m_deferredCmb[m_curFrameIndex],VK_PIPELINE_BIND_POINT_GRAPHICS,
                                m_deferredPipeline.getLayout(),0,2, descriptorSets, 0, nullptr);

        if(spritesVboSize != 0)
        {
            VBuffer vertexBuffer = m_spritesVbos[m_curFrameIndex].getBuffer();
            vkCmdBindVertexBuffers(m_deferredCmb[m_curFrameIndex], 0, 1, &vertexBuffer.buffer, &vertexBuffer.offset);
            vkCmdDraw(m_deferredCmb[m_curFrameIndex], 4, spritesVboSize, 0, 0);
        }

    vkCmdEndRenderPass(m_deferredCmb[m_curFrameIndex]);

    if (vkEndCommandBuffer(m_deferredCmb[m_curFrameIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);

}


bool SceneRenderer::recordAmbientLightingCmb(uint32_t imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT  ;

    if (vkBeginCommandBuffer(m_ambientLightingCmb[imageIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (false);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_ambientLightingRenderPass;
    renderPassInfo.framebuffer = m_ambientLightingFramebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_targetWindow->getSwapchainExtent();

    std::array<VkClearValue, 1> clearValues = {};
    clearValues[0].color = {0.0f, 0.0f, 0.0f, 0.0f};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_ambientLightingCmb[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

        m_ambientLightingPipeline.bind(m_ambientLightingCmb[imageIndex]);
        vkCmdBindDescriptorSets(m_ambientLightingCmb[imageIndex], VK_PIPELINE_BIND_POINT_GRAPHICS, m_ambientLightingPipeline.getLayout(),
                                0, 1, &m_deferredDescriptorSets[imageIndex], 0, NULL);
        vkCmdDraw(m_ambientLightingCmb[imageIndex], 3, 1, 0, 0);

    vkCmdEndRenderPass(m_ambientLightingCmb[imageIndex]);

    if (vkEndCommandBuffer(m_ambientLightingCmb[imageIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}

bool SceneRenderer::init()
{
     //IsoSpriteEntity::initRendering(m_targetWindow->getFramesCount());

    //m_renderView.setExtent({m_targetWindow->getSwapchainExtent().width,
      //                      m_targetWindow->getSwapchainExtent().height});
    m_renderView.setDepthFactor(1024*1024);

    m_spritesVbos = std::vector<DynamicVBO<InstanciedIsoSpriteDatum> >(m_targetWindow->getFramesCount(), DynamicVBO<InstanciedIsoSpriteDatum>(1024));

   // m_renderView.setScreenOffset(glm::vec3(-1.0f, -1.0f, 0.5f));
    m_renderView.setScreenOffset(glm::vec3(0.0f, 0.0f, 0.5f));

    this->createAttachments();

    return AbstractRenderer::init();
}

bool SceneRenderer::createRenderPass()
{
    if(!this->createDeferredRenderPass())
        return (false);
    if(!this->createAmbientLightingRenderPass())
        return (false);

    if(!this->createSemaphores())
        return (false);

    return AbstractRenderer::createRenderPass();
}

bool SceneRenderer::createDescriptorSetLayouts()
{
    VkDevice device = VInstance::device();

    std::vector<VkDescriptorSetLayoutBinding> deferredLayoutBindings;

    deferredLayoutBindings.resize(4);
    for(uint32_t i = 0 ; i < 4 ; ++i)
    {
        deferredLayoutBindings[i] = {};
        deferredLayoutBindings[i].binding = i;
        deferredLayoutBindings[i].descriptorCount = 1;
        deferredLayoutBindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        deferredLayoutBindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    }

    VkDescriptorSetLayoutCreateInfo deferredLayoutInfo = {};
    deferredLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    deferredLayoutInfo.bindingCount = static_cast<uint32_t>(deferredLayoutBindings.size());
    deferredLayoutInfo.pBindings = deferredLayoutBindings.data();

    if(vkCreateDescriptorSetLayout(device, &deferredLayoutInfo, nullptr, &m_deferredDescriptorSetLayout) != VK_SUCCESS)
        return (false);

    VkDescriptorSetLayoutBinding hdrLayoutBinding = {};
    hdrLayoutBinding = {};
    hdrLayoutBinding.binding = 0;
    hdrLayoutBinding.descriptorCount = 1;
    hdrLayoutBinding.stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
    hdrLayoutBinding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;

    VkDescriptorSetLayoutCreateInfo hdrLayoutInfo = {};
    hdrLayoutInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    hdrLayoutInfo.bindingCount = 1;
    hdrLayoutInfo.pBindings = &hdrLayoutBinding;

    if(vkCreateDescriptorSetLayout(device, &hdrLayoutInfo, nullptr, &m_hdrDescriptorSetLayout) != VK_SUCCESS)
        return (false);

    return (true);
}

bool SceneRenderer::createGraphicsPipeline()
{
    if(!this->createDeferredPipeline())
        return (false);
    if(!this->createAmbientLightingPipeline())
        return (false);
    if(!this->createToneMappingPipeline())
        return (false);

    return (true);
}


bool SceneRenderer::createFramebuffers()
{
    if(!this->createDeferredFramebuffers())
        return (false);

    if(!this->createAmbientLightingFramebuffers())
        return (false);

    return AbstractRenderer::createFramebuffers();
}

bool SceneRenderer::createDescriptorPool()
{
    uint32_t imagesCount = static_cast<uint32_t>(m_targetWindow->getSwapchainSize());

    std::vector<VkDescriptorPoolSize> poolSizes;
    poolSizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imagesCount});
    poolSizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, imagesCount});

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = poolSizes.size();
    poolInfo.pPoolSizes = poolSizes.data();

    poolInfo.maxSets = imagesCount * poolSizes.size();

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool SceneRenderer::createDescriptorSets()
{
    VkDevice device = VInstance::device();
    uint32_t imagesCount = static_cast<uint32_t>(m_targetWindow->getSwapchainSize());

    std::vector<VkDescriptorSetLayout> layouts(imagesCount, m_deferredDescriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool = m_descriptorPool;
    allocInfo.descriptorSetCount = static_cast<uint32_t>(imagesCount);
    allocInfo.pSetLayouts = layouts.data();


    m_deferredDescriptorSets.resize(imagesCount);
    if (vkAllocateDescriptorSets(device, &allocInfo,m_deferredDescriptorSets.data()) != VK_SUCCESS)
        return (false);

    layouts = std::vector<VkDescriptorSetLayout>(imagesCount, m_hdrDescriptorSetLayout);
    m_hdrDescriptorSets.resize(imagesCount);
    allocInfo.pSetLayouts = layouts.data();
    if (vkAllocateDescriptorSets(device, &allocInfo,m_hdrDescriptorSets.data()) != VK_SUCCESS)
        return (false);

    for (size_t i = 0; i < imagesCount ; ++i)
    {

        std::array<VkWriteDescriptorSet,4+1> descriptorWrites;
        for(size_t j = 0 ; j < descriptorWrites.size() ; ++j)
        {
            descriptorWrites[j] = {};
            descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j].dstArrayElement = 0;
            descriptorWrites[j].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j].descriptorCount = 1;
        }

        std::array<VkDescriptorImageInfo,4> deferredImageInfos{};

        deferredImageInfos[0].imageView = m_albedoAttachments[i].view;
        deferredImageInfos[1].imageView = m_heightAttachments[i].view;
        deferredImageInfos[2].imageView = m_normalAttachments[i].view;
        deferredImageInfos[3].imageView = m_rmtAttachments[i].view;

        for(size_t j = 0 ; j < deferredImageInfos.size() ; ++j)
        {
            deferredImageInfos[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            deferredImageInfos[j].sampler     = m_attachmentsSampler;

            descriptorWrites[j].dstSet = m_deferredDescriptorSets[i];
            descriptorWrites[j].dstBinding = j;
            descriptorWrites[j].pImageInfo = &deferredImageInfos[j];
        }

        VkDescriptorImageInfo hdrImageInfo = {};
        hdrImageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        hdrImageInfo.sampler     = m_attachmentsSampler;
        hdrImageInfo.imageView   = m_hdrAttachements[i].view;

        descriptorWrites[4].dstSet = m_hdrDescriptorSets[i];
        descriptorWrites[4].dstBinding = 0;
        descriptorWrites[4].pImageInfo = &hdrImageInfo;

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    return (true);
}

bool SceneRenderer::createUBO()
{
    return (true);
}

bool SceneRenderer::createPrimaryCmb()
{
    if(!this->createDeferredCmb())
        return (false);
    if(!this->createAmbientLightingCmb())
        return (false);


    m_primaryCmb.resize(m_targetWindow->getSwapchainSize());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VInstance::commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_primaryCmb.size());

    if (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_primaryCmb.data()) != VK_SUCCESS)
        return (false);

    for(size_t i = 0 ; i < m_primaryCmb.size() ; ++i)
        this->recordPrimaryCmb(i);

    return (true);

    //return AbstractRenderer::createPrimaryCommandBuffers();
}

bool SceneRenderer::createAttachments()
{
    size_t imagesCount = m_targetWindow->getSwapchainSize();
    m_albedoAttachments.resize(imagesCount);
    m_heightAttachments.resize(imagesCount);
    m_normalAttachments.resize(imagesCount);
    m_rmtAttachments.resize(imagesCount);
    m_hdrAttachements.resize(imagesCount);

    uint32_t width = m_targetWindow->getSwapchainExtent().width;
    uint32_t height = m_targetWindow->getSwapchainExtent().height;

    for(size_t i = 0 ; i < imagesCount ; ++i)
    {
        if(!
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_albedoAttachments[i]) &
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_D32_SFLOAT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, m_heightAttachments[i]) &
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_normalAttachments[i]) &
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_rmtAttachments[i]) &
            VulkanHelpers::createAttachment(width, height, VK_FORMAT_R16G16B16A16_SFLOAT, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT, m_hdrAttachements[i])
        )
            return (false);

        /*VulkanHelpers::transitionImageLayout(m_albedoAttachments[i].image.vkImage, 0, VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VulkanHelpers::transitionImageLayout(m_heightAttachments[i].image.vkImage, 0, VK_FORMAT_D32_SFLOAT,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL);
        VulkanHelpers::transitionImageLayout(m_normalAttachments[i].image.vkImage, 0, VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VulkanHelpers::transitionImageLayout(m_rmtAttachments[i].image.vkImage, 0, VK_FORMAT_R8G8B8A8_UNORM,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);
        VulkanHelpers::transitionImageLayout(m_hdrAttachements[i].image.vkImage, 0, VK_FORMAT_R16G16B16A16_SFLOAT,
                                             VK_IMAGE_LAYOUT_UNDEFINED,VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL);*/
    }

    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType       = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter   = VK_FILTER_NEAREST;
    samplerInfo.minFilter   = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode  = VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias  = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    samplerInfo.unnormalizedCoordinates = VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.anisotropyEnable = VK_FALSE;

    return (vkCreateSampler(VInstance::device(), &samplerInfo, nullptr, &m_attachmentsSampler) == VK_SUCCESS);
}

bool SceneRenderer::createDeferredFramebuffers()
{
    size_t imagesCount = m_targetWindow->getSwapchainSize();

    m_deferredFramebuffers.resize(imagesCount);

    for (size_t i = 0; i < imagesCount ; ++i)
    {
        std::array<VkImageView, 4> attachments = {
            m_albedoAttachments[i].view,
            m_heightAttachments[i].view,
            m_normalAttachments[i].view,
            m_rmtAttachments[i].view
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_deferredRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_targetWindow->getSwapchainExtent().width;
        framebufferInfo.height = m_targetWindow->getSwapchainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &m_deferredFramebuffers[i]) != VK_SUCCESS)
            return (false);
    }

    return (true);
}

bool SceneRenderer::createAmbientLightingFramebuffers()
{
    size_t imagesCount = m_targetWindow->getSwapchainSize();

    m_ambientLightingFramebuffers.resize(imagesCount);

    for (size_t i = 0; i < imagesCount ; ++i)
    {
        std::array<VkImageView, 1> attachments = {
            m_hdrAttachements[i].view
        };

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_ambientLightingRenderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_targetWindow->getSwapchainExtent().width;
        framebufferInfo.height = m_targetWindow->getSwapchainExtent().height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &m_ambientLightingFramebuffers[i]) != VK_SUCCESS)
            return (false);
    }

    return (true);
}

bool SceneRenderer::createDeferredRenderPass()
{
     std::array<VkAttachmentDescription, 4> attachments{};

    attachments[0].format = m_albedoAttachments[0].format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[1].format = m_heightAttachments[0].format;
    attachments[1].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[1].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[1].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[1].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[1].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[1].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[1].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL; //VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL

    attachments[2].format = m_normalAttachments[0].format;
    attachments[2].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[2].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[2].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[2].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[2].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[2].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[2].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    attachments[3].format = m_rmtAttachments[0].format;
    attachments[3].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[3].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[3].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[3].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[3].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[3].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[3].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentReference, 3> colorAttachmentRef{};
    colorAttachmentRef[0] = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    colorAttachmentRef[1] = {2, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };
    colorAttachmentRef[2] = {3, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    VkAttachmentReference depthAttachmentRef = {1, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRef.size());
    subpassDescription.pColorAttachments = colorAttachmentRef.data();
    subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    ///Maybe I need dependency to read/write to depth buffer

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount  = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments     = attachments.data();
    renderPassInfo.subpassCount     = 1;
    renderPassInfo.pSubpasses       = &subpassDescription;
    renderPassInfo.dependencyCount  = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies    = dependencies.data();

    return (vkCreateRenderPass(VInstance::device(), &renderPassInfo, nullptr, &m_deferredRenderPass) == VK_SUCCESS);
}

bool SceneRenderer::createAmbientLightingRenderPass()
{
     std::array<VkAttachmentDescription, 1> attachments{};

    attachments[0].format = m_hdrAttachements[0].format;
    attachments[0].samples = VK_SAMPLE_COUNT_1_BIT;
    attachments[0].loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    attachments[0].storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    attachments[0].stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    attachments[0].stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    attachments[0].initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    attachments[0].finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;//VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    std::array<VkAttachmentReference, 1> colorAttachmentRef{};
    colorAttachmentRef[0] = {0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL };

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRef.size());
    subpassDescription.pColorAttachments = colorAttachmentRef.data();

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount  = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments     = attachments.data();
    renderPassInfo.subpassCount     = 1;
    renderPassInfo.pSubpasses       = &subpassDescription;
    renderPassInfo.dependencyCount  = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies    = dependencies.data();

    return (vkCreateRenderPass(VInstance::device(), &renderPassInfo, nullptr, &m_ambientLightingRenderPass) == VK_SUCCESS);
}

bool SceneRenderer::createSemaphores()
{
    size_t framesCount = m_targetWindow->getSwapchainSize();
    VkDevice device = VInstance::device();

    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    m_deferredToAmbientLightingSemaphore.resize(framesCount);
    m_ambientLightingToToneMappingSemaphore.resize(framesCount);

    for(size_t i = 0 ; i < framesCount ; ++i)
    {
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_deferredToAmbientLightingSemaphore[i]) != VK_SUCCESS)
            return (false);
        if(vkCreateSemaphore(device, &semaphoreInfo, nullptr, &m_ambientLightingToToneMappingSemaphore[i]) != VK_SUCCESS)
            return (false);
    }

    return (true);
}

bool SceneRenderer::createDeferredPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << ISOSPRITE_FRAGSHADERFILE;

    m_deferredPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_deferredPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    auto bindingDescription = InstanciedIsoSpriteDatum::getBindingDescription();
    auto attributeDescriptions = InstanciedIsoSpriteDatum::getAttributeDescriptions();
    m_deferredPipeline.setVertexInput(1, &bindingDescription,
                                    attributeDescriptions.size(), attributeDescriptions.data());

    m_deferredPipeline.setInputAssembly(VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP, true);

    m_deferredPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());
    //m_deferredPipeline.setBlendMode(BlendMode_Alpha);

    m_deferredPipeline.attachDescriptorSetLayout(m_renderView.getDescriptorSetLayout());
    m_deferredPipeline.attachDescriptorSetLayout(VTexturesManager::instance()->getDescriptorSetLayout());

    m_deferredPipeline.setDepthTest(true, true, VK_COMPARE_OP_GREATER);

    return m_deferredPipeline.init(m_deferredRenderPass, 0, 3);
}


bool SceneRenderer::createAmbientLightingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << AMBIENTLIGHTING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << AMBIENTLIGHTING_FRAGSHADERFILE;

    m_ambientLightingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_ambientLightingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_ambientLightingPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_ambientLightingPipeline.attachDescriptorSetLayout(m_deferredDescriptorSetLayout);

    return m_ambientLightingPipeline.init(m_ambientLightingRenderPass, 0);
}

bool SceneRenderer::createToneMappingPipeline()
{
    std::ostringstream vertShaderPath,fragShaderPath;
    vertShaderPath << VApp::DEFAULT_SHADERPATH << TONEMAPPING_VERTSHADERFILE;
    fragShaderPath << VApp::DEFAULT_SHADERPATH << TONEMAPPING_FRAGSHADERFILE;

    m_toneMappingPipeline.createShader(vertShaderPath.str(), VK_SHADER_STAGE_VERTEX_BIT);
    m_toneMappingPipeline.createShader(fragShaderPath.str(), VK_SHADER_STAGE_FRAGMENT_BIT);

    m_toneMappingPipeline.setDefaultExtent(m_targetWindow->getSwapchainExtent());

    m_toneMappingPipeline.setBlendMode(BlendMode_None);

    m_toneMappingPipeline.attachDescriptorSetLayout(m_hdrDescriptorSetLayout);

    return m_toneMappingPipeline.init(m_renderPass, 0);
}

bool SceneRenderer::createDeferredCmb()
{
    m_deferredCmb.resize(m_targetWindow->getFramesCount());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VInstance::commandPool(COMMANDPOOL_SHORTLIVED);
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_deferredCmb.size());

    return (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_deferredCmb.data()) == VK_SUCCESS);
}

bool SceneRenderer::createAmbientLightingCmb()
{
    m_ambientLightingCmb.resize(m_targetWindow->getSwapchainSize());

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = VInstance::commandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_ambientLightingCmb.size());

    if(vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_ambientLightingCmb.data()) != VK_SUCCESS)
        return (false);

    for(size_t i = 0 ; i < m_ambientLightingCmb.size() ; ++i)
        this->recordAmbientLightingCmb(i);

    return (true);
}

void SceneRenderer::cleanup()
{
    VkDevice device = VInstance::device();

    m_spritesVbos.clear();

    vkDestroySampler(device, m_attachmentsSampler, nullptr);

    for(auto attachement : m_albedoAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_albedoAttachments.clear();

    for(auto attachement : m_heightAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_heightAttachments.clear();

    for(auto attachement : m_normalAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_normalAttachments.clear();

    for(auto attachement : m_rmtAttachments)
        VulkanHelpers::destroyAttachment(attachement);
    m_rmtAttachments.clear();

    for(auto attachement : m_hdrAttachements)
        VulkanHelpers::destroyAttachment(attachement);
    m_hdrAttachements.clear();

    for(auto framebuffer : m_deferredFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    m_deferredFramebuffers.clear();

    for(auto framebuffer : m_ambientLightingFramebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    m_deferredFramebuffers.clear();

    for(auto semaphore : m_deferredToAmbientLightingSemaphore)
        vkDestroySemaphore(device, semaphore, nullptr);
    m_deferredToAmbientLightingSemaphore.clear();

    for(auto semaphore : m_ambientLightingToToneMappingSemaphore)
        vkDestroySemaphore(device, semaphore, nullptr);
    m_ambientLightingToToneMappingSemaphore.clear();

    vkDestroyRenderPass(device, m_deferredRenderPass, nullptr);
    vkDestroyRenderPass(device, m_ambientLightingRenderPass, nullptr);

    vkDestroyDescriptorPool(device, m_descriptorPool, nullptr);

    vkDestroyDescriptorSetLayout(device, m_deferredDescriptorSetLayout, nullptr);
    vkDestroyDescriptorSetLayout(device, m_hdrDescriptorSetLayout, nullptr);

    m_deferredPipeline.destroy();
    m_ambientLightingPipeline.destroy();
    m_toneMappingPipeline.destroy();

    AbstractRenderer::cleanup();
    //IsoSpriteEntity::cleanupRendering();
}


}
