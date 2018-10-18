#include "Valag/renderers/RenderGraph.h"

#include "Valag/utils/Logger.h"

namespace vlg
{

RenderGraph::RenderGraph(size_t imagesCount, size_t framesCount) :
    m_imagesCount(imagesCount),
    m_framesCount(framesCount),
    m_descriptorPool(VK_NULL_HANDLE),
    m_sampler(VK_NULL_HANDLE)
{
    //ctor
}

RenderGraph::~RenderGraph()
{
    this->destroy();
}

bool RenderGraph::init()
{
    if(!this->createSemaphores())
        return (false);

    if(!this->createDescriptorPool())
        return (false);

    if(!this->createSampler())
        return (false);

    if(!this->initRenderPasses())
        return (false);

    return (true);
}

void RenderGraph::destroy()
{
    for(auto renderPass : m_renderPasses)
        delete renderPass;
    m_renderPasses.clear();

    m_connexions.clear();

    for(auto semaphore : m_semaphores)
        vkDestroySemaphore(VInstance::device(), semaphore, nullptr);
    m_semaphores.clear();

    m_descriptorPoolSizes.clear();
    if(m_descriptorPool != VK_NULL_HANDLE)
        vkDestroyDescriptorPool(VInstance::device(), m_descriptorPool, nullptr);
    m_descriptorPool = VK_NULL_HANDLE;

    if(m_sampler != VK_NULL_HANDLE)
        vkDestroySampler(VInstance::device(), m_sampler, nullptr);
    m_sampler = VK_NULL_HANDLE;
}

void RenderGraph::setDefaultExtent(VkExtent2D extent)
{
    m_defaultExtent = extent;
}

size_t RenderGraph::addRenderPass(VkFlags usage)
{
    m_renderPasses.push_back(new FullRenderPass(m_imagesCount, m_framesCount));
    m_renderPasses.back()->setCmbUsage(usage);
    return m_renderPasses.size()-1;
}

size_t RenderGraph::addDynamicRenderPass()
{
    m_renderPasses.push_back(new FullRenderPass(m_imagesCount, m_framesCount, true));
    m_renderPasses.back()->setCmbUsage(VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
    return m_renderPasses.size()-1;
}

void RenderGraph::connectRenderPasses(size_t src, size_t dst)
{
    if(src >= m_renderPasses.size()
    || dst >= m_renderPasses.size())
    {
        std::ostringstream errorReport;
        errorReport<<"Can not connect render pass "<<src<<" with "<<dst;
        Logger::error(errorReport);
        return;
    }
    m_connexions.insert({src, dst});
}


void RenderGraph::addAttachmentType(size_t renderPassIndex, const VFramebufferAttachmentType &type,
                                    VkAttachmentStoreOp storeOp, VkAttachmentLoadOp loadOp)
{
    m_renderPasses[renderPassIndex]->addAttachmentType(type,storeOp,loadOp,true);
}

void RenderGraph::addNewAttachments(size_t renderPassIndex, const VFramebufferAttachment &attachment,
                                 VkAttachmentStoreOp storeOp, VkAttachmentLoadOp loadOp)
{
    std::vector<VFramebufferAttachment> attachments(m_imagesCount,attachment);
    this->addNewAttachments(renderPassIndex, attachments, storeOp, loadOp);
}

void RenderGraph::addNewAttachments(size_t renderPassIndex, const std::vector<VFramebufferAttachment> &attachments,
                                 VkAttachmentStoreOp storeOp, VkAttachmentLoadOp loadOp)
{
    m_renderPasses[renderPassIndex]->addAttachments(attachments, storeOp, loadOp, true);
}

void RenderGraph::addNewUniforms(size_t renderPassIndex, const std::vector<VBuffer> &buffers)
{
     m_descriptorPoolSizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, static_cast<uint32_t>(m_imagesCount)});
     m_renderPasses[renderPassIndex]->addUniforms(buffers);
}

