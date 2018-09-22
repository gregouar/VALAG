#include "Valag/renderers/FullRenderPass.h"

#include "Valag/utils/Logger.h"

namespace vlg
{

FullRenderPass::FullRenderPass(size_t imagesCount, size_t framesCount) :
    m_imagesCount(imagesCount),
    m_cmbCount(0),
    m_extent{0,0},
    m_cmbUsage(0),
    m_renderPass(VK_NULL_HANDLE)
{
    m_attachments.resize(imagesCount);
    m_waitSemaphores.resize(framesCount);
    m_isFinalPass = false;
}

FullRenderPass::~FullRenderPass()
{
    this->destroy();
}

bool FullRenderPass::init()
{
    if(!this->createRenderPass())
        return (false);
    if(!this->createFramebuffers())
        return (false);
    if(!this->createCmb())
        return (false);

    return (true);
}

void FullRenderPass::destroy()
{
    m_cmbCount      = 0;
    m_extent        = {0,0};
    m_cmbUsage      = 0;
    m_isFinalPass   = false;

    m_clearValues.clear();

    VkDevice device = VInstance::device();

    if(m_renderPass != VK_NULL_HANDLE)
        vkDestroyRenderPass(device, m_renderPass, nullptr);
    m_renderPass = VK_NULL_HANDLE;

    for(auto framebuffer : m_framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    m_framebuffers.clear();

    for(auto attachments : m_attachments)
        attachments.clear();

    m_primaryCmb.clear();
    m_waitSemaphoreStages.clear();

    for(size_t i = 0 ; i < m_waitSemaphores.size() ; ++i)
        m_waitSemaphores[i].clear();

    m_signalSemaphores.clear();
}

VkCommandBuffer FullRenderPass::startRecording(size_t imageIndex, size_t frameIndex, VkSubpassContents contents)
{
    m_curRecordingIndex = (m_cmbUsage == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? frameIndex : imageIndex;

    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = m_cmbUsage;

    if (vkBeginCommandBuffer(m_primaryCmb[m_curRecordingIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (VK_NULL_HANDLE);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass   = m_renderPass;
    renderPassInfo.framebuffer  = m_framebuffers[imageIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_extent;

    renderPassInfo.clearValueCount  = static_cast<uint32_t>(m_clearValues.size());
    renderPassInfo.pClearValues     = m_clearValues.data();

    vkCmdBeginRenderPass(m_primaryCmb[m_curRecordingIndex], &renderPassInfo, contents);

    return m_primaryCmb[m_curRecordingIndex];
}

bool FullRenderPass::endRecording()
{
    vkCmdEndRenderPass(m_primaryCmb[m_curRecordingIndex]);

    if (vkEndCommandBuffer(m_primaryCmb[m_curRecordingIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}

void FullRenderPass::addWaitSemaphores(const std::vector<VkSemaphore> &semaphores, VkPipelineStageFlags stage)
{
    for(size_t i = 0 ; i < semaphores.size() ; ++i)
        m_waitSemaphores[i].push_back(semaphores[i]);

    m_waitSemaphoreStages.push_back(stage);
}

void FullRenderPass::setSignalSemaphores(size_t frameIndex, VkSemaphore *semaphore)
{
    if(m_signalSemaphores.size() <= frameIndex)
        m_signalSemaphores.resize(frameIndex + 1,nullptr);
    m_signalSemaphores[frameIndex] = *semaphore;
}

void FullRenderPass::setExtent(VkExtent2D extent)
{
    m_extent = extent;
}

void FullRenderPass::setCmbCount(size_t buffersCount)
{
    m_cmbCount = buffersCount;
}

void FullRenderPass::setCmbUsage(VkFlags usage)
{
    m_cmbUsage = usage;
}

void FullRenderPass::setClearValues(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth)
{
    if(attachmentIndex >= m_clearValues.size())
        m_clearValues.resize(attachmentIndex+1, VkClearValue{});

    m_clearValues[attachmentIndex].color        = {color.r, color.g, color.b, color.a};
    m_clearValues[attachmentIndex].depthStencil = {depth.x, static_cast<uint32_t>(depth.y)};

    /*if(attachmentIndex >= m_clearColors.size())
        m_clearColors.resize(attachmentIndex+1, {0,0,0,0});
    m_clearColors[attachmentIndex] = color;

    if(attachmentIndex >= m_clearDepths.size())
        m_clearDepths.resize(attachmentIndex+1, {0,0});
    m_clearDepths[attachmentIndex] = depth;*/
}

void FullRenderPass::setAttachments(size_t bufferIndex, const std::vector<VFramebufferAttachment> &attachments)
{
    m_attachments[bufferIndex] = attachments;
}

VkFlags FullRenderPass::getCmbUsage()
{
    return m_cmbUsage;
}

VkExtent2D FullRenderPass::getExtent()
{
    return m_extent;
}

VkRenderPass FullRenderPass::getVkRenderPass()
{
    return m_renderPass;
}

bool FullRenderPass::isFinalPass()
{
    return m_isFinalPass;
}

const  std::vector<VkPipelineStageFlags> &FullRenderPass::getWaitSemaphoresStages()
{
    return m_waitSemaphoreStages;
}

const  std::vector<VkSemaphore> &FullRenderPass::getWaitSemaphores(size_t frameIndex)
{
    return m_waitSemaphores[frameIndex];
}

VkSemaphore *FullRenderPass::getSignalSemaphore(size_t frameIndex)
{
    if(m_signalSemaphores.size() <= frameIndex)
        return nullptr;
    return &m_signalSemaphores[frameIndex];
}

const VkCommandBuffer *FullRenderPass::getPrimaryCmb(size_t imageIndex, size_t frameIndex)
{
    size_t cmbIndex = (m_cmbUsage == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? frameIndex : imageIndex;
    return &m_primaryCmb[cmbIndex];
}


/// Protected ///

bool FullRenderPass::createRenderPass()
{
    std::vector<VkAttachmentDescription> attachments(m_attachments[0].size(), VkAttachmentDescription{});


    std::vector<VkAttachmentReference> colorAttachmentRef;

    bool hasDepthAttachment = false;
    VkAttachmentReference depthAttachmentRef{};

    for(size_t i = 0 ; i < attachments.size() ; ++i)
    {
        VkImageLayout layout = m_attachments[0][i].layout;

        ///Could add verification here
        attachments[i].format = m_attachments[0][i].format;

        attachments[i].samples          = VK_SAMPLE_COUNT_1_BIT;
        attachments[i].loadOp           = VK_ATTACHMENT_LOAD_OP_CLEAR;
        attachments[i].storeOp          = VK_ATTACHMENT_STORE_OP_STORE;
        attachments[i].stencilLoadOp    = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
        attachments[i].stencilStoreOp   = VK_ATTACHMENT_STORE_OP_DONT_CARE;
        attachments[i].initialLayout    = VK_IMAGE_LAYOUT_UNDEFINED;

        if(layout == VK_IMAGE_LAYOUT_PRESENT_SRC_KHR)
        {
            attachments[i].finalLayout      = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;
            m_isFinalPass = true;
        }
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

    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_MEMORY_READ_BIT;
    dependencies[0].dstAccessMask = /*VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |*/ VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    ///Maybe I need dependency to read/write to depth buffer

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT;
    dependencies[1].srcAccessMask = /*VK_ACCESS_COLOR_ATTACHMENT_READ_BIT |*/ VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
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

    return (vkCreateRenderPass(VInstance::device(), &renderPassInfo, nullptr, &m_renderPass) == VK_SUCCESS);
}

bool FullRenderPass::createFramebuffers()
{
    m_framebuffers.resize(m_imagesCount);

    for (size_t i = 0; i < m_imagesCount ; ++i)
    {
        std::vector<VkImageView> attachments(m_attachments[i].size());

        for(size_t j = 0 ; j < attachments.size() ; ++j)
            attachments[j] = m_attachments[i][j].view;

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments    = attachments.data();
        framebufferInfo.width           = m_extent.width;
        framebufferInfo.height          = m_extent.height;
        framebufferInfo.layers          = 1;

        if (vkCreateFramebuffer(VInstance::device(), &framebufferInfo, nullptr, &m_framebuffers[i]) != VK_SUCCESS)
            return (false);
    }

    if(m_clearValues.size() < m_attachments[0].size())
        m_clearValues.resize(m_attachments[0].size(), VkClearValue{});

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
}

}
