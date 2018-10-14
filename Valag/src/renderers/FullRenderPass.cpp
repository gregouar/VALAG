#include "Valag/renderers/FullRenderPass.h"

#include "Valag/utils/Logger.h"

namespace vlg
{

FullRenderPass::FullRenderPass(size_t imagesCount, size_t framesCount) :
    m_imagesCount(imagesCount),
    m_cmbCount(0),
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
    if(!this->createDescriptorSetLayout())
        return (false);
    if(!this->createDescriptorSets(pool, sampler))
        return (false);

    return (true);
}

void FullRenderPass::destroy()
{
    m_cmbCount      = 0;
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
    size_t cmbIndex = (m_renderTarget.getCmbUsage() == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? frameIndex : imageIndex;

    return m_renderTarget.startRecording(cmbIndex, imageIndex, contents);

    /*VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = m_cmbUsage;

    if (vkBeginCommandBuffer(m_primaryCmb[m_curRecordingIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (VK_NULL_HANDLE);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass   = m_renderPass.getVkRenderPass();
    renderPassInfo.framebuffer  = m_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_extent;

    renderPassInfo.clearValueCount  = static_cast<uint32_t>(m_clearValues.size());
    renderPassInfo.pClearValues     = m_clearValues.data();

    vkCmdBeginRenderPass(m_primaryCmb[m_curRecordingIndex], &renderPassInfo, contents);

    return m_primaryCmb[m_curRecordingIndex];*/
}

bool FullRenderPass::endRecording()
{
    /*vkCmdEndRenderPass(m_primaryCmb[m_curRecordingIndex]);

    if (vkEndCommandBuffer(m_primaryCmb[m_curRecordingIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }*/

    return m_renderTarget.endRecording();
}

/*void FullRenderPass::addWaitSemaphores(const std::vector<VkSemaphore> &semaphores, VkPipelineStageFlags stage)
{
    for(size_t i = 0 ; i < semaphores.size() ; ++i)
        m_waitSemaphores[i].push_back(semaphores[i]);

    m_waitSemaphoreStages.push_back(stage);
}

void FullRenderPass::setSignalSemaphores(size_t frameIndex, VkSemaphore semaphore)
{
    if(m_signalSemaphores.size() <= frameIndex)
        m_signalSemaphores.resize(frameIndex + 1,VK_NULL_HANDLE);
    m_signalSemaphores[frameIndex] = semaphore;
}*/

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

void FullRenderPass::setCmbCount(size_t buffersCount)
{
    m_cmbCount = buffersCount;
}

void FullRenderPass::setCmbUsage(VkFlags usage)
{
    m_renderTarget.setCmbUsage(usage);
}

void FullRenderPass::setClearValues(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth)
{
    /*if(attachmentIndex >= m_clearValues.size())
        m_clearValues.resize(attachmentIndex+1, VkClearValue{});

    m_clearValues[attachmentIndex].color        = {color.r, color.g, color.b, color.a};
    m_clearValues[attachmentIndex].depthStencil = {depth.x, static_cast<uint32_t>(depth.y)};*/
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
    //m_attachmentsLoadOp.push_back({loadOp, fromUniform});
    //m_attachmentsStoreOp.push_back({storeOp, false});
    m_renderTarget.addAttachments(attachments);
    //m_attachments.push_back(attachments);
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
    //m_attachmentsLoadOp[attachmentIndex] = {loadOp, fromUniform};
}

void FullRenderPass::setAttachmentsStoreOp(size_t attachmentIndex, VkAttachmentStoreOp storeOp, bool toUniform)
{
    m_renderPass.setAttachmentsStoreOp(attachmentIndex, storeOp, toUniform);
    //m_attachmentsStoreOp[attachmentIndex] = {storeOp, toUniform};
}


VkFlags FullRenderPass::getCmbUsage()
{
    return m_renderTarget.getCmbUsage();
}

VkExtent2D FullRenderPass::getExtent()
{
    return m_renderTarget.getExtent();
}

VkRenderPass FullRenderPass::getVkRenderPass()
{
    return m_renderPass.getVkRenderPass();
}

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
    /*if(attachmentsIndex >= m_attachments.size())
        throw std::runtime_error("Cannot get attachment");
    return m_attachments[attachmentsIndex];*/
    return m_renderTarget.getAttachments(attachmentsIndex);
}

/*VkSemaphore FullRenderPass::getSignalSemaphore(size_t frameIndex)
{
    if(m_signalSemaphores.size() <= frameIndex)
        return VK_NULL_HANDLE;
    return m_signalSemaphores[frameIndex];
}*/