void RenderGraph::addNewUniforms(size_t renderPassIndex, const std::vector<VkImageView> &views)
{
     m_descriptorPoolSizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, static_cast<uint32_t>(m_imagesCount)});
     m_renderPasses[renderPassIndex]->addUniforms(views);
}

void RenderGraph::transferAttachmentsToAttachments(size_t srcRenderPass, size_t dstRenderPass, size_t attachmentsIndex,
                                                    VkAttachmentStoreOp storeOp)
{
    auto attachements = m_renderPasses[srcRenderPass]->getAttachments(attachmentsIndex);
    m_renderPasses[srcRenderPass]->setAttachmentsStoreOp(attachmentsIndex, VK_ATTACHMENT_STORE_OP_STORE);
    m_renderPasses[dstRenderPass]->addAttachments(attachements,storeOp,VK_ATTACHMENT_LOAD_OP_LOAD);
    //this->connectRenderPasses(srcRenderPass, dstRenderPass);
}

void RenderGraph::transferAttachmentsToUniforms(size_t srcRenderPass, size_t dstRenderPass, size_t attachmentsIndex)
{
    auto attachements = m_renderPasses[srcRenderPass]->getAttachments(attachmentsIndex);
    m_renderPasses[srcRenderPass]->setAttachmentsStoreOp(attachmentsIndex, VK_ATTACHMENT_STORE_OP_STORE, true);
    m_descriptorPoolSizes.push_back(VkDescriptorPoolSize{VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, static_cast<uint32_t>(m_imagesCount)});
    m_renderPasses[dstRenderPass]->addUniforms(attachements);
    //this->connectRenderPasses(srcRenderPass, dstRenderPass);
}

void RenderGraph::setClearValue(size_t renderPassIndex, size_t attachmentIndex, glm::vec4 color, glm::vec2 depth)
{
    m_renderPasses[renderPassIndex]->setClearValues(attachmentIndex, color, depth);
}

/*VkRenderPass RenderGraph::getVkRenderPass(size_t renderPassIndex)
{
    return m_renderPasses[renderPassIndex]->getVkRenderPass();
}*/

VRenderPass *RenderGraph::getRenderPass(size_t renderPassIndex)
{
    return m_renderPasses[renderPassIndex]->getRenderPass();
}

VkDescriptorSetLayout RenderGraph::getDescriptorLayout(size_t renderPassIndex)
{
    return m_renderPasses[renderPassIndex]->getDescriptorLayout();
}

VkDescriptorSet RenderGraph::getDescriptorSet(size_t renderPassIndex, size_t imageIndex)
{
    return m_renderPasses[renderPassIndex]->getDescriptorSet(imageIndex);
}

/*size_t RenderGraph::getColorAttachmentsCount(size_t renderPassIndex)
{
    return m_renderPasses[renderPassIndex]->getColorAttachmentsCount();
}*/

VkCommandBuffer RenderGraph::startRecording(size_t renderPassIndex, size_t imageIndex, size_t frameIndex, VkSubpassContents contents)
{
     return m_renderPasses[renderPassIndex]->startRecording(imageIndex, frameIndex, contents);
}

bool RenderGraph::nextRenderTarget(size_t renderPassIndex, VkSubpassContents contents)
{
    return m_renderPasses[renderPassIndex]->nextRenderTarget(contents);
}

bool RenderGraph::endRecording(size_t renderPassIndex)
{
    return m_renderPasses[renderPassIndex]->endRecording();
}

