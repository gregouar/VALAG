#include "Valag/renderers/FullRenderPass.h"

#include "Valag/utils/Logger.h"

namespace vlg
{

FullRenderPass::FullRenderPass(size_t imagesCount, size_t framesCount) :
    m_imagesCount(imagesCount),
    m_framesCount(framesCount),
    //m_extent{0,0},
    //m_cmbUsage(0),
    //m_renderPass(nullptr),
    m_curUniformBinding(0),
    m_descriptorSetLayout(VK_NULL_HANDLE)
{
    m_waitSemaphores.resize(framesCount);
    m_signalSemaphores.resize(framesCount);
    m_descriptorSets.resize(imagesCount);
    m_isFinalPass = false;
}

FullRenderPass::~FullRenderPass()
{
    this->destroy();
}

bool FullRenderPass::init(VkDescriptorPool pool, VkSampler sampler)
{
    if(!this->createRenderPass())
        return (false);
    if(!this->createRenderTarget())
        return (false);
    /*if(!this->createFramebuffers())
        return (false);
    if(!this->createCmb())
        return (false);*/
    if(!this->createCommandBuffers())
        return (false);
    if(!this->createDescriptorSetLayout())
        return (false);
    if(!this->createDescriptorSets(pool, sampler))
        return (false);

    return (true);
}

void FullRenderPass::destroy()
{
    //m_cmbCount      = 1;
    //m_extent        = {0,0};
    //m_cmbUsage      = 0;
    m_isFinalPass   = false;

    //m_clearValues.clear();

    VkDevice device = VInstance::device();

    m_renderTarget.destroy();
    m_renderPass.destroy();

    if(m_descriptorSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(device, m_descriptorSetLayout, nullptr);
    m_descriptorSetLayout = VK_NULL_HANDLE;

    m_curUniformBinding = 0;
    m_uniformAttachments.clear();
    m_uniformBuffers.clear();
    m_uniformViews.clear();

    /*for(auto framebuffer : m_framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    m_framebuffers.clear();*/

    //m_attachmentsLoadOp.clear();
    //m_attachmentsStoreOp.clear();
    //m_attachments.clear();

    //m_primaryCmb.clear();
    m_waitSemaphoreStages.clear();

    for(size_t i = 0 ; i < m_waitSemaphores.size() ; ++i)
        m_waitSemaphores[i].clear();

    for(size_t i = 0 ; i < m_signalSemaphores.size() ; ++i)
        m_signalSemaphores[i].clear();
}

VkCommandBuffer FullRenderPass::startRecording(size_t imageIndex, size_t frameIndex, VkSubpassContents contents)
{
    size_t cmbIndex = (m_cmb.getCmbUsage() == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? frameIndex : imageIndex;

    //return m_renderTarget.startRecording(cmbIndex, imageIndex, contents);
    return m_cmb.startRecording(cmbIndex, imageIndex,  contents, &m_renderPass, &m_renderTarget);
}

bool FullRenderPass::endRecording()
{
    //return m_renderTarget.endRecording();
    return m_cmb.endRecording();
}

void FullRenderPass::addWaitSemaphore(size_t frameIndex, VkSemaphore semaphore, VkPipelineStageFlags stage)
{
    m_waitSemaphores[frameIndex].push_back(semaphore);
    m_waitSemaphoreStages.push_back(stage);
}

void FullRenderPass::addSignalSemaphore(size_t frameIndex, VkSemaphore semaphore)
{
    m_signalSemaphores[frameIndex].push_back(semaphore);
}

void FullRenderPass::setExtent(VkExtent2D extent)
{
    //m_extent = extent;
    m_renderTarget.setExtent(extent);
}

/*void FullRenderPass::setCmbCount(size_t buffersCount)
{
    m_cmbCount = buffersCount;
}*/

void FullRenderPass::setCmbUsage(VkFlags usage)
{
    m_cmb.setCmbUsage(usage);
}

void FullRenderPass::setClearValues(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth)
{
    m_renderTarget.setClearValue(attachmentIndex, color, depth);
}

/*void FullRenderPass::setAttachments(size_t bufferIndex, const std::vector<VFramebufferAttachment> &attachments)
{
    m_attachments[bufferIndex] = attachments;
}*/

void FullRenderPass::addAttachments(const std::vector<VFramebufferAttachment> &attachments,
                                    VkAttachmentStoreOp storeOp, VkAttachmentLoadOp loadOp,
                                    bool fromUniform)
{
    m_renderPass.addAttachmentType(attachments.front().type, storeOp, false, loadOp, fromUniform);

    if(attachments.front().type.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        m_isFinalPass = true;

    m_renderTarget.addAttachments(attachments);
}

void FullRenderPass::addUniforms(const std::vector<VFramebufferAttachment> &attachments)
{
    m_uniformAttachments.push_back({m_curUniformBinding++, attachments});
}

void FullRenderPass::addUniforms(const std::vector<VBuffer> &buffers)
{
    m_uniformBuffers.push_back({m_curUniformBinding++, buffers});
}

void FullRenderPass::addUniforms(const std::vector<VkImageView> &views)
{
    m_uniformViews.push_back({m_curUniformBinding++, views});
}

void FullRenderPass::setAttachmentsLoadOp(size_t attachmentIndex, VkAttachmentLoadOp loadOp, bool fromUniform)
{
    m_renderPass.setAttachmentsLoadOp(attachmentIndex, loadOp, fromUniform);
}

void FullRenderPass::setAttachmentsStoreOp(size_t attachmentIndex, VkAttachmentStoreOp storeOp, bool toUniform)
{
    m_renderPass.setAttachmentsStoreOp(attachmentIndex, storeOp, toUniform);
}


VkExtent2D FullRenderPass::getExtent()
{
    return m_renderTarget.getExtent();
}

/*VkFlags FullRenderPass::getCmbUsage()
{
    return m_cmb.getCmbUsage();
}

VkRenderPass FullRenderPass::getVkRenderPass()
{
    return m_renderPass.getVkRenderPass();
}*/

const VRenderPass *FullRenderPass::getRenderPass()
{
    return &m_renderPass;
}

bool FullRenderPass::isFinalPass()
{
    return m_isFinalPass;
}

//size_t FullRenderPass::getColorAttachmentsCount()
//{
    /*size_t c = 0;

    for(auto &attachment : m_attachments)
        if(attachment[0].type.layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
        || attachment[0].type.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            c++;

    return c;*/

 //   return m_renderPass.getColorAttachmentsCount();
//}

const  std::vector<VkPipelineStageFlags> &FullRenderPass::getWaitSemaphoresStages()
{
    return m_waitSemaphoreStages;
}

const  std::vector<VkSemaphore> &FullRenderPass::getWaitSemaphores(size_t frameIndex)
{
    return m_waitSemaphores[frameIndex];
}

const  std::vector<VkSemaphore> &FullRenderPass::getSignalSemaphores(size_t frameIndex)
{
    return m_signalSemaphores[frameIndex];
}

const  std::vector<VFramebufferAttachment> &FullRenderPass::getAttachments(size_t attachmentsIndex)
{
    return m_renderTarget.getAttachments(attachmentsIndex);
}

const VkCommandBuffer *FullRenderPass::getPrimaryCmb(size_t imageIndex, size_t frameIndex)
{
    size_t cmbIndex = (m_cmb.getCmbUsage() == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? frameIndex : imageIndex;
    return m_cmb.getVkCommandBuffer(cmbIndex);
    //return &m_primaryCmb[cmbIndex];
}

VkDescriptorSetLayout FullRenderPass::getDescriptorLayout()
{
    return m_descriptorSetLayout;
}

VkDescriptorSet FullRenderPass::getDescriptorSet(size_t imageIndex)
{
    return m_descriptorSets[imageIndex];
}


/// Protected ///

bool FullRenderPass::createCommandBuffers()
{
    size_t buffersCount =
       (m_cmb.getCmbUsage() == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? m_framesCount : m_imagesCount;

    return m_cmb.init(buffersCount);
}

bool FullRenderPass::createRenderPass()
{
    /*for(auto attachment : m_renderTarget.getAttachments())
        if(attachment.front().type.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            m_isFinalPass = true;*/

    return m_renderPass.init();
}


bool FullRenderPass::createRenderTarget()
{
    return m_renderTarget.init(m_imagesCount, &m_renderPass);
}

bool FullRenderPass::createDescriptorSetLayout()
{
    if(m_curUniformBinding == 0)
        return (true);

    std::vector<VkDescriptorSetLayoutBinding> layoutBindings;

    layoutBindings.resize(m_curUniformBinding, {});

    size_t i = 0;
    for(auto uniform : m_uniformAttachments)
    {
        layoutBindings[i].binding           = uniform.first;
        layoutBindings[i].descriptorCount   = 1;
        layoutBindings[i].stageFlags        = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[i].descriptorType    = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        i++;
    }

    for(auto uniform : m_uniformBuffers)
    {
        layoutBindings[i].binding           = uniform.first;
        layoutBindings[i].descriptorCount   = 1;
        layoutBindings[i].stageFlags        = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[i].descriptorType    = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        i++;
    }

    for(auto uniform : m_uniformViews)
    {
        layoutBindings[i].binding           = uniform.first;
        layoutBindings[i].descriptorCount   = 1;
        layoutBindings[i].stageFlags        = VK_SHADER_STAGE_FRAGMENT_BIT;
        layoutBindings[i].descriptorType    = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
        i++;
    }

    VkDescriptorSetLayoutCreateInfo layoutInfo = {};
    layoutInfo.sType        = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
    layoutInfo.bindingCount = static_cast<uint32_t>(layoutBindings.size());
    layoutInfo.pBindings    = layoutBindings.data();

    return (vkCreateDescriptorSetLayout(VInstance::device(), &layoutInfo, nullptr, &m_descriptorSetLayout) == VK_SUCCESS);
}

bool FullRenderPass::createDescriptorSets(VkDescriptorPool pool, VkSampler sampler)
{
    if(m_curUniformBinding == 0)
        return (true);

    VkDevice device = VInstance::device();

    std::vector<VkDescriptorSetLayout> layouts(m_descriptorSets.size(), m_descriptorSetLayout);
    VkDescriptorSetAllocateInfo allocInfo = {};
    allocInfo.sType                 = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
    allocInfo.descriptorPool        = pool;
    allocInfo.descriptorSetCount    = static_cast<uint32_t>(m_descriptorSets.size());
    allocInfo.pSetLayouts           = layouts.data();

    if (vkAllocateDescriptorSets(device, &allocInfo,m_descriptorSets.data()) != VK_SUCCESS)
        return (false);

    for (size_t i = 0; i < m_descriptorSets.size() ; ++i)
    {
        std::vector<VkWriteDescriptorSet>    descriptorWrites(m_curUniformBinding);
        std::vector<VkDescriptorImageInfo>   imageInfos(m_curUniformBinding);
        std::vector<VkDescriptorBufferInfo>  bufferInfos(m_curUniformBinding);

        for(size_t j = 0 ; j < m_curUniformBinding ; ++j)
        {
            descriptorWrites[j] = {};
            descriptorWrites[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            descriptorWrites[j].dstArrayElement = 0;
            descriptorWrites[j].descriptorCount = 1;
            descriptorWrites[j].pImageInfo      = &imageInfos[j];
            descriptorWrites[j].pBufferInfo     = &bufferInfos[j];
            descriptorWrites[j].dstSet          = m_descriptorSets[i];

            imageInfos[j]  = {};
            bufferInfos[j] = {};
        }

        size_t j = 0;
        for(auto uniform : m_uniformAttachments)
        {
            imageInfos[j].imageView     = uniform.second[i].view;
            imageInfos[j].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            imageInfos[j].sampler       = sampler;

            descriptorWrites[j].descriptorType  = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            descriptorWrites[j].dstBinding      = uniform.first;
            j++;
        }
        for(auto uniform : m_uniformBuffers)
        {
            bufferInfos[j].buffer = uniform.second[i].buffer;
            bufferInfos[j].offset = uniform.second[i].offset;
            bufferInfos[j].range  = uniform.second[i].alignedSize;

            descriptorWrites[j].descriptorType  = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            descriptorWrites[j].dstBinding      = uniform.first;
            j++;
        }
        for(auto uniform : m_uniformViews)
        {
            imageInfos[j].imageView     = uniform.second[i];
            imageInfos[j].imageLayout   = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            //imageInfos[j].sampler       = sampler;

            descriptorWrites[j].descriptorType  = VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE;
            descriptorWrites[j].dstBinding      = uniform.first;
            j++;
        }

        vkUpdateDescriptorSets(device, static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
    }

    return (true);
}

}