const VkCommandBuffer *FullRenderPass::getPrimaryCmb(size_t imageIndex, size_t frameIndex)
{
    size_t cmbIndex = (m_renderTarget.getCmbUsage() == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? frameIndex : imageIndex;
    return m_renderTarget.getPrimaryCmb(cmbIndex);
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

bool FullRenderPass::createRenderPass()
{
    /*for(auto attachment : m_renderTarget.getAttachments())
        if(attachment.front().type.layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            m_isFinalPass = true;*/

    return m_renderPass.init();

    /*std::vector<VkAttachmentDescription> attachments(m_attachments.size(), VkAttachmentDescription{});

    std::vector<VkAttachmentReference> colorAttachmentRef;

    bool hasDepthAttachment = false;
    VkAttachmentReference depthAttachmentRef{};

    for(size_t i = 0 ; i < attachments.size() ; ++i)
    {
        VkImageLayout layout = m_attachments[i][0].type.layout;

        ///Could add verification here
        attachments[i].format = m_attachments[i][0].type.format;

        attachments[i].samples          = VK_SAMPLE_COUNT_1_BIT;
        attachments[i].loadOp           = m_attachmentsLoadOp[i].first;
        attachments[i].storeOp          = m_attachmentsStoreOp[i].first;//VK_ATTACHMENT_STORE_OP_STORE;

        ///I'll need to update this
        attachments[i].stencilLoadOp    = m_attachmentsLoadOp[i].first;///VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[i].stencilStoreOp   = m_attachmentsStoreOp[i].first;///VK_ATTACHMENT_STORE_OP_DONT_CARE;


        ///Could change this to be smarter
        if(m_attachmentsLoadOp[i].first == VK_ATTACHMENT_LOAD_OP_LOAD)
        {
            if(m_attachmentsLoadOp[i].second)
                attachments[i].initialLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            else
                attachments[i].initialLayout = layout;
        }
        else
            attachments[i].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;

        if(layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            attachments[i].finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            m_isFinalPass = true;
        }
        else if(m_attachmentsStoreOp[i].first == VK_ATTACHMENT_STORE_OP_STORE && !m_attachmentsStoreOp[i].second)
             attachments[i].finalLayout     = layout;
        else
            attachments[i].finalLayout      = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        if(layout == VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL || layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
            colorAttachmentRef.push_back({static_cast<uint32_t>(i),
                                         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL});
        if(layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
        {
            depthAttachmentRef = {static_cast<uint32_t>(i),
            VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL};
            hasDepthAttachment = true;
        }
    }

    VkSubpassDescription subpassDescription = {};
    subpassDescription.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpassDescription.colorAttachmentCount = static_cast<uint32_t>(colorAttachmentRef.size());
    subpassDescription.pColorAttachments = colorAttachmentRef.data();

    if(hasDepthAttachment)
        subpassDescription.pDepthStencilAttachment = &depthAttachmentRef;

    std::array<VkSubpassDependency, 4> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask =  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[2].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[2].dstSubpass = 0;
    dependencies[2].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[2].dstStageMask = VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependencies[2].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[2].dstAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[2].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    ///Maybe I need dependency to read/write to depth buffer

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask =  VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[3].srcSubpass = 0;
    dependencies[3].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[3].srcStageMask = VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT;
    dependencies[3].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[3].srcAccessMask = VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT  | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    dependencies[3].dstAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[3].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount  = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments     = attachments.data();
    renderPassInfo.subpassCount     = 1;
    renderPassInfo.pSubpasses       = &subpassDescription;
    renderPassInfo.dependencyCount  = static_cast<uint32_t>(dependencies.size());
    renderPassInfo.pDependencies    = dependencies.data();

    return (vkCreateRenderPass(VInstance::device(), &renderPassInfo, nullptr, &m_vkRenderPass) == VK_SUCCESS);*/
}


bool FullRenderPass::createRenderTarget()
{
    return m_renderTarget.init(m_imagesCount, m_cmbCount, &m_renderPass);
}

/*bool FullRenderPass::createFramebuffers()
{
    m_framebuffers.resize(m_imagesCount);

    for (size_t i = 0; i < m_imagesCount ; ++i)
    {
        std::vector<VkImageView> attachments(m_attachments.size());

        for(size_t j = 0 ; j < attachments.size() ; ++j)
            attachments[j] = m_attachments[j][i].view;

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = m_renderPass.getVkRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = m_extent.width;
        framebufferInfo.height          = m_extent.height;
        framebufferInfo.layers          = 1;

        if (vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            return (false);
    }

    if(m_clearValues.size() < m_attachments.size())
        m_clearValues.resize(m_attachments.size(), VkClearValue{});

    return (true);
}

bool FullRenderPass::createCmb()
{
    m_primaryCmb.resize(m_cmbCount);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandPoolName pool    = (m_cmbUsage == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? COMMANDPOOL_SHORTLIVED : COMMANDPOOL_DEFAULT;
    allocInfo.commandPool   = VInstance::commandPool(pool);
    allocInfo.level         = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_primaryCmb.size());

    return (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_primaryCmb.data()) == VK_SUCCESS);
}*/

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