std::vector<FullRenderPass*> RenderGraph::submitToGraphicsQueue(size_t imageIndex, size_t frameIndex)
{
    std::vector<FullRenderPass*> finalRenderPasses;
    std::vector<VkSubmitInfo> submitInfos;

    for(auto renderPass : m_renderPasses)
    {
        if(renderPass->isFinalPass())
        {
            finalRenderPasses.push_back(renderPass);
        }
        else if(renderPass->needToSubmit())
        {
            VkSubmitInfo submitInfo{};
            submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
            submitInfo.waitSemaphoreCount   = /*0;//*/static_cast<uint32_t>(renderPass->getWaitSemaphores(frameIndex).size());
            submitInfo.pWaitDstStageMask    = renderPass->getWaitSemaphoresStages().data();
            submitInfo.pWaitSemaphores      = renderPass->getWaitSemaphores(frameIndex).data();

            submitInfo.signalSemaphoreCount   = /*0;//*/static_cast<uint32_t>(renderPass->getSignalSemaphores(frameIndex).size());
            submitInfo.pSignalSemaphores      = renderPass->getSignalSemaphores(frameIndex).data();

            submitInfo.commandBufferCount   = 1;
            submitInfo.pCommandBuffers      = renderPass->getPrimaryCmb(imageIndex, frameIndex);

            submitInfos.push_back(submitInfo);
        }
    }

    VInstance::submitToGraphicsQueue(submitInfos, 0);

    return finalRenderPasses;
}



///Protected ///

bool RenderGraph::initRenderPasses()
{
    for(auto renderPass : m_renderPasses)
    {
        /*size_t bufferCount =
           (renderPass->getCmbUsage() == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? m_framesCount : m_imagesCount;
        renderPass->setCmbCount(bufferCount);*/

        if(renderPass->getExtent().width == 0)
            renderPass->setExtent(m_defaultExtent);

        if(!renderPass->init(m_descriptorPool, m_sampler))
            return (false);
    }

    return (true);
}

bool RenderGraph::createSemaphores()
{
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    for(auto &connexion : m_connexions)
    {
        for(size_t i = 0 ; i < m_framesCount ; ++i)
        {
            VkSemaphore semaphore;
            if(vkCreateSemaphore(VInstance::device(), &semaphoreInfo, nullptr, &semaphore) != VK_SUCCESS)
                return (false);
            m_renderPasses[connexion.first]->addSignalSemaphore(i,semaphore);
            m_renderPasses[connexion.second]->addWaitSemaphore(i,semaphore, VK_PIPELINE_STAGE_ALL_COMMANDS_BIT);
            m_semaphores.push_back(semaphore);
        }
    }

    return (true);
}

bool RenderGraph::createDescriptorPool()
{
    if(m_descriptorPoolSizes.empty())
        return (true);

    VkDescriptorPoolCreateInfo poolInfo = {};
    poolInfo.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    poolInfo.poolSizeCount = static_cast<uint32_t>(m_descriptorPoolSizes.size());
    poolInfo.pPoolSizes = m_descriptorPoolSizes.data();

    poolInfo.maxSets = m_imagesCount * m_descriptorPoolSizes.size();

    return (vkCreateDescriptorPool(VInstance::device(), &poolInfo, nullptr, &m_descriptorPool) == VK_SUCCESS);
}

bool RenderGraph::createSampler()
{
    VkSamplerCreateInfo samplerInfo = {};
    samplerInfo.sType        = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
    samplerInfo.magFilter   = VK_FILTER_NEAREST;
    samplerInfo.minFilter   = VK_FILTER_NEAREST;
    samplerInfo.mipmapMode  = VK_SAMPLER_MIPMAP_MODE_NEAREST;//VK_SAMPLER_MIPMAP_MODE_LINEAR;
    samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
    samplerInfo.addressModeV = samplerInfo.addressModeU;
    samplerInfo.addressModeW = samplerInfo.addressModeU;
    samplerInfo.mipLodBias  = 0.0f;
    samplerInfo.maxAnisotropy = 1.0f;
    samplerInfo.minLod = 0.0f;
    samplerInfo.maxLod = 0.0f;//1.0f;
    samplerInfo.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;

    samplerInfo.unnormalizedCoordinates = VK_TRUE;//VK_FALSE;
    samplerInfo.compareEnable = VK_FALSE;
    samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
    samplerInfo.anisotropyEnable = VK_FALSE;

    return (vkCreateSampler(VInstance::device(), &samplerInfo, nullptr, &m_sampler) == VK_SUCCESS);
}

}
