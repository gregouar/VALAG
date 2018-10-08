#include "Valag/renderers/RenderTarget.h"

#include "Valag/utils/Logger.h"

namespace vlg
{

RenderTarget::RenderTarget() :
    m_imagesCount(1),
    m_extent({0,0}),
    m_cmbUsage(0),
    m_usedRenderPass(nullptr),
    m_curRecordingIndex(0)
{
    //ctor
}

RenderTarget::~RenderTarget()
{
    //dtor
}

bool RenderTarget::init(size_t framebuffersCount, size_t cmbCount, VRenderPass *renderPass)
{
    m_usedRenderPass = renderPass;

    if(!this->createFramebuffers(framebuffersCount))
        return (false);
    if(!this->createCmb(cmbCount))
        return (false);

    return (true);
}

void RenderTarget::destroy()
{
    VkDevice device = VInstance::device();

    m_clearValues.clear();
    for(auto framebuffer : m_framebuffers)
        vkDestroyFramebuffer(device, framebuffer, nullptr);
    m_framebuffers.clear();

    m_primaryCmb.clear();

    m_usedRenderPass = nullptr;
}

void RenderTarget::addAttachments(const std::vector<VFramebufferAttachment> &attachments)
{
    m_attachments.push_back(attachments);
}

void RenderTarget::createAttachments(VFramebufferAttachmentType type)
{
    /** do something **/
}

VkCommandBuffer RenderTarget::startRecording(size_t cmbIndex, size_t framebufferIndex, VkSubpassContents contents)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = m_cmbUsage;

    if (vkBeginCommandBuffer(m_primaryCmb[cmbIndex], &beginInfo) != VK_SUCCESS)
    {
        Logger::error("Failed to begin recording command buffer");
        return (VK_NULL_HANDLE);
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType        = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass   = m_usedRenderPass->getVkRenderPass();
    renderPassInfo.framebuffer  = m_framebuffers[framebufferIndex];
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_extent;

    renderPassInfo.clearValueCount  = static_cast<uint32_t>(m_clearValues.size());
    renderPassInfo.pClearValues     = m_clearValues.data();

    vkCmdBeginRenderPass(m_primaryCmb[cmbIndex], &renderPassInfo, contents);

    m_curRecordingIndex = cmbIndex;

    return m_primaryCmb[cmbIndex];
}

bool RenderTarget::endRecording()
{
    vkCmdEndRenderPass(m_primaryCmb[m_curRecordingIndex]);

    if (vkEndCommandBuffer(m_primaryCmb[m_curRecordingIndex]) != VK_SUCCESS)
    {
        Logger::error("Failed to record primary command buffer");
        return (false);
    }

    return (true);
}

void RenderTarget::setExtent(VkExtent2D extent)
{
    m_extent = extent;
}

void RenderTarget::setClearValue(size_t attachmentIndex, glm::vec4 color, glm::vec2 depth)
{
    if(attachmentIndex >= m_clearValues.size())
        m_clearValues.resize(attachmentIndex+1, VkClearValue{});

    m_clearValues[attachmentIndex].color        = {color.r, color.g, color.b, color.a};
    m_clearValues[attachmentIndex].depthStencil = {depth.x, static_cast<uint32_t>(depth.y)};
}

void RenderTarget::setCmbUsage(VkFlags usage)
{
    m_cmbUsage = usage;
}

VkExtent2D RenderTarget::getExtent()
{
    return m_extent;
}

VkFlags RenderTarget::getCmbUsage()
{
    return m_cmbUsage;
}

const VkCommandBuffer *RenderTarget::getPrimaryCmb(size_t cmbIndex)
{
    return &m_primaryCmb[cmbIndex];
}

const  std::vector<VFramebufferAttachment> &RenderTarget::getAttachments(size_t attachmentsIndex)
{
    if(attachmentsIndex >= m_attachments.size())
        throw std::runtime_error("Cannot get attachment");
    return m_attachments[attachmentsIndex];
}


/// Protected ///

bool RenderTarget::createFramebuffers(size_t framebuffersCount)
{
    m_framebuffers.resize(framebuffersCount);

    for (size_t i = 0; i < framebuffersCount ; ++i)
    {
        std::vector<VkImageView> attachments(m_attachments.size());

        for(size_t j = 0 ; j < attachments.size() ; ++j)
        {
            if(m_extent.width != m_attachments[j][i].extent.width
            || m_extent.height != m_attachments[j][i].extent.height)
                m_extent = m_attachments[j][i].extent;
            attachments[j] = m_attachments[j][i].view;
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType           = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass      = m_usedRenderPass->getVkRenderPass();
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

bool RenderTarget::createCmb(size_t cmbCount)
{
    m_primaryCmb.resize(cmbCount);

    VkCommandBufferAllocateInfo allocInfo = {};
    allocInfo.sType         = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    CommandPoolName pool    = (m_cmbUsage == VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT) ? COMMANDPOOL_SHORTLIVED : COMMANDPOOL_DEFAULT;
    allocInfo.commandPool   = VInstance::commandPool(pool);
    allocInfo.level         = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_primaryCmb.size());

    return (vkAllocateCommandBuffers(VInstance::device(), &allocInfo, m_primaryCmb.data()) == VK_SUCCESS);
}

}
